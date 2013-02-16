//
//  FileManager.cpp
//  KREngine
//
//  Copyright 2012 Kearwood Gilbert. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without modification, are
//  permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice, this list of
//  conditions and the following disclaimer.
//
//  2. Redistributions in binary form must reproduce the above copyright notice, this list
//  of conditions and the following disclaimer in the documentation and/or other materials
//  provided with the distribution.
//
//  THIS SOFTWARE IS PROVIDED BY KEARWOOD GILBERT ''AS IS'' AND ANY EXPRESS OR IMPLIED
//  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL KEARWOOD GILBERT OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//  The views and conclusions contained in the software and documentation are those of the
//  authors and should not be interpreted as representing official policies, either expressed
//  or implied, of Kearwood Gilbert.
//

#include "KRAudioManager.h"
#include "KRAudioSample.h"
#include "KREngine-common.h"
#include "KRDataBlock.h"
#include "KRAudioBuffer.h"
#include "KRContext.h"

#include <Accelerate/Accelerate.h>

OSStatus  alcASASetListenerProc(const ALuint property, ALvoid *data, ALuint dataSize);
ALvoid  alcMacOSXRenderingQualityProc(const ALint value);

KRAudioManager::KRAudioManager(KRContext &context) : KRContextObject(context)
{
    m_audio_engine = KRAKEN_AUDIO_SIREN;
    
    m_global_reverb_send_level = 0.25f;
    
    // OpenAL
    m_alDevice = 0;
    m_alContext = 0;
    
    // Siren
    m_auGraph = NULL;
    m_auMixer = NULL;
    
    m_audio_frame = 0;
    

    m_output_sample = 0;
    m_fft_setup = NULL;
    
    m_reverb_input_samples = NULL;
    m_reverb_input_next_sample = 0;
    
    m_reverb_workspace_data = NULL;
    m_reverb_sequence = 0;

    for(int i=0; i < KRENGINE_MAX_REVERB_IMPULSE_MIX; i++) {
        m_reverb_impulse_responses[i] = NULL;
        m_reverb_impulse_responses_weight[i] = 0.0f;
    }
}

void KRAudioManager::initAudio()
{
    switch(m_audio_engine) {
        case KRAKEN_AUDIO_OPENAL:
            initOpenAL();
            break;
        case KRAKEN_AUDIO_SIREN:
            initSiren();
            break;
        case KRAKEN_AUDIO_NONE:
            break;
    }
}

void KRAudioManager::renderAudio(UInt32 inNumberFrames, AudioBufferList *ioData)
{
	AudioUnitSampleType *outA = (AudioUnitSampleType *)ioData->mBuffers[0].mData;
    AudioUnitSampleType *outB = (AudioUnitSampleType *)ioData->mBuffers[1].mData; // Non-Interleaved only
    
    int output_frame = 0;
    
    while(output_frame < inNumberFrames) {
        int frames_ready = KRENGINE_AUDIO_BLOCK_LENGTH - m_output_sample;
        if(frames_ready == 0) {
            renderBlock();
            m_output_sample = 0;
            frames_ready = KRENGINE_AUDIO_BLOCK_LENGTH;
        }
        
        int frames_processed = inNumberFrames - output_frame;
        if(frames_processed > frames_ready) frames_processed = frames_ready;
        
        float *block_data = getBlockAddress(0);
        
        for(int i=0; i < frames_processed; i++) {
            
            float left_channel = block_data[m_output_sample * KRENGINE_MAX_OUTPUT_CHANNELS];
            float right_channel = block_data[m_output_sample * KRENGINE_MAX_OUTPUT_CHANNELS + 1];
            m_output_sample++;
            
#if CA_PREFER_FIXED_POINT
            // Interleaved
            //        outA[i*2] = (SInt16)(left_channel * 32767.0f);
            //        outA[i*2 + 1] = (SInt16)(right_channel * 32767.0f);
            
            // Non-Interleaved
            outA[output_frame] = (SInt32)(left_channel * 0x1000000f);
            outB[output_frame] = (SInt32)(right_channel * 0x1000000f);
#else
            
            // Interleaved
            //        outA[i*2] = (Float32)left_channel;
            //        outA[i*2 + 1] = (Float32)right_channel;
            
            // Non-Interleaved
            outA[output_frame] = (Float32)left_channel;
            outB[output_frame] = (Float32)right_channel;
#endif
            output_frame++;
            
        }
    }
}

float *KRAudioManager::getBlockAddress(int block_offset)
{
    return m_output_accumulation + (m_output_accumulation_block_start + block_offset * KRENGINE_AUDIO_BLOCK_LENGTH * KRENGINE_MAX_OUTPUT_CHANNELS) % (KRENGINE_REVERB_MAX_SAMPLES * KRENGINE_MAX_OUTPUT_CHANNELS);
}

void KRAudioManager::renderReverbImpulseResponse(KRAudioSample *impulse_response, int impulse_response_offset, int frame_count_log_2)
{
    int frame_count = 1 << frame_count_log_2;
    int fft_size = frame_count * 2;
    int fft_size_log_2 = frame_count_log_2 + 1;
    
    DSPSplitComplex reverb_sample_data_complex = m_reverb_workspace[0];
    DSPSplitComplex impulse_block_data_complex = m_reverb_workspace[1];
    DSPSplitComplex conv_data_complex = m_reverb_workspace[2];
    
    int reverb_offset = (m_reverb_input_next_sample + KRENGINE_AUDIO_BLOCK_LENGTH - frame_count);
    if(reverb_offset < 0) {
        reverb_offset += KRENGINE_REVERB_MAX_SAMPLES;
    } else {
        reverb_offset = reverb_offset % KRENGINE_REVERB_MAX_SAMPLES;
    }
    
    int frames_left = frame_count;
    while(frames_left) {
        int frames_to_process = KRENGINE_REVERB_MAX_SAMPLES - reverb_offset;
        if(frames_to_process > frames_left) frames_to_process = frames_left;
        memcpy(reverb_sample_data_complex.realp + frame_count - frames_left, m_reverb_input_samples + reverb_offset, frames_to_process * sizeof(float));
        
        frames_left -= frames_to_process;
        reverb_offset = (reverb_offset + frames_to_process) % KRENGINE_REVERB_MAX_SAMPLES;
    }
    
    memset(reverb_sample_data_complex.realp + frame_count, 0, frame_count * sizeof(float));
    memset(reverb_sample_data_complex.imagp, 0, fft_size * sizeof(float));

    vDSP_fft_zip(m_fft_setup, &reverb_sample_data_complex, 1, fft_size_log_2, kFFTDirection_Forward);
    
    float scale = 1.0f / fft_size;
    
    int impulse_response_channels = 2;
    for(int channel=0; channel < impulse_response_channels; channel++) {
        
        impulse_response->sample(impulse_response_offset, frame_count, channel, impulse_block_data_complex.realp, 1.0f);
        memset(impulse_block_data_complex.realp + frame_count, 0, frame_count * sizeof(float));
        memset(impulse_block_data_complex.imagp, 0, fft_size * sizeof(float));
        
        vDSP_fft_zip(m_fft_setup, &impulse_block_data_complex, 1, fft_size_log_2, kFFTDirection_Forward);
        vDSP_zvmul(&reverb_sample_data_complex, 1, &impulse_block_data_complex, 1, &conv_data_complex, 1, fft_size, 1);
        vDSP_fft_zip(m_fft_setup, &conv_data_complex, 1, fft_size_log_2, kFFTDirection_Inverse);
        vDSP_vsmul(conv_data_complex.realp, 1, &scale, conv_data_complex.realp, 1, fft_size);
        
        
        int output_offset = (m_output_accumulation_block_start + impulse_response_offset * KRENGINE_MAX_OUTPUT_CHANNELS) % (KRENGINE_REVERB_MAX_SAMPLES * KRENGINE_MAX_OUTPUT_CHANNELS);
        frames_left = fft_size;
        while(frames_left) {
            int frames_to_process = (KRENGINE_REVERB_MAX_SAMPLES * KRENGINE_MAX_OUTPUT_CHANNELS - output_offset) / KRENGINE_MAX_OUTPUT_CHANNELS;
            if(frames_to_process > frames_left) frames_to_process = frames_left;
            vDSP_vadd(m_output_accumulation + output_offset + channel, KRENGINE_MAX_OUTPUT_CHANNELS, conv_data_complex.realp + fft_size - frames_left, 1, m_output_accumulation + output_offset + channel, KRENGINE_MAX_OUTPUT_CHANNELS, frames_to_process);
            frames_left -= frames_to_process;
            output_offset = (output_offset + frames_to_process * KRENGINE_MAX_OUTPUT_CHANNELS) % (KRENGINE_REVERB_MAX_SAMPLES * KRENGINE_MAX_OUTPUT_CHANNELS);
        }
    }
}

void KRAudioManager::renderHRTF()
{
    
}

void KRAudioManager::renderReverb()
{
    float reverb_data[KRENGINE_AUDIO_BLOCK_LENGTH];
    
    float *reverb_accum = m_reverb_input_samples + m_reverb_input_next_sample;
    memset(reverb_accum, 0, sizeof(float) * KRENGINE_AUDIO_BLOCK_LENGTH);
    for(std::set<KRAudioSource *>::iterator itr=m_activeAudioSources.begin(); itr != m_activeAudioSources.end(); itr++) {
        KRAudioSource *source = *itr;
        float reverb_send_level = m_global_reverb_send_level * source->getReverb();
        if(reverb_send_level > 0.0f) {
            KRAudioSample *sample = source->getAudioSample();
            if(sample) {
                sample->sample((int)((__int64_t)m_audio_frame - source->getStartAudioFrame()), KRENGINE_AUDIO_BLOCK_LENGTH, 0, reverb_data, reverb_send_level);
                vDSP_vadd(reverb_accum, 1, reverb_data, 1, reverb_accum, 1, KRENGINE_AUDIO_BLOCK_LENGTH);
            }
        }
    }
    
    // Apply impulse response reverb
    //    KRAudioSample *impulse_response = getContext().getAudioManager()->get("hrtf_kemar_H10e040a");
    KRAudioSample *impulse_response_sample = getContext().getAudioManager()->get("test_reverb");
    if(impulse_response_sample) {
        int impulse_response_blocks = impulse_response_sample->getFrameCount() / KRENGINE_AUDIO_BLOCK_LENGTH + 1;
        int period_log_2 = 0;
        int impulse_response_block=0;
        int period_count = 0;
        
//        fprintf(stderr, "m_reverb_sequence - %i - ", m_reverb_sequence);
        while(impulse_response_block < impulse_response_blocks) {
            int period = 1 << period_log_2;
            
            if((m_reverb_sequence + period - period_count) % period == 0) {
//                fprintf(stderr, "-%02i-", period);
                renderReverbImpulseResponse(impulse_response_sample, impulse_response_block * KRENGINE_AUDIO_BLOCK_LENGTH, KRENGINE_AUDIO_BLOCK_LOG2N + period_log_2);
            } else {
//                fprintf(stderr, "----");
            }
            
            impulse_response_block += period;
            
            period_count++;
            if(period_count >= period) {
                period_count = 0;
                if(KRENGINE_AUDIO_BLOCK_LOG2N + period_log_2 + 1 < KRENGINE_REVERB_MAX_FFT_LOG_2) { // FFT Size is double the number of frames in the period
                    period_log_2++;
                }
            }
        }
//        fprintf(stderr, "\n");
    }
    
    m_reverb_sequence = (m_reverb_sequence + 1) % 0x1000000;
    
    // Rotate reverb buffer
    m_reverb_input_next_sample = (m_reverb_input_next_sample + KRENGINE_AUDIO_BLOCK_LENGTH) % KRENGINE_REVERB_MAX_SAMPLES;
}

void KRAudioManager::renderBlock()
{
    // ----====---- Advance to next block in accumulation buffer ----====----
    
    // Zero out block that was last used, so it will be ready for the next pass through the circular buffer
    float *block_data = getBlockAddress(0);
    memset(block_data, 0, KRENGINE_AUDIO_BLOCK_LENGTH * KRENGINE_MAX_OUTPUT_CHANNELS * sizeof(float));
    
    // Advance to the next block, and wrap around
    m_output_accumulation_block_start = (m_output_accumulation_block_start + KRENGINE_AUDIO_BLOCK_LENGTH * KRENGINE_MAX_OUTPUT_CHANNELS) % (KRENGINE_REVERB_MAX_SAMPLES * KRENGINE_MAX_OUTPUT_CHANNELS);
    
    // ----====---- Render Direct / HRTF audio ----====----
    renderHRTF();
//    renderITD();
    
    // ----====---- Render Indirect / Reverb channel ----====----
    renderReverb();
    
    // ----====---- Advance audio sources ----====----
    m_audio_frame += KRENGINE_AUDIO_BLOCK_LENGTH;
}

// audio render procedure, don't allocate memory, don't take any locks, don't waste time
OSStatus KRAudioManager::renderInput(void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData)
{
    ((KRAudioManager*)inRefCon)->renderAudio(inNumberFrames, ioData);
	return noErr;
}


void KRSetAUCanonical(AudioStreamBasicDescription &desc, UInt32 nChannels, bool interleaved)
{
    desc.mFormatID = kAudioFormatLinearPCM;
#if CA_PREFER_FIXED_POINT
    desc.mFormatFlags = kAudioFormatFlagsCanonical | (kAudioUnitSampleFractionBits << kLinearPCMFormatFlagsSampleFractionShift);
#else
    desc.mFormatFlags = kAudioFormatFlagsCanonical;
#endif
    desc.mChannelsPerFrame = nChannels;
    desc.mFramesPerPacket = 1;
    desc.mBitsPerChannel = 8 * sizeof(AudioUnitSampleType);
    if (interleaved)
        desc.mBytesPerPacket = desc.mBytesPerFrame = nChannels * sizeof(AudioUnitSampleType);
    else {
        desc.mBytesPerPacket = desc.mBytesPerFrame = sizeof(AudioUnitSampleType);
        desc.mFormatFlags |= kAudioFormatFlagIsNonInterleaved;
    }
}

void KRAudioManager::initSiren()
{
    if(m_auGraph == NULL) {
        m_output_sample = KRENGINE_AUDIO_BLOCK_LENGTH;
        
        // initialize double-buffer for reverb input
        int buffer_size = sizeof(float) * KRENGINE_REVERB_MAX_SAMPLES; // Reverb input is a single channel, circular buffered
        m_reverb_input_samples = (float *)malloc(buffer_size);
        memset(m_reverb_input_samples, 0, buffer_size);
        m_reverb_input_next_sample = 0;
        
        m_output_accumulation = (float *)malloc(buffer_size * 2); // 2 channels
        memset(m_output_accumulation, 0, buffer_size * 2);
        m_output_accumulation_block_start = 0;
        
        m_reverb_workspace_data = (float *)malloc(KRENGINE_REVERB_WORKSPACE_SIZE * 6 * sizeof(float));
        m_reverb_workspace[0].realp = m_reverb_workspace_data + KRENGINE_REVERB_WORKSPACE_SIZE * 0;
        m_reverb_workspace[0].imagp = m_reverb_workspace_data + KRENGINE_REVERB_WORKSPACE_SIZE * 1;
        m_reverb_workspace[1].realp = m_reverb_workspace_data + KRENGINE_REVERB_WORKSPACE_SIZE * 2;
        m_reverb_workspace[1].imagp = m_reverb_workspace_data + KRENGINE_REVERB_WORKSPACE_SIZE * 3;
        m_reverb_workspace[2].realp = m_reverb_workspace_data + KRENGINE_REVERB_WORKSPACE_SIZE * 4;
        m_reverb_workspace[2].imagp = m_reverb_workspace_data + KRENGINE_REVERB_WORKSPACE_SIZE * 5;
        
        m_reverb_sequence = 0;
        
        m_fft_setup = vDSP_create_fftsetup( KRENGINE_REVERB_MAX_FFT_LOG_2, kFFTRadix2);
        
        // ----====---- Initialize Core Audio Objects ----====----
        OSDEBUG(NewAUGraph(&m_auGraph));
        
        // ---- Create output node ----
        AudioComponentDescription output_desc;
        output_desc.componentType = kAudioUnitType_Output;
#if TARGET_OS_IPHONE
        output_desc.componentSubType = kAudioUnitSubType_RemoteIO;
#else
        output_desc.componentSubType = kAudioUnitSubType_DefaultOutput;
#endif
        output_desc.componentFlags = 0;
        output_desc.componentFlagsMask = 0;
        output_desc.componentManufacturer = kAudioUnitManufacturer_Apple;
        AUNode outputNode = 0;
        OSDEBUG(AUGraphAddNode(m_auGraph, &output_desc, &outputNode));
        
        // ---- Create mixer node ----
        AudioComponentDescription mixer_desc;
        mixer_desc.componentType = kAudioUnitType_Mixer;
        mixer_desc.componentSubType = kAudioUnitSubType_MultiChannelMixer;
        mixer_desc.componentFlags = 0;
        mixer_desc.componentFlagsMask = 0;
        mixer_desc.componentManufacturer = kAudioUnitManufacturer_Apple;
        AUNode mixerNode = 0;
        OSDEBUG(AUGraphAddNode(m_auGraph, &mixer_desc, &mixerNode ));
        
        // ---- Connect mixer to output node ----
        OSDEBUG(AUGraphConnectNodeInput(m_auGraph, mixerNode, 0, outputNode, 0));
        
        // ---- Open the audio graph ----
        OSDEBUG(AUGraphOpen(m_auGraph));
        
        // ---- Get a handle to the mixer ----
        OSDEBUG(AUGraphNodeInfo(m_auGraph, mixerNode, NULL, &m_auMixer));
        
        // ---- Add output channel to mixer ----
        UInt32 bus_count = 1;
        OSDEBUG(AudioUnitSetProperty(m_auMixer, kAudioUnitProperty_ElementCount, kAudioUnitScope_Input, 0, &bus_count, sizeof(bus_count)));
        
        // ---- Attach render function to channel ----
        AURenderCallbackStruct renderCallbackStruct;
		renderCallbackStruct.inputProc = &renderInput;
		renderCallbackStruct.inputProcRefCon = this;
        OSDEBUG(AUGraphSetNodeInputCallback(m_auGraph, mixerNode, 0, &renderCallbackStruct)); // 0 = mixer input number
        
        AudioStreamBasicDescription desc;
        memset(&desc, 0, sizeof(desc));
        
        UInt32 size = sizeof(desc);
        memset(&desc, 0, sizeof(desc));
		OSDEBUG(AudioUnitGetProperty(  m_auMixer,
                                      kAudioUnitProperty_StreamFormat,
                                      kAudioUnitScope_Input,
                                      0, // 0 = mixer input number
                                      &desc,
                                      &size));

        KRSetAUCanonical(desc, 2, false);
        desc.mSampleRate = 44100.0f;
        /*
        desc.mSampleRate = 44100.0; // set sample rate
		desc.mFormatID = kAudioFormatLinearPCM;
		desc.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
		desc.mBitsPerChannel = sizeof(AudioSampleType) * 8; // AudioSampleType == 16 bit signed ints
		desc.mChannelsPerFrame = 2;
		desc.mFramesPerPacket = 1;
		desc.mBytesPerFrame = (desc.mBitsPerChannel / 8) * desc.mChannelsPerFrame;
		desc.mBytesPerPacket = desc.mBytesPerFrame * desc.mFramesPerPacket;
         */
        
        OSDEBUG(AudioUnitSetProperty(m_auMixer,
                             kAudioUnitProperty_StreamFormat,
                             kAudioUnitScope_Input,
                             0, // 0 == mixer input number
                             &desc,
                            sizeof(desc)));
        
        // ---- Apply properties to mixer output ----
        OSDEBUG(AudioUnitSetProperty(m_auMixer,
                             kAudioUnitProperty_StreamFormat,
                             kAudioUnitScope_Output,
                             0, // Always 0 for output bus
                             &desc,
                             sizeof(desc)));
        
        
        memset(&desc, 0, sizeof(desc));
        size = sizeof(desc);
        OSDEBUG(AudioUnitGetProperty(m_auMixer,
                             kAudioUnitProperty_StreamFormat,
                             kAudioUnitScope_Output,
                             0,
                             &desc,
                             &size));
        
        // ----
        // AUCanonical on the iPhone is the 8.24 integer format that is native to the iPhone.

        KRSetAUCanonical(desc, 2, false);
        desc.mSampleRate = 44100.0f;
//        int channel_count = 2;
//        bool interleaved = true;
//        
//        desc.mFormatID = kAudioFormatLinearPCM;
//#if CA_PREFER_FIXED_POINT
//        desc.mFormatFlags = kAudioFormatFlagsCanonical | (kAudioUnitSampleFractionBits << kLinearPCMFormatFlagsSampleFractionShift);
//#else
//        desc.mFormatFlags = kAudioFormatFlagsCanonical;
//#endif
//        desc.mChannelsPerFrame = channel_count;
//        desc.mFramesPerPacket = 1;
//        desc.mBitsPerChannel = 8 * sizeof(AudioUnitSampleType);
//        if (interleaved) {
//            desc.mBytesPerPacket = desc.mBytesPerFrame = channel_count * sizeof(AudioUnitSampleType);
//        } else {
//            desc.mBytesPerPacket = desc.mBytesPerFrame = sizeof(AudioUnitSampleType);
//            desc.mFormatFlags |= kAudioFormatFlagIsNonInterleaved;
//        }
        
        // ----

        OSDEBUG(AudioUnitSetProperty(m_auMixer,
                                      kAudioUnitProperty_StreamFormat,
                                      kAudioUnitScope_Output,
                                      0,
                                      &desc,
                                      sizeof(desc)));
        
        
        OSDEBUG(AudioUnitSetParameter(m_auMixer, kMultiChannelMixerParam_Volume, kAudioUnitScope_Input, 0, 1.0, 0));
        OSDEBUG(AudioUnitSetParameter(m_auMixer, kMultiChannelMixerParam_Volume, kAudioUnitScope_Output, 0, 1.0, 0));
        
        OSDEBUG(AUGraphInitialize(m_auGraph));
        
        OSDEBUG(AUGraphStart(m_auGraph));
        
//        CAShow(m_auGraph);
    }
}


void KRAudioManager::cleanupSiren()
{
    if(m_auGraph) {
        OSDEBUG(AUGraphStop(m_auGraph));
        OSDEBUG(DisposeAUGraph(m_auGraph));
        m_auGraph = NULL;
        m_auMixer = NULL;
    }
    
    if(m_reverb_input_samples) {
        free(m_reverb_input_samples);
        m_reverb_input_samples = NULL;
    }
    
    if(m_output_accumulation) {
        free(m_output_accumulation);
        m_output_accumulation = NULL;
    }
    
    if(m_reverb_workspace_data) {
        free(m_reverb_workspace_data);
        m_reverb_workspace_data = NULL;
    }
        
    for(int i=0; i < KRENGINE_MAX_REVERB_IMPULSE_MIX; i++) {
        m_reverb_impulse_responses[i] = NULL;
        m_reverb_impulse_responses_weight[i] = 0.0f;
    }
    
    if(m_fft_setup) {
        vDSP_destroy_fftsetup(m_fft_setup);
        m_fft_setup = NULL;
    }
}

void KRAudioManager::initOpenAL()
{
    if(m_alDevice == 0) {
        // ----- Initialize OpenAL -----
        ALDEBUG(m_alDevice = alcOpenDevice(NULL));
        ALDEBUG(m_alContext=alcCreateContext(m_alDevice,NULL));
        ALDEBUG(alcMakeContextCurrent(m_alContext));
        
        // ----- Configure listener -----
        ALDEBUG(alDistanceModel(AL_EXPONENT_DISTANCE));
        ALDEBUG(alSpeedOfSound(1116.43701f)); // 1116.43701 feet per second
        
         /*
          // BROKEN IN IOS 6!!
    #if TARGET_OS_IPHONE
        ALDEBUG(alcMacOSXRenderingQualityProc(ALC_IPHONE_SPATIAL_RENDERING_QUALITY_HEADPHONES));
    #else
        ALDEBUG(alcMacOSXRenderingQualityProc(ALC_MAC_OSX_SPATIAL_RENDERING_QUALITY_HIGH));
    #endif
          */
        bool enable_reverb = false;
        if(enable_reverb) {
            UInt32 setting = 1;
            ALDEBUG(alcASASetListenerProc(ALC_ASA_REVERB_ON, &setting, sizeof(setting)));
            ALfloat global_reverb_level = -5.0f;
            ALDEBUG(alcASASetListenerProc(ALC_ASA_REVERB_GLOBAL_LEVEL, &global_reverb_level, sizeof(global_reverb_level)));
            
            setting = ALC_ASA_REVERB_ROOM_TYPE_SmallRoom; // ALC_ASA_REVERB_ROOM_TYPE_MediumHall2;
            ALDEBUG(alcASASetListenerProc(ALC_ASA_REVERB_ROOM_TYPE, &setting, sizeof(setting)));
            
            
            ALfloat global_reverb_eq_gain = 0.0f;
            ALDEBUG(alcASASetListenerProc(ALC_ASA_REVERB_EQ_GAIN, &global_reverb_eq_gain, sizeof(global_reverb_eq_gain)));
            
            ALfloat global_reverb_eq_bandwidth = 0.0f;
            ALDEBUG(alcASASetListenerProc(ALC_ASA_REVERB_EQ_BANDWITH, &global_reverb_eq_bandwidth, sizeof(global_reverb_eq_bandwidth)));
            
            ALfloat global_reverb_eq_freq = 0.0f;
            ALDEBUG(alcASASetListenerProc(ALC_ASA_REVERB_EQ_FREQ, &global_reverb_eq_freq, sizeof(global_reverb_eq_freq)));
        }
    }
}

void KRAudioManager::cleanupAudio()
{
    cleanupOpenAL();
    cleanupSiren();
}

void KRAudioManager::cleanupOpenAL()
{
    if(m_alContext) {
        ALDEBUG(alcDestroyContext(m_alContext));
        m_alContext = 0;
    }
    if(m_alDevice) {
        ALDEBUG(alcCloseDevice(m_alDevice));
        m_alDevice = 0;
    }
}

KRAudioManager::~KRAudioManager()
{
    for(map<std::string, KRAudioSample *>::iterator name_itr=m_sounds.begin(); name_itr != m_sounds.end(); name_itr++) {
        delete (*name_itr).second;
    }
    
    cleanupAudio();
    
    for(std::vector<KRDataBlock *>::iterator itr = m_bufferPoolIdle.begin(); itr != m_bufferPoolIdle.end(); itr++) {
        delete *itr;
    }
}

void KRAudioManager::makeCurrentContext()
{
    initAudio();
    if(m_audio_engine == KRAKEN_AUDIO_OPENAL) {
        if(m_alContext != 0) {
            ALDEBUG(alcMakeContextCurrent(m_alContext));
        }
    }
}

void KRAudioManager::setViewMatrix(const KRMat4 &viewMatrix)
{
    KRMat4 invView = viewMatrix;
    invView.invert();
    
    m_listener_position = KRMat4::Dot(invView, KRVector3(0.0, 0.0, 0.0));
    m_listener_forward = KRMat4::Dot(invView, KRVector3(0.0, 0.0, -1.0)) - m_listener_position;
    m_listener_up = KRMat4::Dot(invView, KRVector3(0.0, 1.0, 0.0)) - m_listener_position;
    
    m_listener_up.normalize();
    m_listener_forward.normalize();
    
    makeCurrentContext();
    if(m_audio_engine == KRAKEN_AUDIO_OPENAL) {
        ALDEBUG(alListener3f(AL_POSITION, m_listener_position.x, m_listener_position.y, m_listener_position.z));
        ALfloat orientation[] = {m_listener_forward.x, m_listener_forward.y, m_listener_forward.z, m_listener_up.x, m_listener_up.y, m_listener_up.z};
        ALDEBUG(alListenerfv(AL_ORIENTATION, orientation));
    }
}

void KRAudioManager::add(KRAudioSample *sound)
{
    std::string lower_name = sound->getName();
    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
    
    map<std::string, KRAudioSample *>::iterator name_itr = m_sounds.find(lower_name);
    if(name_itr != m_sounds.end()) {
        delete (*name_itr).second;
        (*name_itr).second = sound;
    } else {
        m_sounds[lower_name] = sound;
    }
}

KRAudioSample *KRAudioManager::load(const std::string &name, const std::string &extension, KRDataBlock *data)
{
    KRAudioSample *Sound = new KRAudioSample(getContext(), name, extension, data);
    if(Sound) add(Sound);
    return Sound;
}

KRAudioSample *KRAudioManager::get(const std::string &name)
{
    std::string lower_name = name;
    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
    return m_sounds[lower_name];
}

KRDataBlock *KRAudioManager::getBufferData(int size)
{
    KRDataBlock *data;
    // Note: We only store and recycle buffers with a size of CIRCA_AUDIO_MAX_BUFFER_SIZE
    if(size == KRENGINE_AUDIO_MAX_BUFFER_SIZE && m_bufferPoolIdle.size() > 0) {
        // Recycle a buffer from the pool
        data = m_bufferPoolIdle.back();
        m_bufferPoolIdle.pop_back();
    } else {
        data = new KRDataBlock();
        data->expand(size);
    }
    
    return data;

}

void KRAudioManager::recycleBufferData(KRDataBlock *data)
{
    if(data != NULL) {
        if(data->getSize() == KRENGINE_AUDIO_MAX_BUFFER_SIZE && m_bufferPoolIdle.size() < KRENGINE_AUDIO_MAX_POOL_SIZE) {
            m_bufferPoolIdle.push_back(data);
        } else {
            delete data;
        }
    }
}

OSStatus alcASASetListenerProc(const ALuint property, ALvoid *data, ALuint dataSize)
{
    OSStatus    err = noErr;
    static  alcASASetListenerProcPtr    proc = NULL;
    
    if (proc == NULL) {
        proc = (alcASASetListenerProcPtr) alcGetProcAddress(NULL, "alcASASetListener");
    }
    
    if (proc)
        err = proc(property, data, dataSize);
    return (err);
}

ALvoid alcMacOSXRenderingQualityProc(const ALint value)
{
    static  alcMacOSXRenderingQualityProcPtr    proc = NULL;
    
    if (proc == NULL) {
        proc = (alcMacOSXRenderingQualityProcPtr) alcGetProcAddress(NULL, (const ALCchar*) "alcMacOSXRenderingQuality");
    }
    
    if (proc)
        proc(value);
    
    return;
}

KRAudioManager::audio_engine_t KRAudioManager::getAudioEngine()
{
    return m_audio_engine;
}

void KRAudioManager::activateAudioSource(KRAudioSource *audioSource)
{
    m_activeAudioSources.insert(audioSource);
}

void KRAudioManager::deactivateAudioSource(KRAudioSource *audioSource)
{
    m_activeAudioSources.erase(audioSource);
}

__int64_t KRAudioManager::getAudioFrame()
{
    return m_audio_frame;
}

KRAudioBuffer *KRAudioManager::getBuffer(KRAudioSample &audio_sample, int buffer_index)
{
    // ----====---- Try to find the buffer in the cache ----====----
    for(std::vector<KRAudioBuffer *>::iterator itr=m_bufferCache.begin(); itr != m_bufferCache.end(); itr++) {
        KRAudioBuffer *cache_buffer = *itr;
        if(cache_buffer->getAudioSample() == &audio_sample && cache_buffer->getIndex() == buffer_index) {
            return cache_buffer;
        }
    }
    
    // ----====---- Make room in the cache for a new buffer ----====----
    if(m_bufferCache.size() >= KRENGINE_AUDIO_MAX_POOL_SIZE) {
        // delete a random entry from the cache
        int index_to_delete = arc4random() % m_bufferCache.size();
        std::vector<KRAudioBuffer *>::iterator itr_to_delete = m_bufferCache.begin() + index_to_delete;
        delete *itr_to_delete;
        m_bufferCache.erase(itr_to_delete);
    }
    
    // ----====---- Request new buffer, add to cache, and return it ----====----
    KRAudioBuffer *buffer = audio_sample.getBuffer(buffer_index);
    m_bufferCache.push_back(buffer);
    return buffer;
}

float KRAudioManager::getGlobalReverbSendLevel()
{
    return m_global_reverb_send_level;
}

void KRAudioManager::setGlobalReverbSendLevel(float send_level)
{
    m_global_reverb_send_level = send_level;
}


void KRAudioManager::renderITD()
{
    /*
     
     
     // hrtf_kemar_H-10e000a.wav
     
     float speed_of_sound = 1126.0f; // feed per second FINDME - TODO - This needs to be configurable for scenes with different units
     float head_radius = 0.7431f; // 0.74ft = 22cm
     float half_max_itd_time = head_radius / speed_of_sound / 2.0f; // half of ITD time (Interaural time difference) when audio source is directly 90 degrees azimuth to one ear.
     
     //    KRVector3 m_listener_position;
     //    KRVector3 m_listener_forward;
     //    KRVector3 m_listener_up;
     
     KRVector3 listener_right = KRVector3::Cross(m_listener_forward, m_listener_up);
     KRVector3 listener_right_ear = m_listener_position + listener_right * head_radius / 2.0f;
     KRVector3 listener_left_ear = m_listener_position - listener_right * head_radius / 2.0f;
     
     // Get a pointer to the dataBuffer of the AudioBufferList
     
     AudioUnitSampleType *outA = (AudioUnitSampleType *)ioData->mBuffers[0].mData;
     AudioUnitSampleType *outB = (AudioUnitSampleType *)ioData->mBuffers[1].mData; // Non-Interleaved only
     
     
     // ----====---- Zero out accumulation / output buffer ----====----
     for (UInt32 i = 0; i < inNumberFrames; ++i) {
     
     #if CA_PREFER_FIXED_POINT
     // Interleaved
     //        outA[i*2] = (SInt16)(left_channel * 32767.0f);
     //        outA[i*2 + 1] = (SInt16)(right_channel * 32767.0f);
     
     // Non-Interleaved
     outA[i] = (SInt32)(0x1000000f);
     outB[i] = (SInt32)(0x1000000f);
     #else
     
     // Interleaved
     //        outA[i*2] = (Float32)left_channel;
     //        outA[i*2 + 1] = (Float32)right_channel;
     
     // Non-Interleaved
     outA[i] = (Float32)0.0f;
     outB[i] = (Float32)0.0f;
     #endif
     }
     
     // ----====---- Render direct / HRTF audio ----====----
     for(std::set<KRAudioSource *>::iterator itr=m_activeAudioSources.begin(); itr != m_activeAudioSources.end(); itr++) {
     KRAudioSource *source = *itr;
     KRVector3 listener_to_source = source->getWorldTranslation() - m_listener_position;
     KRVector3 right_ear_to_source = source->getWorldTranslation() - listener_right_ear;
     KRVector3 left_ear_to_source = source->getWorldTranslation() - listener_left_ear;
     KRVector3 source_direction = KRVector3::Normalize(listener_to_source);
     float right_ear_distance = right_ear_to_source.magnitude();
     float left_ear_distance = left_ear_to_source.magnitude();
     float right_itd_time = right_ear_distance / speed_of_sound;
     float left_itd_time = left_ear_distance / speed_of_sound;
     
     float rolloff_factor = source->getRolloffFactor();
     float left_gain = 1.0f / pow(left_ear_distance / rolloff_factor, 2.0f);
     float right_gain = 1.0f / pow(left_ear_distance / rolloff_factor, 2.0f);
     if(left_gain > 1.0f) left_gain = 1.0f;
     if(right_gain > 1.0f) right_gain = 1.0f;
     left_gain *= source->getGain();
     right_gain *= source->getGain();
     int left_itd_offset = (int)(left_itd_time * 44100.0f);
     int right_itd_offset = (int)(right_itd_time * 44100.0f);
     KRAudioSample *sample = source->getAudioSample();
     if(sample) {
     __int64_t source_start_frame = source->getStartAudioFrame();
     int sample_frame = (int)(m_audio_frame - source_start_frame);
     for (UInt32 i = 0; i < inNumberFrames; ++i) {
     float left_channel=sample->sample(sample_frame + left_itd_offset, 44100, 0) * left_gain;
     float right_channel = sample->sample(sample_frame + right_itd_offset, 44100, 0) * right_gain;
     
     //                left_channel = 0.0f;
     //                right_channel = 0.0f;
     
     #if CA_PREFER_FIXED_POINT
     // Interleaved
     //        outA[i*2] = (SInt16)(left_channel * 32767.0f);
     //        outA[i*2 + 1] = (SInt16)(right_channel * 32767.0f);
     
     // Non-Interleaved
     outA[i] += (SInt32)(left_channel * 0x1000000f);
     outB[i] += (SInt32)(right_channel * 0x1000000f);
     #else
     
     // Interleaved
     //        outA[i*2] = (Float32)left_channel;
     //        outA[i*2 + 1] = (Float32)right_channel;
     
     // Non-Interleaved
     outA[i] += (Float32)left_channel;
     outB[i] += (Float32)right_channel;
     #endif
     sample_frame++;
     }
     }
     }
     
     KRAudioSample *reverb_impulse_response = getContext().getAudioManager()->get("test_reverb");
     
     // ----====---- Render Indirect / Reverb channel ----====----
     int buffer_frame_start=0;
     int remaining_frames = inNumberFrames - buffer_frame_start;
     while(remaining_frames) {
     int frames_processed = remaining_frames;
     if(frames_processed > KRENGINE_REVERB_FILTER_LENGTH) {
     frames_processed = KRENGINE_REVERB_FILTER_LENGTH;
     }
     bool first_source = true;
     for(std::set<KRAudioSource *>::iterator itr=m_activeAudioSources.begin(); itr != m_activeAudioSources.end(); itr++) {
     KRAudioSource *source = *itr;
     KRAudioSample *sample = source->getAudioSample();
     if(sample) {
     __int64_t source_start_frame = source->getStartAudioFrame();
     int sample_frame = (int)(m_audio_frame - source_start_frame + buffer_frame_start);
     
     
     
     
     for (UInt32 i = 0; i < inNumberFrames; ++i) {
     if(first_source) {
     sample->sample(sample_frame, 44100, 0, m_reverb_input_next_sample);
     }
     sample->sampleAdditive(sample_frame, 44100, 0, m_reverb_input_next_sample);
     if(m_reverb_input_next_sample >= KRENGINE_REVERB_MAX_SAMPLES) {
     m_reverb_input_next_sample -= KRENGINE_REVERB_MAX_SAMPLES;
     }
     }
     }
     first_source = false;
     }
     
     
     if(reverb_impulse_response) {
     // Perform FFT Convolution for Impulse-response based reverb
     
     memcpy(m_reverb_accumulation, m_reverb_accumulation + KRENGINE_REVERB_FILTER_LENGTH, KRENGINE_REVERB_FILTER_LENGTH * sizeof(float));
     memset(m_reverb_accumulation + KRENGINE_REVERB_FILTER_LENGTH, 0, KRENGINE_REVERB_FILTER_LENGTH * sizeof(float));
     }
     
     buffer_frame_start += frames_processed;
     remaining_frames = inNumberFrames - buffer_frame_start;
     }
     
     
     */
}