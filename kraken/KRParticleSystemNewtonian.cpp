//
//  KRParticleSystemNewtonian.cpp
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


#include "KREngine-common.h"
#include "KRParticleSystemNewtonian.h"
#include "KRTexture.h"
#include "KRContext.h"

KRParticleSystemNewtonian::KRParticleSystemNewtonian(KRScene &scene, std::string name) : KRParticleSystem(scene, name)
{
    m_particlesAbsoluteTime = 0.0f;
}

KRParticleSystemNewtonian::~KRParticleSystemNewtonian()
{
    
}

std::string KRParticleSystemNewtonian::getElementName()
{
    return "newtonian_particles";
}

void KRParticleSystemNewtonian::loadXML(tinyxml2::XMLElement *e)
{
    KRParticleSystem::loadXML(e);
}

tinyxml2::XMLElement *KRParticleSystemNewtonian::saveXML( tinyxml2::XMLNode *parent)
{
    tinyxml2::XMLElement *e = KRParticleSystem::saveXML(parent);
    return e;
}


AABB KRParticleSystemNewtonian::getBounds()
{
    return AABB::Create(-Vector3::One(), Vector3::One(), getModelMatrix());
}

void KRParticleSystemNewtonian::physicsUpdate(float deltaTime)
{
    KRParticleSystem::physicsUpdate(deltaTime);
    m_particlesAbsoluteTime += deltaTime;
}

bool KRParticleSystemNewtonian::hasPhysics()
{
    return true;
}

void KRParticleSystemNewtonian::render(VkCommandBuffer& commandBuffer, KRCamera *pCamera, std::vector<KRPointLight *> &point_lights, std::vector<KRDirectionalLight *> &directional_lights, std::vector<KRSpotLight *>&spot_lights, const KRViewport &viewport, KRNode::RenderPass renderPass) {
    
    if(m_lod_visible <= LOD_VISIBILITY_PRESTREAM) return;
    
    KRNode::render(commandBuffer, pCamera, point_lights, directional_lights, spot_lights, viewport, renderPass);
    
    if(renderPass == KRNode::RENDER_PASS_ADDITIVE_PARTICLES) {
        if(viewport.visible(getBounds())) {
            

            // Enable z-buffer test
            GLDEBUG(glEnable(GL_DEPTH_TEST));
            GLDEBUG(glDepthRangef(0.0, 1.0));
            
            KRTexture *pParticleTexture = m_pContext->getTextureManager()->getTexture("flare");
            m_pContext->getTextureManager()->selectTexture(0, pParticleTexture, 0.0f, KRTexture::TEXTURE_USAGE_PARTICLE);
            
            int particle_count = 10000;
            
            KRPipeline *pParticleShader = m_pContext->getPipelineManager()->getPipeline("dust_particle", pCamera, point_lights, directional_lights, spot_lights, 0, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, renderPass);
            
            // Vector3 rim_color; Vector4 fade_color;
            if(getContext().getPipelineManager()->selectPipeline(*pCamera, pParticleShader, viewport, getModelMatrix(), point_lights, directional_lights, spot_lights, 0, renderPass, Vector3::Zero(), 0.0f, Vector4::Zero())) {
                pParticleShader->setUniform(KRPipeline::KRENGINE_UNIFORM_FLARE_SIZE, 1.0f);

                KRDataBlock index_data;
                m_pContext->getMeshManager()->bindVBO(commandBuffer, m_pContext->getMeshManager()->getRandomParticles(), index_data, (1 << KRMesh::KRENGINE_ATTRIB_VERTEX) | (1 << KRMesh::KRENGINE_ATTRIB_TEXUVA), false, 1.0f
                
#if KRENGINE_DEBUG_GPU_LABELS
                  , "Newtonian Particles"
#endif
                );
                GLDEBUG(glDrawArrays(GL_TRIANGLES, 0, particle_count*3));
            }
        }
    }
}
