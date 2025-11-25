//
//  KRAudioSource.cpp
//  Kraken Engine
//
//  Copyright 2025 Kearwood Gilbert. All rights reserved.
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

#include "KRAudioSource.h"
#include "KRContext.h"
#include "resources/audio/KRAudioManager.h"
#include "resources/audio/KRAudioSample.h"
#include "KRAudioBuffer.h"
#include "KRRenderPass.h"

using namespace hydra;

/* static */
void KRAudioSource::InitNodeInfo(KrNodeInfo* nodeInfo)
{
  KRNode::InitNodeInfo(nodeInfo);
  nodeInfo->audio_source.enable_obstruction = decltype(m_enable_obstruction)::defaultVal;
  nodeInfo->audio_source.enable_occlusion = decltype(m_enable_occlusion)::defaultVal;
  nodeInfo->audio_source.gain = decltype(m_gain)::defaultVal;
  nodeInfo->audio_source.is_3d = true;
  nodeInfo->audio_source.looping = decltype(m_looping)::defaultVal;
  nodeInfo->audio_source.pitch = decltype(m_pitch)::defaultVal;
  nodeInfo->audio_source.reference_distance = decltype(m_referenceDistance)::defaultVal;
  nodeInfo->audio_source.reverb = decltype(m_reverb)::defaultVal;
  nodeInfo->audio_source.rolloff_factor = decltype(m_rolloffFactor)::defaultVal;
  nodeInfo->audio_source.sample = -1;
}

KRAudioSource::KRAudioSource(KRScene& scene, std::string name) : KRNode(scene, name)
{
  m_currentBufferFrame = 0;
  m_playing = false;
  m_isPrimed = false;

  m_start_audio_frame = -1;
  m_paused_audio_frame = 0;
}

KRAudioSource::~KRAudioSource()
{
  while (m_audioBuffers.size()) {
    delete m_audioBuffers.front();
    m_audioBuffers.pop();
  }
}

std::string KRAudioSource::getElementName()
{
  return "audio_source";
}

tinyxml2::XMLElement* KRAudioSource::saveXML(tinyxml2::XMLNode* parent)
{
  tinyxml2::XMLElement* e = KRNode::saveXML(parent);
  m_sample.save(e);
  m_gain.save(e);
  m_pitch.save(e);
  m_looping.save(e);
  m_is3d.save(e);
  m_referenceDistance.save(e);
  m_reverb.save(e);
  m_rolloffFactor.save(e);
  m_enable_occlusion.save(e);
  m_enable_obstruction.save(e);
  return e;
}

void KRAudioSource::loadXML(tinyxml2::XMLElement* e)
{
  m_sample.load(e);
  m_gain.load(e);
  m_pitch.load(e);
  m_looping.load(e);
  m_is3d.load(e);
  m_referenceDistance.load(e);
  m_reverb.load(e);
  m_rolloffFactor.load(e);
  m_enable_obstruction.load(e);
  m_enable_occlusion.load(e);

  KRNode::loadXML(e);
}

void KRAudioSource::prime()
{
  if (!m_isPrimed) {
    m_sample.val.bind(&getContext());
    if (m_sample.val.isBound()) {
      // Prime the buffer queue
      m_nextBufferIndex = 0;
      for (int i = 0; i < KRENGINE_AUDIO_BUFFERS_PER_SOURCE; i++) {
        queueBuffer();
      }

      m_isPrimed = true;
    }
  }
}

void KRAudioSource::queueBuffer()
{
  KRAudioBuffer* buffer = m_sample.val.get()->getBuffer(m_nextBufferIndex);
  m_audioBuffers.push(buffer);
  m_nextBufferIndex = (m_nextBufferIndex + 1) % m_sample.val.get()->getBufferCount();
}

void KRAudioSource::render(RenderInfo& ri)
{
  KRNode::render(ri);

  bool bVisualize = false;

  if (ri.renderPass->getType() == RenderPassType::RENDER_PASS_FORWARD_TRANSPARENT && bVisualize) {
    KRMesh* sphereModel = getContext().getMeshManager()->getMesh("__sphere");
    if (sphereModel) {
      Matrix4 sphereModelMatrix = getModelMatrix();

      PipelineInfo info{};
      std::string shader_name("visualize_overlay");
      info.shader_name = &shader_name;
      info.pCamera = ri.camera;
      info.point_lights = &ri.point_lights;
      info.directional_lights = &ri.directional_lights;
      info.spot_lights = &ri.spot_lights;
      info.renderPass = ri.renderPass;
      info.rasterMode = RasterMode::kAdditive;
      info.modelFormat = sphereModel->getModelFormat();
      info.vertexAttributes = sphereModel->getVertexAttributes();

      KRPipeline* pShader = getContext().getPipelineManager()->getPipeline(*ri.surface, info);
      pShader->bind(ri, sphereModelMatrix);

      sphereModel->renderNoMaterials(ri.commandBuffer, ri.renderPass, getName(), "visualize_overlay", 1.0f);
    } // sphereModels.size()
  }
}

void KRAudioSource::setGain(float gain)
{
  m_gain = gain;
}

float KRAudioSource::getGain()
{
  return m_gain;
}

void KRAudioSource::setPitch(float pitch)
{
  m_pitch = pitch;
}


float KRAudioSource::getReferenceDistance()
{
  return m_referenceDistance;
}
void KRAudioSource::setReferenceDistance(float reference_distance)
{
  m_referenceDistance = reference_distance;
}

float KRAudioSource::getReverb()
{
  return m_reverb;
}

void KRAudioSource::setReverb(float reverb)
{
  m_reverb = reverb;
}

float KRAudioSource::getRolloffFactor()
{
  return m_rolloffFactor;
}

void KRAudioSource::setRolloffFactor(float rolloff_factor)
{
  m_rolloffFactor = rolloff_factor;
}

void KRAudioSource::setLooping(bool looping)
{
  // Enable or disable looping playback; Audio source must be stopped and re-started for loop mode changes to take effect
  m_looping = looping;
}

bool KRAudioSource::getLooping()
{
  // Returns true if the playback will automatically loop
  return m_looping;
}

bool KRAudioSource::getEnableOcclusion()
{
  return m_enable_occlusion;
}

void KRAudioSource::setEnableOcclusion(bool enable_occlusion)
{
  m_enable_occlusion = enable_occlusion;
}

bool KRAudioSource::getEnableObstruction()
{
  return m_enable_obstruction;
}

void KRAudioSource::setEnableObstruction(bool enable_obstruction)
{
  m_enable_obstruction = enable_obstruction;
}

bool KRAudioSource::getIs3D()
{
  return m_is3d;
}
void KRAudioSource::setIs3D(bool is3D)
{
  // Audio source must be stopped and re-started for mode change to take effect
  m_is3d = is3D;
}

void KRAudioSource::advanceBuffer()
{
  if (m_audioBuffers.size()) {
    delete m_audioBuffers.front();
    m_audioBuffers.pop();
  }
  queueBuffer();
}

void KRAudioSource::physicsUpdate(float deltaTime)
{
  KRNode::physicsUpdate(deltaTime);

  KRAudioManager* audioManager = getContext().getAudioManager();
  audioManager->activateAudioSource(this);
}

void KRAudioSource::play()
{
  // Start playback of audio at the current audio sample position.  If audio is already playing, this has no effect.
  // play() does not automatically seek to the beginning of the sample.  Call setAudioFrame( 0 ) first if you wish the playback to begin at the start of the audio sample.
  // If not set to looping, audio playback ends automatically at the end of the sample

  if (!m_playing) {
    KRAudioManager* audioManager = getContext().getAudioManager();
    assert(m_start_audio_frame == -1);
    m_start_audio_frame = audioManager->getAudioFrame() - m_paused_audio_frame;
    m_paused_audio_frame = -1;
    audioManager->activateAudioSource(this);
  }
  m_playing = true;
}

void KRAudioSource::stop()
{
  // Stop playback of audio.  If audio is already stopped, this has no effect.
  // If play() is called afterwards, playback will continue at the current audio sample position.

  if (m_playing) {
    m_paused_audio_frame = getAudioFrame();
    m_start_audio_frame = -1;
    m_playing = false;
    getContext().getAudioManager()->deactivateAudioSource(this);
  }
}

bool KRAudioSource::isPlaying()
{
  // Returns true if audio is playing.  Will return false if a non-looped playback has reached the end of the audio sample.

  return m_playing;
}


void KRAudioSource::setSample(const std::string& sound_name)
{
  m_sample.val.set(sound_name);
}

std::string KRAudioSource::getSample()
{
  return m_sample.val.getName();
}

KRAudioSample* KRAudioSource::getAudioSample()
{
  m_sample.val.bind(&getContext());
  return m_sample.val.get();
}

void KRAudioSource::advanceFrames(int frame_count)
{
  m_currentBufferFrame += frame_count;

  KRAudioBuffer* buffer = getBuffer();
  while (buffer != NULL && m_currentBufferFrame >= buffer->getFrameCount()) {
    m_currentBufferFrame -= buffer->getFrameCount();
    advanceBuffer();
    buffer = getBuffer();
  }

  if (buffer == NULL) {
    m_currentBufferFrame = 0;
    stop();
  }
}

KRAudioBuffer* KRAudioSource::getBuffer()
{
  if (m_playing) {
    prime();
    return m_audioBuffers.front();
  } else {
    return NULL;
  }
}

int KRAudioSource::getBufferFrame()
{
  return m_currentBufferFrame;
}

__int64_t KRAudioSource::getAudioFrame()
{
  // Returns the audio playback position in units of integer audio frames.

  if (m_playing) {
    return getContext().getAudioManager()->getAudioFrame() - m_start_audio_frame;
  } else {
    return m_paused_audio_frame;
  }
}

void KRAudioSource::setAudioFrame(__int64_t next_frame)
{
  // Sets the audio playback position with units of integer audio frames.
  if (m_playing) {
    m_start_audio_frame = getContext().getAudioManager()->getAudioFrame() - next_frame;
  } else {
    m_paused_audio_frame = next_frame;
  }
}


float KRAudioSource::getAudioTime()
{
  // Gets the audio playback position with units of floating point seconds.

  return getAudioFrame() / 44100.0f;
}

void KRAudioSource::setAudioTime(float new_position)
{
  // Sets the audio playback position with units of floating point seconds.
  setAudioFrame((__int64_t)(new_position * 44100.0f));
}

void KRAudioSource::sample(int frame_count, int channel, float* buffer, float gain)
{
  KRAudioSample* source_sample = getAudioSample();
  if (source_sample && m_playing) {
    __int64_t next_frame = getAudioFrame();
    source_sample->sample(next_frame, frame_count, channel, buffer, gain, m_looping);
    if (!m_looping && next_frame > source_sample->getFrameCount()) {
      stop();
    }
  } else {
    memset(buffer, 0, sizeof(float) * frame_count);
  }
}
