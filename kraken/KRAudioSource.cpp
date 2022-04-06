//
//  KRAudioSource.cpp
//  Kraken Engine
//
//  Copyright 2022 Kearwood Gilbert. All rights reserved.
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
#include "KRAudioManager.h"
#include "KRAudioSample.h"
#include "KRAudioBuffer.h"

/* static */
void KRAudioSource::InitNodeInfo(KrNodeInfo* nodeInfo)
{
  KRNode::InitNodeInfo(nodeInfo);
  nodeInfo->audio_source.enable_obstruction = true;
  nodeInfo->audio_source.enable_occlusion = true;
  nodeInfo->audio_source.gain = 1.0f;
  nodeInfo->audio_source.is_3d = true;
  nodeInfo->audio_source.looping = false;
  nodeInfo->audio_source.pitch = 1.0f;
  nodeInfo->audio_source.reference_distance = 1.0f;
  nodeInfo->audio_source.reverb = 0.0f;
  nodeInfo->audio_source.rolloff_factor = 2.0f;
  nodeInfo->audio_source.sample = -1;
}

KRAudioSource::KRAudioSource(KRScene &scene, std::string name) : KRNode(scene, name)
{
    m_currentBufferFrame = 0;
    m_playing = false;
    m_is3d = true;
    m_isPrimed = false;
    m_audioFile = NULL;
    m_gain = 1.0f;
    m_pitch = 1.0f;
    m_looping = false;
    
    m_referenceDistance = 1.0f;
    m_reverb = 0.0f;
    m_rolloffFactor = 2.0f;
    m_enable_occlusion = true;
    m_enable_obstruction = true;
    
    m_start_audio_frame = -1;
    m_paused_audio_frame = 0;
}

KRAudioSource::~KRAudioSource()
{
    while(m_audioBuffers.size()) {
        delete m_audioBuffers.front();
        m_audioBuffers.pop();
    }
}

std::string KRAudioSource::getElementName() {
    return "audio_source";
}

tinyxml2::XMLElement *KRAudioSource::saveXML( tinyxml2::XMLNode *parent)
{
    tinyxml2::XMLElement *e = KRNode::saveXML(parent);
    e->SetAttribute("sample", m_audio_sample_name.c_str());
    e->SetAttribute("gain", m_gain);
    e->SetAttribute("pitch", m_pitch);
    e->SetAttribute("looping", m_looping ? "true" : "false");
    e->SetAttribute("is3d", m_is3d ? "true" : "false");
    e->SetAttribute("reference_distance", m_referenceDistance);
    e->SetAttribute("reverb", m_reverb);
    e->SetAttribute("rolloff_factor", m_rolloffFactor);
    e->SetAttribute("enable_occlusion", m_enable_occlusion ? "true" : "false");
    e->SetAttribute("enable_obstruction", m_enable_obstruction ? "true" : "false");
    return e;
}

void KRAudioSource::loadXML(tinyxml2::XMLElement *e)
{
    m_audio_sample_name = e->Attribute("sample");
    
    float gain = 1.0f;
    if(e->QueryFloatAttribute("gain", &gain)  != tinyxml2::XML_SUCCESS) {
        gain = 1.0f;
    }
    setGain(gain);
    
    float pitch = 1.0f;
    if(e->QueryFloatAttribute("pitch", &pitch) != tinyxml2::XML_SUCCESS) {
        pitch = 1.0f;
    }
    setPitch(m_pitch);
    
    bool looping = false;
    if(e->QueryBoolAttribute("looping", &looping) != tinyxml2::XML_SUCCESS) {
        looping = false;
    }
    setLooping(looping);
    
    bool is3d = true;
    if(e->QueryBoolAttribute("is3d", &is3d) != tinyxml2::XML_SUCCESS) {
        is3d = true;
    }
    setIs3D(is3d);
    
    float reference_distance = 1.0f;
    if(e->QueryFloatAttribute("reference_distance", &reference_distance) != tinyxml2::XML_SUCCESS) {
        reference_distance = 1.0f;
    }
    setReferenceDistance(reference_distance);
    
    float reverb = 0.0f;
    if(e->QueryFloatAttribute("reverb", &reverb) != tinyxml2::XML_SUCCESS) {
        reverb = 0.0f;
    }
    setReverb(reverb);
    
    float rolloff_factor = 2.0f;
    if(e->QueryFloatAttribute("rolloff_factor", &rolloff_factor) != tinyxml2::XML_SUCCESS) {
        rolloff_factor = 2.0f;
    }
    setRolloffFactor(rolloff_factor);
    
    m_enable_obstruction = true;
    if(e->QueryBoolAttribute("enable_obstruction", &m_enable_obstruction) != tinyxml2::XML_SUCCESS) {
        m_enable_obstruction = true;
    }

    m_enable_occlusion = true;
    if(e->QueryBoolAttribute("enable_occlusion", &m_enable_occlusion) != tinyxml2::XML_SUCCESS) {
        m_enable_occlusion = true;
    }
    
    KRNode::loadXML(e);
}

void KRAudioSource::prime()
{
    if(!m_isPrimed) {
        if(m_audioFile == NULL && m_audio_sample_name.size() != 0) {
            m_audioFile = getContext().getAudioManager()->get(m_audio_sample_name);
        }
        if(m_audioFile) {
            // Prime the buffer queue
            m_nextBufferIndex = 0;
            for(int i=0; i < KRENGINE_AUDIO_BUFFERS_PER_SOURCE; i++) {
                queueBuffer();
            }
            
            m_isPrimed = true;
        }
    }
}

void KRAudioSource::queueBuffer()
{
    KRAudioBuffer *buffer = m_audioFile->getBuffer(m_nextBufferIndex);
    m_audioBuffers.push(buffer);
    m_nextBufferIndex = (m_nextBufferIndex + 1) % m_audioFile->getBufferCount();
}

void KRAudioSource::render(VkCommandBuffer& commandBuffer, KRCamera *pCamera, std::vector<KRPointLight *> &point_lights, std::vector<KRDirectionalLight *> &directional_lights, std::vector<KRSpotLight *>&spot_lights, const KRViewport &viewport, KRNode::RenderPass renderPass)
{
    
    if(m_lod_visible <= LOD_VISIBILITY_PRESTREAM) return;
    
    KRNode::render(commandBuffer, pCamera, point_lights, directional_lights, spot_lights, viewport, renderPass);
    
    bool bVisualize = false;
    
    if(renderPass == KRNode::RENDER_PASS_FORWARD_TRANSPARENT && bVisualize) {
        Matrix4 sphereModelMatrix = getModelMatrix();

        KRPipelineManager::PipelineInfo info{};
        std::string shader_name("visualize_overlay");
        info.shader_name = &shader_name;
        info.pCamera = pCamera;
        info.point_lights = &point_lights;
        info.directional_lights = &directional_lights;
        info.spot_lights = &spot_lights;
        info.renderPass = renderPass;
        
        KRPipeline *pShader = getContext().getPipelineManager()->getPipeline(info);
        
        if(getContext().getPipelineManager()->selectPipeline(*pCamera, pShader, viewport, sphereModelMatrix, &point_lights, &directional_lights, &spot_lights, 0, renderPass, Vector3::Zero(), 0.0f, Vector4::Zero())) {
            
            // Enable additive blending
            GLDEBUG(glEnable(GL_BLEND));
            GLDEBUG(glBlendFunc(GL_ONE, GL_ONE));
            
            
            // Disable z-buffer write
            GLDEBUG(glDepthMask(GL_FALSE));
            
            // Enable z-buffer test
            GLDEBUG(glEnable(GL_DEPTH_TEST));
            GLDEBUG(glDepthFunc(GL_LEQUAL));
            GLDEBUG(glDepthRangef(0.0, 1.0));
            std::vector<KRMesh *> sphereModels = getContext().getMeshManager()->getModel("__sphere");
            if(sphereModels.size()) {
                for(int i=0; i < sphereModels[0]->getSubmeshCount(); i++) {
                    sphereModels[0]->renderSubmesh(commandBuffer, i, renderPass, getName(), "visualize_overlay", 1.0f);
                }
            }
            
            // Enable alpha blending
            GLDEBUG(glEnable(GL_BLEND));
            GLDEBUG(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
        }
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
    if(m_audioBuffers.size()) {
        delete m_audioBuffers.front();
        m_audioBuffers.pop();
    }
    queueBuffer();
}

void KRAudioSource::physicsUpdate(float deltaTime)
{
    KRNode::physicsUpdate(deltaTime);
    
    KRAudioManager *audioManager = getContext().getAudioManager();
    audioManager->activateAudioSource(this);
}

void KRAudioSource::play()
{
    // Start playback of audio at the current audio sample position.  If audio is already playing, this has no effect.
    // play() does not automatically seek to the beginning of the sample.  Call setAudioFrame( 0 ) first if you wish the playback to begin at the start of the audio sample.
    // If not set to looping, audio playback ends automatically at the end of the sample
    
    if(!m_playing) {
        KRAudioManager *audioManager = getContext().getAudioManager();
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
    
    if(m_playing) {
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


void KRAudioSource::setSample(const std::string &sound_name)
{
    m_audio_sample_name = sound_name;
}

std::string KRAudioSource::getSample()
{
    return m_audio_sample_name;
}

KRAudioSample *KRAudioSource::getAudioSample()
{
    if(m_audioFile == NULL && m_audio_sample_name.size() != 0) {
        m_audioFile = getContext().getAudioManager()->get(m_audio_sample_name);
    }
    return m_audioFile;
}

void KRAudioSource::advanceFrames(int frame_count)
{
    m_currentBufferFrame += frame_count;
    
    KRAudioBuffer *buffer = getBuffer();
    while(buffer != NULL && m_currentBufferFrame >= buffer->getFrameCount()) {
        m_currentBufferFrame -= buffer->getFrameCount();
        advanceBuffer();
        buffer = getBuffer();
    }
    
    if(buffer == NULL) {
        m_currentBufferFrame = 0;
        stop();
    }
}

KRAudioBuffer *KRAudioSource::getBuffer()
{
    if(m_playing) {
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
    
    if(m_playing) {
        return getContext().getAudioManager()->getAudioFrame() - m_start_audio_frame;
    } else {
        return m_paused_audio_frame;
    }
}

void KRAudioSource::setAudioFrame(__int64_t next_frame)
{
    // Sets the audio playback position with units of integer audio frames.
    if(m_playing) {
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

void KRAudioSource::sample(int frame_count, int channel, float *buffer, float gain)
{
    KRAudioSample *source_sample = getAudioSample();
    if(source_sample && m_playing) {
        __int64_t next_frame = getAudioFrame();
        source_sample->sample(next_frame, frame_count, channel, buffer, gain, m_looping);
        if(!m_looping && next_frame > source_sample->getFrameCount()) {
            stop();
        }
    } else {
        memset(buffer, 0, sizeof(float) * frame_count);
    }
}
