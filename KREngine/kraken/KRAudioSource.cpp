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

OSStatus alcASASetSourceProc(const ALuint property, ALuint source, ALvoid *data, ALuint dataSize);

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
    
    m_referenceDistance = 1.0f;
    m_reverb = 0.0f;
    m_rolloffFactor = 2.0f;
    m_enable_occlusion = true;
    m_enable_obstruction = true;
}

KRAudioSource::~KRAudioSource()
{
    if(m_sourceID) {
        getContext().getAudioManager()->makeCurrentContext();
        ALDEBUG(alDeleteSources(1, &m_sourceID));
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
    e->SetAttribute("is3d", m_is3d ? "true" : "false");
    e->SetAttribute("reference_distance", &m_referenceDistance);
    e->SetAttribute("reverb", &m_reverb);
    e->SetAttribute("rolloff_factor", &m_rolloffFactor);
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
            getContext().getAudioManager()->makeCurrentContext();
            
            // Initialize audio source
            m_sourceID = 0;
            ALDEBUG(alGenSources(1, &m_sourceID));
            
            // Prime the buffer queue
            m_nextBufferIndex = 0;
            for(int i=0; i < KRENGINE_AUDIO_BUFFERS_PER_SOURCE; i++) {
                queueBuffer();
            }
            
            //alSourcei(_sourceID, AL_BUFFER, firstBuffer.bufferID);
            ALDEBUG(alSourcef(m_sourceID, AL_PITCH, m_pitch));
            ALDEBUG(alSourcei(m_sourceID, AL_LOOPING, m_looping && m_audioFile->getBufferCount() == 1));
            ALDEBUG(alSourcef(m_sourceID, AL_GAIN, m_gain));
            
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
    ALDEBUG(alSourceQueueBuffers(m_sourceID, 1, buffer_ids));
    
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
            std::vector<KRMesh *> sphereModels = getContext().getModelManager()->getModel("__sphere");
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


float KRAudioSource::getReferenceDistance()
{
    return m_referenceDistance;
}
void KRAudioSource::setReferenceDistance(float reference_distance)
{
    m_referenceDistance = reference_distance;
    if(m_isPrimed && m_is3d) {
        getContext().getAudioManager()->makeCurrentContext();
        ALDEBUG(alSourcef(m_sourceID, AL_REFERENCE_DISTANCE, m_referenceDistance));
    }
}

float KRAudioSource::getReverb()
{
    return m_reverb;
}

void KRAudioSource::setReverb(float reverb)
{
    m_reverb = reverb;
    if(m_isPrimed && m_is3d) {
        getContext().getAudioManager()->makeCurrentContext();
        ALDEBUG(alcASASetSourceProc(ALC_ASA_REVERB_SEND_LEVEL, m_sourceID, &m_reverb, sizeof(m_reverb)));
    }
}


float KRAudioSource::getRolloffFactor()
{
    return m_rolloffFactor;
}

void KRAudioSource::setRolloffFactor(float rolloff_factor)
{
    m_rolloffFactor = rolloff_factor;
    if(m_isPrimed && m_is3d) {
        getContext().getAudioManager()->makeCurrentContext();
        ALDEBUG(alSourcef(m_sourceID, AL_ROLLOFF_FACTOR, m_rolloffFactor));
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

bool KRAudioSource::hasPhysics()
{
    return true;
}

void KRAudioSource::physicsUpdate(float deltaTime)
{
    if(m_isPrimed && m_playing) {
        getContext().getAudioManager()->makeCurrentContext();
        updatePosition();
        ALint processed_count = 0;
        ALDEBUG(alGetSourcei(m_sourceID, AL_BUFFERS_PROCESSED, &processed_count));
        while(processed_count-- > 0) {
            ALuint finished_buffer = 0;
            ALDEBUG(alSourceUnqueueBuffers(m_sourceID, 1, &finished_buffer));
            delete m_audioBuffers.front();
            m_audioBuffers.pop();
            queueBuffer();
        }
        
        ALint val;
        // Make sure the source is still playing, and restart it if needed.
        ALDEBUG(alGetSourcei(m_sourceID, AL_SOURCE_STATE, &val));
        ALDEBUG(if(val != AL_PLAYING) alSourcePlay(m_sourceID));
    }
}

void KRAudioSource::play()
{
    getContext().getAudioManager()->makeCurrentContext();
    prime();
    updatePosition();

    if(m_is3d) {
        ALDEBUG(alSource3f(m_sourceID, AL_VELOCITY, 0.0f, 0.0f, 0.0f));
        ALDEBUG(alSourcef(m_sourceID, AL_REFERENCE_DISTANCE, m_referenceDistance));
        ALDEBUG(alSourcef(m_sourceID, AL_ROLLOFF_FACTOR, m_rolloffFactor));
        ALDEBUG(alcASASetSourceProc(ALC_ASA_REVERB_SEND_LEVEL, m_sourceID, &m_reverb, sizeof(m_reverb)));
        ALDEBUG(alSourcei(m_sourceID, AL_SOURCE_RELATIVE, AL_FALSE));
    } else {
        ALDEBUG(alSourcei(m_sourceID, AL_SOURCE_RELATIVE, AL_TRUE));
        ALDEBUG(alSource3f(m_sourceID, AL_POSITION, 0.0, 0.0, 0.0));
    }
    ALDEBUG(alSourcePlay(m_sourceID));
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

void KRAudioSource::updatePosition()
{
    if(m_is3d) {
        ALfloat occlusion = 0.0f; // type ALfloat	-100.0 db (most occlusion) - 0.0 db (no occlusion, 0.0 default)
        ALfloat obstruction = 0.0f; // type ALfloat	-100.0 db (most obstruction) - 0.0 db (no obstruction, 0.0 default)
        
        KRVector3 worldPosition = getWorldTranslation();
        ALDEBUG(alSource3f(m_sourceID, AL_POSITION, worldPosition.x, worldPosition.y, worldPosition.z));
        ALDEBUG(alSourcef(m_sourceID, AL_GAIN, m_gain));
        ALDEBUG(alSourcef(m_sourceID, AL_MIN_GAIN, 0.0));
        ALDEBUG(alSourcef(m_sourceID, AL_MAX_GAIN, 1.0));
        
        ALDEBUG(alcASASetSourceProc(ALC_ASA_OCCLUSION, m_sourceID, &occlusion, sizeof(occlusion)));
        ALDEBUG(alcASASetSourceProc(ALC_ASA_OBSTRUCTION, m_sourceID, &obstruction, sizeof(obstruction)));
        ALDEBUG(alcASASetSourceProc(ALC_ASA_REVERB_SEND_LEVEL, m_sourceID, &m_reverb, sizeof(m_reverb)));
        

    }
}


OSStatus alcASASetSourceProc(const ALuint property, ALuint source, ALvoid *data, ALuint dataSize)
{
    OSStatus	err = noErr;
	static	alcASASetSourceProcPtr	proc = NULL;
    
    if (proc == NULL) {
        proc = (alcASASetSourceProcPtr) alcGetProcAddress(NULL, (const ALCchar*) "alcASASetSource");
    }
    
    if (proc)
        err = proc(property, source, data, dataSize);
    return (err);
}

