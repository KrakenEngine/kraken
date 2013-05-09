//
//  KRParticleSystemNewtonian.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-11-02.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KRParticleSystemNewtonian.h"
#include "KRAABB.h"
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


KRAABB KRParticleSystemNewtonian::getBounds()
{
    return KRAABB(-KRVector3::One(), KRVector3::One(), getModelMatrix());
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

void KRParticleSystemNewtonian::render(KRCamera *pCamera, std::vector<KRPointLight *> &point_lights, std::vector<KRDirectionalLight *> &directional_lights, std::vector<KRSpotLight *>&spot_lights, const KRViewport &viewport, KRNode::RenderPass renderPass) {
    
    
    KRNode::render(pCamera, point_lights, directional_lights, spot_lights, viewport, renderPass);
    
    if(renderPass == KRNode::RENDER_PASS_ADDITIVE_PARTICLES) {
        if(viewport.visible(getBounds())) {
            

            // Enable z-buffer test
            GLDEBUG(glEnable(GL_DEPTH_TEST));
            GLDEBUG(glDepthRangef(0.0, 1.0));
            
            KRTexture *pParticleTexture = m_pContext->getTextureManager()->getTexture("flare");
            m_pContext->getTextureManager()->selectTexture(0, pParticleTexture);
            
            int particle_count = 10000;
            
            KRShader *pParticleShader = m_pContext->getShaderManager()->getShader("dust_particle", pCamera, point_lights, directional_lights, spot_lights, 0, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, renderPass);
            
            if(getContext().getShaderManager()->selectShader(*pCamera, pParticleShader, viewport, getModelMatrix(), point_lights, directional_lights, spot_lights, 0, renderPass)) {
                pParticleShader->setUniform(KRShader::KRENGINE_UNIFORM_FLARE_SIZE, 1.0f);

                m_pContext->getModelManager()->bindVBO((void *)m_pContext->getModelManager()->getRandomParticles(), particle_count * 3 * sizeof(KRMeshManager::RandomParticleVertexData), NULL, 0, true, false, false, true, false, false, false, false);
                GLDEBUG(glDrawArrays(GL_TRIANGLES, 0, particle_count*3));
            }
        }
    }
}

//

//

//
//        KRMat4 particleModelMatrix = KRMat4();
////                particleModelMatrix.scale(particleBlockScale);
////
////                KRVector3 particleBlockOrigin = KRVector3(m_viewport.getCameraPosition().x - fmod(m_viewport.getCameraPosition().x + x * particleBlockScale, particleBlockScale), m_viewport.getCameraPosition().y - fmod(m_viewport.getCameraPosition().y + y * particleBlockScale, particleBlockScale),m_viewport.getCameraPosition().z - fmod(m_viewport.getCameraPosition().z + z * particleBlockScale, particleBlockScale));
////
////                particleModelMatrix.translate(particleBlockOrigin);
////                particleModelMatrix.translate(sin(m_particlesAbsoluteTime * 0.0523f) * 10.0f, sin(m_particlesAbsoluteTime * 0.0553f) * 10.0f, sin(m_particlesAbsoluteTime * 0.0521f) * 10.0f);
//
//                int particle_count = 10000;
//
//                if(getContext().getShaderManager()->selectShader(pParticleShader, m_viewport, particleModelMatrix, lightDirection, shadowmvpmatrix, shadowDepthTexture, m_cShadowBuffers, KRNode::RENDER_PASS_ADDITIVE_PARTICLES)) {
//                    GLDEBUG(glUniform1f(
//                                        pParticleShader->m_uniforms[KRShader::KRENGINE_UNIFORM_FLARE_SIZE],
//                                        1.0f
//                                        ));
//
//                    m_pContext->getModelManager()->bindVBO((void *)m_pContext->getModelManager()->getRandomParticles(), particle_count * 3 * sizeof(KRMeshManager::RandomParticleVertexData), NULL, 0, true, false, false, true, false, false);
//                    GLDEBUG(glDrawArrays(GL_TRIANGLES, 0, particle_count*3));
//                }
////            }
////        }
////    }