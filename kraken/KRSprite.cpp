//
//  KRSprite.cpp
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
#include "KRSprite.h"

#include "KRNode.h"
#include "KRCamera.h"
#include "KRContext.h"

#include "KRPipelineManager.h"
#include "KRPipeline.h"
#include "KRDirectionalLight.h"
#include "KRSpotLight.h"
#include "KRPointLight.h"

/* static */
void KRSprite::InitNodeInfo(KrNodeInfo* nodeInfo)
{
  KRNode::InitNodeInfo(nodeInfo);
  nodeInfo->sprite.alpha = 1.0f;
  nodeInfo->sprite.texture = -1;
}

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

AABB KRSprite::getBounds() {
    return AABB::Create(-Vector3::One() * 0.5f, Vector3::One() * 0.5f, getModelMatrix());
}


void KRSprite::render(RenderInfo& ri) {
    
    if(m_lod_visible >= LOD_VISIBILITY_PRESTREAM && ri.renderPass == KRNode::RENDER_PASS_PRESTREAM) {
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
    
    KRNode::render(ri);
    
    
    if(ri.renderPass == KRNode::RENDER_PASS_ADDITIVE_PARTICLES) {
        if(m_spriteTexture.size() && m_spriteAlpha > 0.0f) {
            

            if(!m_pSpriteTexture && m_spriteTexture.size()) {
                m_pSpriteTexture = getContext().getTextureManager()->getTexture(m_spriteTexture);
            }
            
            if(m_pSpriteTexture) {               
                // TODO - Sprites are currently additive only.  Need to expose this and allow for multiple blending modes
                               
                // Render light sprite on transparency pass
                PipelineInfo info{};
                std::string shader_name("sprite");
                info.shader_name = &shader_name;
                info.pCamera = ri.camera;
                info.point_lights = &ri.point_lights;
                info.directional_lights = &ri.directional_lights;
                info.spot_lights = &ri.spot_lights;
                info.renderPass = ri.renderPass;
                info.rasterMode = PipelineInfo::RasterMode::kAdditive;
                KRPipeline *pShader = getContext().getPipelineManager()->getPipeline(*ri.surface, info);
                if(getContext().getPipelineManager()->selectPipeline(*ri.surface, *ri.camera, pShader, ri.viewport, getModelMatrix(), &ri.point_lights, &ri.directional_lights, &ri.spot_lights, 0, ri.renderPass, Vector3::Zero(), 0.0f, Vector4::Zero())) {
                    pShader->setUniform(KRPipeline::KRENGINE_UNIFORM_MATERIAL_ALPHA, m_spriteAlpha);
                    m_pContext->getTextureManager()->selectTexture(0, m_pSpriteTexture, 0.0f, KRTexture::TEXTURE_USAGE_SPRITE);
                    m_pContext->getMeshManager()->bindVBO(ri.commandBuffer, &m_pContext->getMeshManager()->KRENGINE_VBO_DATA_2D_SQUARE_VERTICES, 1.0f);
                    GLDEBUG(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
                }
            }
        }
        
    }
}
