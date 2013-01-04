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

KRAudioSource::KRAudioSource(KRScene &scene, std::string name) : KRNode(scene, name)
{
    
}

KRAudioSource::~KRAudioSource()
{
}

std::string KRAudioSource::getElementName() {
    return "audio_source";
}

tinyxml2::XMLElement *KRAudioSource::saveXML( tinyxml2::XMLNode *parent)
{
    tinyxml2::XMLElement *e = KRNode::saveXML(parent);
    e->SetAttribute("sound", m_sound.c_str());
    return e;
}

void KRAudioSource::loadXML(tinyxml2::XMLElement *e)
{
    m_sound = e->Attribute("sound");
    KRNode::loadXML(e);
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

void KRAudioSource::play()
{
    
}

void KRAudioSource::stop()
{
    
}

bool KRAudioSource::isPlaying()
{
    return false;
}


void KRAudioSource::setSound(const std::string &sound_name)
{
    m_sound = sound_name;
}

std::string KRAudioSource::getSound()
{
    return m_sound;
}
