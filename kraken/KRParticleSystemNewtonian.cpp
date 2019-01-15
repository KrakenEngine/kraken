//
//  KRParticleSystemNewtonian.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-11-02.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
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

void KRParticleSystemNewtonian::render(KRCamera *pCamera, std::vector<KRPointLight *> &point_lights, std::vector<KRDirectionalLight *> &directional_lights, std::vector<KRSpotLight *>&spot_lights, const KRViewport &viewport, KRNode::RenderPass renderPass) {
    
    if(m_lod_visible <= LOD_VISIBILITY_PRESTREAM) return;
    
    KRNode::render(pCamera, point_lights, directional_lights, spot_lights, viewport, renderPass);
    
    if(renderPass == KRNode::RENDER_PASS_ADDITIVE_PARTICLES) {
        if(viewport.visible(getBounds())) {
            

            // Enable z-buffer test
            GLDEBUG(glEnable(GL_DEPTH_TEST));
            GLDEBUG(glDepthRangef(0.0, 1.0));
            
            KRTexture *pParticleTexture = m_pContext->getTextureManager()->getTexture("flare");
            m_pContext->getTextureManager()->selectTexture(0, pParticleTexture, 0.0f, KRTexture::TEXTURE_USAGE_PARTICLE);
            
            int particle_count = 10000;
            
            KRShader *pParticleShader = m_pContext->getShaderManager()->getShader("dust_particle", pCamera, point_lights, directional_lights, spot_lights, 0, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, renderPass);
            
            // Vector3 rim_color; Vector4 fade_color;
            if(getContext().getShaderManager()->selectShader(*pCamera, pParticleShader, viewport, getModelMatrix(), point_lights, directional_lights, spot_lights, 0, renderPass, Vector3::Zero(), 0.0f, Vector4::Zero())) {
                pParticleShader->setUniform(KRShader::KRENGINE_UNIFORM_FLARE_SIZE, 1.0f);

                KRDataBlock index_data;
                m_pContext->getMeshManager()->bindVBO(m_pContext->getMeshManager()->getRandomParticles(), index_data, (1 << KRMesh::KRENGINE_ATTRIB_VERTEX) | (1 << KRMesh::KRENGINE_ATTRIB_TEXUVA), false, 1.0f);
                GLDEBUG(glDrawArrays(GL_TRIANGLES, 0, particle_count*3));
            }
        }
    }
}
