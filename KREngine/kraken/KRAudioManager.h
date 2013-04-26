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

const int KRENGINE_REVERB_MAX_FFT_LOG2 = 17;
const int KRENGINE_REVERB_WORKSPACE_SIZE = 1 << KRENGINE_REVERB_MAX_FFT_LOG2;

const float KRENGINE_AUDIO_CUTOFF = 0.02f; // Cutoff gain level, to cull out processing of very quiet sounds

const int KRENGINE_REVERB_MAX_SAMPLES = 435200; // At least 10s reverb impulse response length, divisible by KRENGINE_AUDIO_BLOCK_LENGTH
const int KRENGINE_MAX_REVERB_IMPULSE_MIX = 16; // Maximum number of impulse response filters that can be mixed simultaneously
const int KRENGINE_MAX_OUTPUT_CHANNELS = 2;

const int KRENGINE_MAX_ACTIVE_SOURCES = 24;
const int KRENGINE_AUDIO_ANTICLICK_SAMPLES = 64;


class KRAmbientZone;
class KRReverbZone;

typedef struct {
    float weight;
    KRAmbientZone *ambient_zone;
    KRAudioSample *ambient_sample;
} siren_ambient_zone_weight_info;

typedef struct {
    float weight;
    KRReverbZone *reverb_zone;
    KRAudioSample *reverb_sample;
} siren_reverb_zone_weight_info;

class KRAudioManager : public KRContextObject {
public:
    KRAudioManager(KRContext &context);
    virtual ~KRAudioManager();
    
    void add(KRAudioSample *Sound);
    
    KRAudioSample *load(const std::string &name, const std::string &extension, KRDataBlock *data);
    KRAudioSample *get(const std::string &name);
    
    // Listener position and orientation
    KRScene *getListenerScene();
    void setListenerScene(KRScene *scene);
    void setListenerOrientation(const KRVector3 &position, const KRVector3 &forward, const KRVector3 &up);
    void setListenerOrientationFromModelMatrix(const KRMat4 &modelMatrix);
    KRVector3 &getListenerForward();
    KRVector3 &getListenerPosition();
    KRVector3 &getListenerUp();
    
    
    // Global audio gain / attenuation
    float getGlobalGain();
    void setGlobalGain(float gain);
    float getGlobalReverbSendLevel();
    void setGlobalReverbSendLevel(float send_level);
    float getGlobalAmbientGain();
    void setGlobalAmbientGain(float gain);

    
    
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
    

    void startFrame(float deltaTime);
private:
    
    KRScene *m_listener_scene; // For now, only one scene is allowed to have active audio at once
    
    float m_global_reverb_send_level;
    float m_global_ambient_gain;
    float m_global_gain;
    
    KRVector3 m_listener_position;
    KRVector3 m_listener_forward;
    KRVector3 m_listener_up;
    
    unordered_map<std::string, KRAudioSample *> m_sounds;
    
    std::vector<KRDataBlock *> m_bufferPoolIdle;
    
    std::vector<KRAudioBuffer *> m_bufferCache;
    
    std::set<KRAudioSource *> m_activeAudioSources;
    
    void initAudio();
    void initOpenAL();
    void initSiren();
    void initHRTF();
    
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
    float *m_workspace_data;
    DSPSplitComplex m_workspace[3];
    
    
    
    float *getBlockAddress(int block_offset);
    void renderBlock();
    void renderReverb();
    void renderAmbient();
    void renderHRTF();
    void renderITD();
    void renderReverbImpulseResponse(int impulse_response_offset, int frame_count_log2);
    
    std::vector<KRVector2> m_hrtf_sample_locations;
    float *m_hrtf_data;
    unordered_map<KRVector2, DSPSplitComplex> m_hrtf_spectral[2];
    
    KRVector2 getNearestHRTFSample(const KRVector2 &dir);
    void getHRTFMix(const KRVector2 &dir, KRVector2 &hrtf1, KRVector2 &hrtf2, KRVector2 &hrtf3, KRVector2 &hrtf4, float &mix1, float &mix2, float &mix3, float &mix4);
    KRAudioSample *getHRTFSample(const KRVector2 &hrtf_dir);
    DSPSplitComplex getHRTFSpectral(const KRVector2 &hrtf_dir, const int channel);
    
    
    unordered_map<std::string, siren_ambient_zone_weight_info> m_ambient_zone_weights;
    float m_ambient_zone_total_weight = 0.0f; // For normalizing zone weights
    
    unordered_map<std::string, siren_reverb_zone_weight_info> m_reverb_zone_weights;
    float m_reverb_zone_total_weight = 0.0f; // For normalizing zone weights
    
    boost::signals2::mutex m_mutex;
    mach_timebase_info_data_t m_timebase_info;
    
    
    unordered_multimap<KRVector2, std::pair<KRAudioSource *, std::pair<float, float> > > m_mapped_sources, m_prev_mapped_sources;
    bool m_anticlick_block;
    bool m_high_quality_hrtf; // If true, 4 HRTF samples will be interpolated; if false, the nearest HRTF sample will be used without interpolation
};

#endif /* defined(KRAUDIO_MANAGER_H) */
