//
//  KRParticleSystemBrownian.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-11-02.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KRParticleSystemBrownian.h"
#include "KRAABB.h"
#include "KRTexture.h"
#include "KRContext.h"

KRParticleSystemBrownian::KRParticleSystemBrownian(KRScene &scene, std::string name) : KRParticleSystem(scene, name)
{
    m_particlesAbsoluteTime = 0.0f;
}

KRParticleSystemBrownian::~KRParticleSystemBrownian()
{
    
}

std::string KRParticleSystemBrownian::getElementName()
{
    return "brownian_particles";
}

void KRParticleSystemBrownian::loadXML(tinyxml2::XMLElement *e)
{
    KRParticleSystem::loadXML(e);
}

tinyxml2::XMLElement *KRParticleSystemBrownian::saveXML( tinyxml2::XMLNode *parent)
{
    tinyxml2::XMLElement *e = KRParticleSystem::saveXML(parent);
    return e;
}


KRAABB KRParticleSystemBrownian::getBounds()
{
    return KRAABB(-KRVector3::One(), KRVector3::One(), getModelMatrix());
}

void KRParticleSystemBrownian::physicsUpdate(float deltaTime)
{
    KRParticleSystem::physicsUpdate(deltaTime);
    m_particlesAbsoluteTime += deltaTime;
}


#if TARGET_OS_IPHONE

void KRParticleSystemBrownian::render(KRCamera *pCamera, KRContext *pContext, const KRViewport &viewport, const KRViewport *pShadowViewports, KRVector3 &lightDirection, GLuint *shadowDepthTextures, int cShadowBuffers, KRNode::RenderPass renderPass) {
    
    
    KRNode::render(pCamera, pContext, viewport, pShadowViewports, lightDirection, shadowDepthTextures, cShadowBuffers, renderPass);
    
    if(renderPass == KRNode::RENDER_PASS_ADDITIVE_PARTICLES) {
        float lod_coverage = getBounds().coverage(viewport.getViewProjectionMatrix(), viewport.getSize()); // This also checks the view frustrum culling
        if(lod_coverage > 0.0f) {
            

            // Enable z-buffer test
            GLDEBUG(glEnable(GL_DEPTH_TEST));
            GLDEBUG(glDepthRangef(0.0, 1.0));
            
            KRTexture *pParticleTexture = m_pContext->getTextureManager()->getTexture("flare");
            m_pContext->getTextureManager()->selectTexture(0, pParticleTexture, 2048);
            
            KRShader *pParticleShader = m_pContext->getShaderManager()->getShader("particle", pCamera, false, false, false, 0, false, false, false, false, false, false, false, false, false, false, false, false, false, KRNode::RENDER_PASS_ADDITIVE_PARTICLES);
            
            int particle_count = 10000;
            
            if(pParticleShader->bind(viewport, pShadowViewports, getModelMatrix(), lightDirection, shadowDepthTextures, cShadowBuffers, KRNode::RENDER_PASS_ADDITIVE_PARTICLES)) {
                GLDEBUG(glUniform1f(
                    pParticleShader->m_uniforms[KRShader::KRENGINE_UNIFORM_FLARE_SIZE],
                    1.0f
                    ));

                m_pContext->getModelManager()->bindVBO((void *)m_pContext->getModelManager()->getRandomParticles(), particle_count * 3 * sizeof(KRModelManager::RandomParticleVertexData), true, false, false, true, false);
                GLDEBUG(glDrawArrays(GL_TRIANGLES, 0, particle_count*3));
            }
        }
    }
}

#endif

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
//                if(pParticleShader->bind(m_viewport, particleModelMatrix, lightDirection, shadowmvpmatrix, shadowDepthTexture, m_cShadowBuffers, KRNode::RENDER_PASS_ADDITIVE_PARTICLES)) {
//                    GLDEBUG(glUniform1f(
//                                        pParticleShader->m_uniforms[KRShader::KRENGINE_UNIFORM_FLARE_SIZE],
//                                        1.0f
//                                        ));
//
//                    m_pContext->getModelManager()->bindVBO((void *)m_pContext->getModelManager()->getRandomParticles(), particle_count * 3 * sizeof(KRModelManager::RandomParticleVertexData), true, false, false, true, false);
//                    GLDEBUG(glDrawArrays(GL_TRIANGLES, 0, particle_count*3));
//                }
////            }
////        }
////    }