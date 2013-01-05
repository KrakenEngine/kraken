//
//  KRAudioSource.cpp
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

#include "KRAudioSource.h"
#include "KRContext.h"
#include "KRAudioManager.h"
#include "KRAudioSample.h"
#include "KRAudioBuffer.h"

KRAudioSource::KRAudioSource(KRScene &scene, std::string name) : KRNode(scene, name)
{
    m_playing = false;
    m_is3d = true;
    m_isPrimed = false;
    m_audioFile = NULL;
    m_sourceID = 0;
    m_gain = 1.0f;
    m_pitch = 1.0f;
    m_looping = false;
}

KRAudioSource::~KRAudioSource()
{
    if(m_sourceID) {
        getContext().getAudioManager()->makeCurrentContext();
        alDeleteSources(1, &m_sourceID);
        m_sourceID = 0;
    }
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
    
    KRNode::loadXML(e);
}

void KRAudioSource::prime()
{
    if(!m_isPrimed) {
        if(m_audioFile == NULL && m_audio_sample_name.size() != 0) {
            m_audioFile = getContext().getAudioManager()->get(m_audio_sample_name);
        }
        if(m_audioFile) {
            getContext().getAudioManager()->makeCurrentContext();
            
            // Initialize audio source
            m_sourceID = 0;
            alGenSources(1, &m_sourceID);
            
            // Prime the buffer queue
            m_nextBufferIndex = 0;
            for(int i=0; i < KRENGINE_AUDIO_BUFFERS_PER_SOURCE; i++) {
                queueBuffer();
            }
            
            //alSourcei(_sourceID, AL_BUFFER, firstBuffer.bufferID);
            alSourcef(m_sourceID, AL_PITCH, m_pitch);
            alSourcei(m_sourceID, AL_LOOPING, m_looping && m_audioFile->getBufferCount() == 1);
            alSourcef(m_sourceID, AL_GAIN, m_gain);
            
            m_isPrimed = true;
        }
    }
}

void KRAudioSource::queueBuffer()
{
    KRAudioBuffer *buffer = m_audioFile->getBuffer(m_nextBufferIndex);
    m_audioBuffers.push(buffer);
    ALuint buffer_ids[1];
    buffer_ids[0] = buffer->getBufferID();
    alSourceQueueBuffers(m_sourceID, 1, buffer_ids);
    
    m_nextBufferIndex = (m_nextBufferIndex + 1) % m_audioFile->getBufferCount();
}

void KRAudioSource::render(KRCamera *pCamera, std::vector<KRLight *> &lights, const KRViewport &viewport, KRNode::RenderPass renderPass)
{
    
    KRNode::render(pCamera, lights, viewport, renderPass);
    
    bool bVisualize = false;
    
    if(renderPass == KRNode::RENDER_PASS_FORWARD_TRANSPARENT && bVisualize) {
        KRMat4 sphereModelMatrix = getModelMatrix();
        
        KRShader *pShader = getContext().getShaderManager()->getShader("visualize_overlay", pCamera, lights, 0, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, renderPass);
        
        if(getContext().getShaderManager()->selectShader(*pCamera, pShader, viewport, sphereModelMatrix, lights, 0, renderPass)) {
            
            // Enable additive blending
            GLDEBUG(glEnable(GL_BLEND));
            GLDEBUG(glBlendFunc(GL_ONE, GL_ONE));
            
            
            // Disable z-buffer write
            GLDEBUG(glDepthMask(GL_FALSE));
            
            // Enable z-buffer test
            GLDEBUG(glEnable(GL_DEPTH_TEST));
            GLDEBUG(glDepthFunc(GL_LEQUAL));
            GLDEBUG(glDepthRangef(0.0, 1.0));
            std::vector<KRModel *> sphereModels = getContext().getModelManager()->getModel("__sphere");
            if(sphereModels.size()) {
                for(int i=0; i < sphereModels[0]->getSubmeshCount(); i++) {
                    sphereModels[0]->renderSubmesh(i);
                }
            }
            
            // Enable alpha blending
            GLDEBUG(glEnable(GL_BLEND));
            GLDEBUG(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        }
    }
}

void KRAudioSource::setGain(float gain)
{
    m_gain = gain;
    if(m_isPrimed) {
        getContext().getAudioManager()->makeCurrentContext();
        alSourcef(m_sourceID, AL_GAIN, m_gain);
    }
}

float KRAudioSource::getGain()
{
    return m_gain;
}

void KRAudioSource::setPitch(float pitch)
{
    m_pitch = pitch;
    if(m_isPrimed ) {
        getContext().getAudioManager()->makeCurrentContext();
        alSourcef(m_sourceID, AL_PITCH, m_pitch);
        
    }
}

void KRAudioSource::setLooping(bool looping)
{
    m_looping = looping;
    // Audio source must be stopped and re-started for loop mode changes to take effect
}

bool KRAudioSource::getLooping()
{
    return m_looping;
}

bool KRAudioSource::hasPhysics()
{
    return true;
}

void KRAudioSource::physicsUpdate(float deltaTime)
{
    if(m_isPrimed && m_playing) {
        getContext().getAudioManager()->makeCurrentContext();
        ALint processed_count = 0;
        alGetSourcei(m_sourceID, AL_BUFFERS_PROCESSED, &processed_count);
        while(processed_count-- > 0) {
            ALuint finished_buffer = 0;
            alSourceUnqueueBuffers(m_sourceID, 1, &finished_buffer);
            delete m_audioBuffers.front();
            m_audioBuffers.pop();
            queueBuffer();
        }
        
        ALint val;
        // Make sure the source is still playing, and restart it if needed.
        alGetSourcei(m_sourceID, AL_SOURCE_STATE, &val);
        if(val != AL_PLAYING) alSourcePlay(m_sourceID);
    }
}

void KRAudioSource::play()
{
    prime();
    getContext().getAudioManager()->makeCurrentContext();

    alSourcePlay(m_sourceID);
    m_playing = true;
}

void KRAudioSource::stop()
{
    
}

bool KRAudioSource::isPlaying()
{
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
