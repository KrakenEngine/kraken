//
//  FileManager.h
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

#ifndef KRAUDIO_MANAGER_H
#define KRAUDIO_MANAGER_H

#include "KREngine-common.h"

#include "KRAudioSample.h"
#include "KRContextObject.h"
#include "KRDataBlock.h"
#include "KRMat4.h"
#include "KRAudioSource.h"

const int KRENGINE_AUDIO_MAX_POOL_SIZE = 32;
const int KRENGINE_AUDIO_MAX_BUFFER_SIZE = 64*1024;
const int KRENGINE_AUDIO_BUFFERS_PER_SOURCE = 3;

const int KRENGINE_AUDIO_BLOCK_LENGTH = 128; // Length of one block to process.  Determines the latency of the audio system and sets size for FFT's used in HRTF convolution
const int KRENGINE_AUDIO_BLOCK_LOG2N = 7; // 2 ^ KRENGINE_AUDIO_BLOCK_LOG2N = KRENGINE_AUDIO_BLOCK_LENGTH

const int KRENGINE_REVERB_MAX_FFT_LOG_2 = 15;
const int KRENGINE_REVERB_WORKSPACE_SIZE = 1 << KRENGINE_REVERB_MAX_FFT_LOG_2;

const int KRENGINE_REVERB_MAX_SAMPLES = 435200; // At least 10s reverb impulse response length, divisible by KRENGINE_AUDIO_BLOCK_LENGTH
const int KRENGINE_MAX_REVERB_IMPULSE_MIX = 16; // Maximum number of impulse response filters that can be mixed simultaneously
const int KRENGINE_MAX_OUTPUT_CHANNELS = 2;

class KRAudioManager : public KRContextObject {
public:
    KRAudioManager(KRContext &context);
    virtual ~KRAudioManager();
    
    void add(KRAudioSample *Sound);
    
    KRAudioSample *load(const std::string &name, const std::string &extension, KRDataBlock *data);
    KRAudioSample *get(const std::string &name);
    
    void setViewMatrix(const KRMat4 &viewMatrix);
    

    void makeCurrentContext();
    
    KRDataBlock *getBufferData(int size);
    void recycleBufferData(KRDataBlock *data);
    
    enum audio_engine_t {
        KRAKEN_AUDIO_NONE,
        KRAKEN_AUDIO_OPENAL,
        KRAKEN_AUDIO_SIREN
    };
    
    audio_engine_t getAudioEngine();
    
    void activateAudioSource(KRAudioSource *audioSource);
    void deactivateAudioSource(KRAudioSource *audioSource);
    
    __int64_t getAudioFrame();
    
    KRAudioBuffer *getBuffer(KRAudioSample &audio_sample, int buffer_index);
    
    float getGlobalReverbSendLevel();
    void setGlobalReverbSendLevel(float send_level);
    
private:
    
    float m_global_reverb_send_level;
    
    KRVector3 m_listener_position;
    KRVector3 m_listener_forward;
    KRVector3 m_listener_up;
    
    map<std::string, KRAudioSample *> m_sounds;
    
    std::vector<KRDataBlock *> m_bufferPoolIdle;
    
    std::vector<KRAudioBuffer *> m_bufferCache;
    
    std::set<KRAudioSource *> m_activeAudioSources;
    
    void initAudio();
    void initOpenAL();
    void initSiren();
    
    void cleanupAudio();
    void cleanupOpenAL();
    void cleanupSiren();
    

    
    audio_engine_t m_audio_engine;
    
    // OpenAL Handles
    ALCcontext* m_alContext;
    ALCdevice* m_alDevice;
    
    // Siren Handles
    AUGraph m_auGraph;
    AudioUnit m_auMixer;
    static OSStatus renderInput(void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData);
    void renderAudio(UInt32 inNumberFrames, AudioBufferList *ioData);
    
    __int64_t m_audio_frame; // Number of audio frames processed since the start of the application
    
    float *m_reverb_input_samples; // Circular-buffered reverb input, single channel
    int m_reverb_input_next_sample; // Pointer to next sample in reverb buffer
    int m_reverb_sequence;
    
    KRAudioSample *m_reverb_impulse_responses[KRENGINE_MAX_REVERB_IMPULSE_MIX];
    float m_reverb_impulse_responses_weight[KRENGINE_MAX_REVERB_IMPULSE_MIX];
    
    float *m_output_accumulation; // Interleaved output accumulation buffer
    int m_output_accumulation_block_start;
    int m_output_sample;
    
    FFTSetup m_fft_setup;
    float *m_reverb_workspace_data;
    DSPSplitComplex m_reverb_workspace[3];
    
    float *getBlockAddress(int block_offset);
    void renderBlock();
    void renderReverb();
    void renderHRTF();
    void renderITD();
    void renderReverbImpulseResponse(KRAudioSample *impulse_response, int impulse_response_offset, int frame_count_log_2);
};

#endif /* defined(KRAUDIO_MANAGER_H) */
