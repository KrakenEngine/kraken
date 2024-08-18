//
//  FileManager.h
//  Kraken Engine
//
//  Copyright 2024 Kearwood Gilbert. All rights reserved.
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

#pragma once

#include "KREngine-common.h"

#include "resources/KRResourceManager.h"

#include "KRContextObject.h"
#include "block.h"
#include "nodes/KRAudioSource.h"
#include "siren.h"

const int KRENGINE_AUDIO_MAX_POOL_SIZE = 60; //32;
    // for Circa we play a maximum of 11 mono audio streams at once + cross fading with ambient
    // so we could safely say a maximum of 12 or 13 streams, which would be 39 buffers
    // do the WAV files for the reverb use the same buffer pool ???

const int KRENGINE_AUDIO_MAX_BUFFER_SIZE = 5120;  // in bytes
    // this is the buffer for our decoded audio (not the source file data)
    // it should be greater then 1152 samples (the size of an mp3 frame in samples)
    // so it should be greater then 2304 bytes and also a multiple of 128 samples (to make
    // the data flow efficient) but it shouldn't be too large or it will cause
    // the render loop to stall out decoding large chunks of mp3 data.
    // 2560 bytes would be the smallest size for mono sources, and 5120 would be smallest for stereo.

const int KRENGINE_AUDIO_BUFFERS_PER_SOURCE = 3;

const int KRENGINE_AUDIO_BLOCK_LOG2N = 7;   // 2 ^ KRENGINE_AUDIO_BLOCK_LOG2N = KRENGINE_AUDIO_BLOCK_LENGTH
    // 7 is 128 .. NOTE: the hrtf code uses magic numbers everywhere and is hardcoded to 128 samples per frame

const int KRENGINE_AUDIO_BLOCK_LENGTH = 1 << KRENGINE_AUDIO_BLOCK_LOG2N;
// Length of one block to process.  Determines the latency of the audio system and sets size for FFT's used in HRTF convolution
// the AUGraph works in 1024 sample chunks. At 128 we are making 8 consecutive calls to the renderBlock method for each
// render initiated by the AUGraph.

const int KRENGINE_REVERB_MAX_FFT_LOG2 = 15;
const int KRENGINE_REVERB_WORKSPACE_SIZE = 1 << KRENGINE_REVERB_MAX_FFT_LOG2;

const float KRENGINE_AUDIO_CUTOFF = 0.02f; // Cutoff gain level, to cull out processing of very quiet sounds

const int KRENGINE_REVERB_MAX_SAMPLES = 128000; // 2.9 seconds //435200; // At least 10s reverb impulse response length, divisible by KRENGINE_AUDIO_BLOCK_LENGTH
const int KRENGINE_MAX_REVERB_IMPULSE_MIX = 8; // Maximum number of impulse response filters that can be mixed simultaneously
const int KRENGINE_MAX_OUTPUT_CHANNELS = 2;

const int KRENGINE_MAX_ACTIVE_SOURCES = 16;
const int KRENGINE_AUDIO_ANTICLICK_SAMPLES = 64;


class KRAmbientZone;
class KRReverbZone;

typedef struct
{
  float weight;
  KRAmbientZone* ambient_zone;
  KRAudioSample* ambient_sample;
} siren_ambient_zone_weight_info;

typedef struct
{
  float weight;
  KRReverbZone* reverb_zone;
  KRAudioSample* reverb_sample;
} siren_reverb_zone_weight_info;

class KRAudioManager : public KRResourceManager
{
public:
  KRAudioManager(KRContext& context);
  virtual ~KRAudioManager();
  void destroy();

  virtual KRResource* loadResource(const std::string& name, const std::string& extension, mimir::Block* data) override;
  virtual KRResource* getResource(const std::string& name, const std::string& extension) override;

  unordered_map<std::string, KRAudioSample*>& getSounds();

  void add(KRAudioSample* Sound);

  KRAudioSample* load(const std::string& name, const std::string& extension, mimir::Block* data);
  KRAudioSample* get(const std::string& name);

  // Listener position and orientation
  KRScene* getListenerScene();
  void setListenerScene(KRScene* scene);
  void setListenerOrientation(const hydra::Vector3& position, const hydra::Vector3& forward, const hydra::Vector3& up);
  void setListenerOrientationFromModelMatrix(const hydra::Matrix4& modelMatrix);
  hydra::Vector3& getListenerForward();
  hydra::Vector3& getListenerPosition();
  hydra::Vector3& getListenerUp();


  // Global audio gain / attenuation
  float getGlobalGain();
  void setGlobalGain(float gain);
  float getGlobalReverbSendLevel();
  void setGlobalReverbSendLevel(float send_level);
  float getGlobalAmbientGain();
  void setGlobalAmbientGain(float gain);



  void makeCurrentContext();

  mimir::Block* getBufferData(int size);
  void recycleBufferData(mimir::Block* data);

  void activateAudioSource(KRAudioSource* audioSource);
  void deactivateAudioSource(KRAudioSource* audioSource);

  __int64_t getAudioFrame();

  KRAudioBuffer* getBuffer(KRAudioSample& audio_sample, int buffer_index);

  static void mute(bool onNotOff);
  void goToSleep();

  void startFrame(float deltaTime);

  bool getEnableAudio();
  void setEnableAudio(bool enable);

  bool getEnableHRTF();
  void setEnableHRTF(bool enable);

  bool getEnableReverb();
  void setEnableReverb(bool enable);

  float getReverbMaxLength();
  void setReverbMaxLength(float max_length);

  void _registerOpenAudioSample(KRAudioSample* audioSample);
  void _registerCloseAudioSample(KRAudioSample* audioSample);

private:
  bool m_enable_audio;
  bool m_enable_hrtf;
  bool m_enable_reverb;
  float m_reverb_max_length;

  KRScene* m_listener_scene; // For now, only one scene is allowed to have active audio at once

  float m_global_reverb_send_level;
  float m_global_ambient_gain;
  float m_global_gain;

  hydra::Vector3 m_listener_position;
  hydra::Vector3 m_listener_forward;
  hydra::Vector3 m_listener_up;

  unordered_map<std::string, KRAudioSample*> m_sounds;

  std::vector<mimir::Block*> m_bufferPoolIdle;

  std::vector<KRAudioBuffer*> m_bufferCache;

  std::set<KRAudioSource*> m_activeAudioSources;

  std::set<KRAudioSample*> m_openAudioSamples;

  void initAudio();
  void initHRTF();

  void cleanupAudio();

  bool m_initialized;

#ifdef __APPLE__
  // Apple Core Audio
  AUGraph m_auGraph;
  AudioUnit m_auMixer;

  static OSStatus renderInput(void* inRefCon, AudioUnitRenderActionFlags* ioActionFlags, const AudioTimeStamp* inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList* ioData);
  void renderAudio(UInt32 inNumberFrames, AudioBufferList* ioData);
#endif

  siren::dsp::FFTWorkspace m_fft_setup[KRENGINE_REVERB_MAX_FFT_LOG2 - KRENGINE_AUDIO_BLOCK_LOG2N + 1];

  __int64_t m_audio_frame; // Number of audio frames processed since the start of the application

  float* m_reverb_input_samples; // Circular-buffered reverb input, single channel
  int m_reverb_input_next_sample; // Pointer to next sample in reverb buffer
  int m_reverb_sequence;

  KRAudioSample* m_reverb_impulse_responses[KRENGINE_MAX_REVERB_IMPULSE_MIX];
  float m_reverb_impulse_responses_weight[KRENGINE_MAX_REVERB_IMPULSE_MIX];

  float* m_output_accumulation; // Interleaved output accumulation buffer
  int m_output_accumulation_block_start;
  int m_output_sample;

  float* m_workspace_data;
  siren::dsp::SplitComplex m_workspace[3];

  float* getBlockAddress(int block_offset);
  void renderBlock();
  void renderReverb();
  void renderAmbient();
  void renderHRTF();
  void renderITD();
  void renderReverbImpulseResponse(int impulse_response_offset, int frame_count_log2);
  void renderLimiter();

  std::vector<hydra::Vector2> m_hrtf_sample_locations;
  float* m_hrtf_data;
  unordered_map<hydra::Vector2, siren::dsp::SplitComplex> m_hrtf_spectral[2];

  hydra::Vector2 getNearestHRTFSample(const hydra::Vector2& dir);
  void getHRTFMix(const hydra::Vector2& dir, hydra::Vector2& hrtf1, hydra::Vector2& hrtf2, hydra::Vector2& hrtf3, hydra::Vector2& hrtf4, float& mix1, float& mix2, float& mix3, float& mix4);
  KRAudioSample* getHRTFSample(const hydra::Vector2& hrtf_dir);
  siren::dsp::SplitComplex getHRTFSpectral(const hydra::Vector2& hrtf_dir, const int channel);

  unordered_map<std::string, siren_ambient_zone_weight_info> m_ambient_zone_weights;
  float m_ambient_zone_total_weight = 0.0f; // For normalizing zone weights

  unordered_map<std::string, siren_reverb_zone_weight_info> m_reverb_zone_weights;
  float m_reverb_zone_total_weight = 0.0f; // For normalizing zone weights

  std::mutex m_mutex;
#ifdef __APPLE__
  mach_timebase_info_data_t m_timebase_info;
#endif


  unordered_multimap<hydra::Vector2, std::pair<KRAudioSource*, std::pair<float, float> > > m_mapped_sources, m_prev_mapped_sources;
  bool m_anticlick_block;
  bool m_high_quality_hrtf; // If true, 4 HRTF samples will be interpolated; if false, the nearest HRTF sample will be used without interpolation
};
