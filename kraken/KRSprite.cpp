//
//  KRSprite.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-05.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//


#include "KREngine-common.h"
#include "KRSprite.h"

#include "KRNode.h"
#include "KRCamera.h"
#include "KRContext.h"

#include "KRShaderManager.h"
#include "KRShader.h"
#include "KRStockGeometry.h"
#include "KRDirectionalLight.h"
#include "KRSpotLight.h"
#include "KRPointLight.h"


KRSprite::KRSprite(KRScene &scene, std::string name) : KRNode(scene, name)
{
    m_spriteTexture = "";
    m_pSpriteTexture = NULL;
    m_spriteAlpha = 1.0f;
}

KRSprite::~KRSprite()
{
}

std::string KRSprite::getElementName() {
    return "sprite";
}

tinyxml2::XMLElement *KRSprite::saveXML( tinyxml2::XMLNode *parent)
{
    tinyxml2::XMLElement *e = KRNode::saveXML(parent);
    e->SetAttribute("sprite_texture", m_spriteTexture.c_str());
    e->SetAttribute("sprite_alpha", m_spriteAlpha);
    return e;
}

void KRSprite::loadXML(tinyxml2::XMLElement *e) {
    KRNode::loadXML(e);

    if(e->QueryFloatAttribute("sprite_alpha", &m_spriteAlpha) != tinyxml2::XML_SUCCESS) {
        m_spriteAlpha = 1.0f;
    }
    
    const char *szSpriteTexture = e->Attribute("sprite_texture");
    if(szSpriteTexture) {
        m_spriteTexture = szSpriteTexture;
    } else {
        m_spriteTexture = "";
    }
    m_pSpriteTexture = NULL;
}

void KRSprite::setSpriteTexture(std::string sprite_texture) {
    m_spriteTexture = sprite_texture;
    m_pSpriteTexture = NULL;
}

void KRSprite::setSpriteAlpha(float alpha)
{
    m_spriteAlpha = alpha;
}

float KRSprite::getSpriteAlpha() const
{
    return m_spriteAlpha;
}

KRAABB KRSprite::getBounds() {
    return KRAABB(-KRVector3::One() * 0.5f, KRVector3::One() * 0.5f, getModelMatrix());
}


void KRSprite::render(KRCamera *pCamera, std::vector<KRPointLight *> &point_lights, std::vector<KRDirectionalLight *> &directional_lights, std::vector<KRSpotLight *>&spot_lights, const KRViewport &viewport, KRNode::RenderPass renderPass) {
    
    if(m_lod_visible >= LOD_VISIBILITY_PRESTREAM && renderPass == KRNode::RENDER_PASS_PRESTREAM) {
        // Pre-stream sprites, even if the alpha is zero
        if(m_spriteTexture.size() && m_pSpriteTexture == NULL) {
            if(!m_pSpriteTexture && m_spriteTexture.size()) {
                m_pSpriteTexture = getContext().getTextureManager()->getTexture(m_spriteTexture);
            }
        }
        
        if(m_pSpriteTexture) {
            m_pSpriteTexture->resetPoolExpiry(0.0f, KRTexture::TEXTURE_USAGE_SPRITE);
        }
    }
    
    if(m_lod_visible <= LOD_VISIBILITY_PRESTREAM) return;
    
    KRNode::render(pCamera, point_lights, directional_lights, spot_lights, viewport, renderPass);
    
    
    if(renderPass == KRNode::RENDER_PASS_ADDITIVE_PARTICLES) {
        if(m_spriteTexture.size() && m_spriteAlpha > 0.0f) {
            

            if(!m_pSpriteTexture && m_spriteTexture.size()) {
                m_pSpriteTexture = getContext().getTextureManager()->getTexture(m_spriteTexture);
            }
            
            if(m_pSpriteTexture) {
                /*
                // Enable additive blending
                GLDEBUG(glEnable(GL_BLEND));
                GLDEBUG(glBlendFunc(GL_ONE, GL_ONE));
                
                // Disable z-buffer write
                GLDEBUG(glDepthMask(GL_FALSE));
                 */
                
                // TODO - Sprites are currently additive only.  Need to expose this and allow for multiple blending modes
                
                // Enable z-buffer test
                GLDEBUG(glEnable(GL_DEPTH_TEST));
                GLDEBUG(glDepthFunc(GL_LEQUAL));
                GLDEBUG(glDepthRangef(0.0, 1.0));
                
                // Render light sprite on transparency pass
                KRShader *pShader = getContext().getShaderManager()->getShader("sprite", pCamera, point_lights, directional_lights, spot_lights, 0, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, renderPass);
                if(getContext().getShaderManager()->selectShader(*pCamera, pShader, viewport, getModelMatrix(), point_lights, directional_lights, spot_lights, 0, renderPass, KRVector3::Zero(), 0.0f, KRVector4::Zero())) {
                    pShader->setUniform(KRShader::KRENGINE_UNIFORM_MATERIAL_ALPHA, m_spriteAlpha);
                    m_pContext->getTextureManager()->selectTexture(0, m_pSpriteTexture, 0.0f, KRTexture::TEXTURE_USAGE_SPRITE);
                    m_pContext->getMeshManager()->bindVBO(&m_pContext->getMeshManager()->KRENGINE_VBO_DATA_2D_SQUARE_VERTICES, 1.0f);
                    GLDEBUG(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
                }
            }
        }
        
    }
}
