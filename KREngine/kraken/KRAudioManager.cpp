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
#include "KREngine-common.h"
#include "KRDataBlock.h"
#include "KRAudioBuffer.h"

OSStatus  alcASASetListenerProc(const ALuint property, ALvoid *data, ALuint dataSize);
ALvoid  alcMacOSXRenderingQualityProc(const ALint value);

KRAudioManager::KRAudioManager(KRContext &context) : KRContextObject(context)
{
    m_audio_engine = KRAKEN_AUDIO_SIREN;
    
    // OpenAL
    m_alDevice = 0;
    m_alContext = 0;
    
    // Siren
    m_auGraph = NULL;
    m_auMixer = NULL;
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
    static float phase;
    static float pan_phase;
    
	// Get a pointer to the dataBuffer of the AudioBufferList
    
	AudioUnitSampleType *outA = (AudioUnitSampleType *)ioData->mBuffers[0].mData;
    AudioUnitSampleType *outB = (AudioUnitSampleType *)ioData->mBuffers[1].mData; // Non-Interleaved only
    
	// Calculations to produce a 600 Hz sinewave
	// A constant frequency value, you can pass in a reference vary this.
	float freq = 300;
	// The amount the phase changes in  single sample
	double phaseIncrement = M_PI * freq / 44100.0;
	// Pass in a reference to the phase value, you have to keep track of this
	// so that the sin resumes right where the last call left off
    
	// Loop through the callback buffer, generating samples
	for (UInt32 i = 0; i < inNumberFrames; ++i) {
        
        // calculate the next sample
        float sinSignal = sin(phase);
        // Put the sample into the buffer
        // Scale the -1 to 1 values float to
        // -32767 to 32767 and then cast to an integer
        float left_channel = sinSignal * (sin(pan_phase) * 0.5f + 0.5f);
        float right_channel = sinSignal * (-sin(pan_phase) * 0.5f + 0.5f);
        
        
        left_channel = 0;
        right_channel = 0;
        
#if CA_PREFER_FIXED_POINT
        // Interleaved
        //        outA[i*2] = (SInt16)(left_channel * 32767.0f);
        //        outA[i*2 + 1] = (SInt16)(right_channel * 32767.0f);
        
        // Non-Interleaved
        outA[i] = (SInt32)(left_channel * 0x1000000f);
        outB[i] = (SInt32)(right_channel * 0x1000000f);
#else
        
        // Interleaved
        //        outA[i*2] = (Float32)left_channel;
        //        outA[i*2 + 1] = (Float32)right_channel;
        
        // Non-Interleaved
        outA[i] = (Float32)left_channel;
        outB[i] = (Float32)right_channel;
#endif
        // calculate the phase for the next sample
        phase = phase + phaseIncrement;
        pan_phase = pan_phase + 1 / 44100.0 * M_PI;
    }
    // Reset the phase value to prevent the float from overflowing
    if (phase >=  M_PI * freq) {
		phase = phase - M_PI * freq;
	}
    
    for(std::set<KRAudioSource *>::iterator itr=m_activeAudioSources.begin(); itr != m_activeAudioSources.end(); itr++) {
        int channel_count = 1;
        
        KRAudioSource *source = *itr;
        KRAudioBuffer *buffer = source->getBuffer();
        int buffer_offset = source->getBufferFrame();
        int frames_advanced = 0;
        for (UInt32 i = 0; i < inNumberFrames; ++i) {
            float left_channel=0;
            float right_channel=0;
            if(buffer) {
                short *frame = buffer->getFrameData() + (buffer_offset++ * channel_count);
                frames_advanced++;
                left_channel = (float)frame[0] / 32767.0f;
                if(channel_count == 2) {
                    right_channel = (float)frame[1] / 32767.0f;
                } else {
                    right_channel = left_channel;
                }
                if(buffer_offset >= buffer->getFrameCount()) {
                    source->advanceFrames(frames_advanced);
                    frames_advanced = 0;
                    buffer = source->getBuffer();
                    buffer_offset = source->getBufferFrame();
                }
            }
            
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
        }
        source->advanceFrames(frames_advanced);
    }
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
}

void KRAudioManager::initSiren()
{
    if(m_auGraph == NULL) {
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
    
    KRVector3 player_position = KRMat4::Dot(invView, KRVector3(0.0, 0.0, 0.0));
    KRVector3 vectorForward = KRMat4::Dot(invView, KRVector3(0.0, 0.0, -1.0)) - player_position;
    KRVector3 vectorUp = KRMat4::Dot(invView, KRVector3(0.0, 1.0, 0.0)) - player_position;
    
    vectorUp.normalize();
    vectorForward.normalize();
    
    makeCurrentContext();
    if(m_audio_engine == KRAKEN_AUDIO_OPENAL) {
        ALDEBUG(alListener3f(AL_POSITION, player_position.x, player_position.y, player_position.z));
        ALfloat orientation[] = {vectorForward.x, vectorForward.y, vectorForward.z, vectorUp.x, vectorUp.y, vectorUp.z};
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
