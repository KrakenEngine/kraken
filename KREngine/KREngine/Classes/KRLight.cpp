//
//  KRLight.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-05.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include <iostream>



#import "KRLight.h"

#import "KRNode.h"
#import "KRMat4.h"
#import "KRVector3.h"
#import "KRCamera.h"
#import "KRContext.h"

#import "KRShaderManager.h"
#import "KRShader.h"
#import "KRStockGeometry.h"
#import "assert.h"

KRLight::KRLight(KRScene &scene, std::string name) : KRNode(scene, name)
{
    m_intensity = 1.0f;
    m_flareTexture = "";
    m_pFlareTexture = NULL;
    m_flareSize = 0.0;
    m_casts_shadow = true;
    m_light_shafts = true;
    
    // Initialize shadow buffers
    m_cShadowBuffers = 0;
    for(int iBuffer=0; iBuffer < KRENGINE_MAX_SHADOW_BUFFERS; iBuffer++) {
        shadowFramebuffer[iBuffer] = 0;
        shadowDepthTexture[iBuffer] = 0;
        shadowValid[iBuffer] = false;
    }
}

KRLight::~KRLight()
{
    allocateShadowBuffers(0);
}

tinyxml2::XMLElement *KRLight::saveXML( tinyxml2::XMLNode *parent)
{
    tinyxml2::XMLElement *e = KRNode::saveXML(parent);
    e->SetAttribute("intensity", m_intensity);
    e->SetAttribute("color_r", m_color.x);
    e->SetAttribute("color_g", m_color.y);
    e->SetAttribute("color_b", m_color.z);
    e->SetAttribute("decay_start", m_decayStart);
    e->SetAttribute("flare_size", m_flareSize);
    e->SetAttribute("flare_texture", m_flareTexture.c_str());
    e->SetAttribute("casts_shadow", m_casts_shadow ? "true" : "false");
    e->SetAttribute("light_shafts", m_light_shafts ? "true" : "false");
    return e;
}

void KRLight::loadXML(tinyxml2::XMLElement *e) {
    KRNode::loadXML(e);
    float x,y,z;
    if(e->QueryFloatAttribute("color_r", &x) != tinyxml2::XML_SUCCESS) {
        x = 1.0;
    }
    if(e->QueryFloatAttribute("color_g", &y) != tinyxml2::XML_SUCCESS) {
        y = 1.0;
    }
    if(e->QueryFloatAttribute("color_b", &z) != tinyxml2::XML_SUCCESS) {
        z = 1.0;
    }
    m_color = KRVector3(x,y,z);
    
    if(e->QueryFloatAttribute("intensity", &m_intensity) != tinyxml2::XML_SUCCESS) {
        m_intensity = 100.0;
    }
    
    if(e->QueryFloatAttribute("decay_start", &m_decayStart) != tinyxml2::XML_SUCCESS) {
        m_decayStart = 0.0;
    }
    
    if(e->QueryFloatAttribute("flare_size", &m_flareSize) != tinyxml2::XML_SUCCESS) {
        m_flareSize = 0.0;
    }
    
    if(e->QueryBoolAttribute("casts_shadow", &m_casts_shadow) != tinyxml2::XML_SUCCESS) {
        m_casts_shadow = true;
    }
    
    if(e->QueryBoolAttribute("light_shafts", &m_light_shafts) != tinyxml2::XML_SUCCESS) {
        m_light_shafts = true;
    }
    
    const char *szFlareTexture = e->Attribute("flare_texture");
    if(szFlareTexture) {
        m_flareTexture = szFlareTexture;
    } else {
        m_flareTexture = "";
    }
    m_pFlareTexture = NULL;
}

void KRLight::setFlareTexture(std::string flare_texture) {
    m_flareTexture = flare_texture;
    m_pFlareTexture = NULL;
}

void KRLight::setFlareSize(float flare_size) {
    m_flareSize = flare_size;
}

void KRLight::setIntensity(float intensity) {
    m_intensity = intensity;
}
float KRLight::getIntensity() {
    return m_intensity;
}

const KRVector3 &KRLight::getColor() {
    return m_color;
}

void KRLight::setColor(const KRVector3 &color) {
    m_color = color;
}

void KRLight::setDecayStart(float decayStart) {
    m_decayStart = decayStart;
}

float KRLight::getDecayStart() {
    return m_decayStart;
}

#if TARGET_OS_IPHONE

void KRLight::render(KRCamera *pCamera, std::vector<KRLight *> &lights, const KRViewport &viewport, KRNode::RenderPass renderPass) {

    KRNode::render(pCamera, lights, viewport, renderPass);
    
    if(renderPass == KRNode::RENDER_PASS_GENERATE_SHADOWMAPS && (pCamera->volumetric_environment_enable || (pCamera->m_cShadowBuffers > 0 && m_casts_shadow))) {
        allocateShadowBuffers(configureShadowBufferViewports(viewport));
        renderShadowBuffers(pCamera);
    }
    
    if(renderPass == KRNode::RENDER_PASS_VOLUMETRIC_EFFECTS_ADDITIVE && pCamera->volumetric_environment_enable && m_light_shafts) {
        std::string shader_name = pCamera->volumetric_environment_downsample != 0 ? "volumetric_fog_downsampled" : "volumetric_fog";
        
        std::vector<KRLight *> this_light;
        this_light.push_back(this);
        
        KRShader *pFogShader = m_pContext->getShaderManager()->getShader(shader_name, pCamera, this_light, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, KRNode::RENDER_PASS_ADDITIVE_PARTICLES);
        
        
        if(getContext().getShaderManager()->selectShader(pFogShader, viewport, KRMat4(), this_light, KRNode::RENDER_PASS_VOLUMETRIC_EFFECTS_ADDITIVE)) {
            int slice_count = (int)(pCamera->volumetric_environment_quality * 495.0) + 5;
            
            float slice_near = -pCamera->getPerspectiveNearZ();
            float slice_far = -pCamera->volumetric_environment_max_distance;
            float slice_spacing = (slice_far - slice_near) / slice_count;
            
            KRVector2(slice_near, slice_spacing).setUniform(pFogShader->m_uniforms[KRShader::KRENGINE_UNIFORM_SLICE_DEPTH_SCALE]);
            (KRVector3::One() * pCamera->volumetric_environment_intensity * -slice_spacing / 1000.0f).setUniform(pFogShader->m_uniforms[KRShader::KRENGINE_UNIFORM_LIGHT_COLOR]);
            
            m_pContext->getModelManager()->bindVBO((void *)m_pContext->getModelManager()->getVolumetricLightingVertexes(), KRModelManager::MAX_VOLUMETRIC_PLANES * 6 * sizeof(KRModelManager::VolumetricLightingVertexData), true, false, false, false, false);
            GLDEBUG(glDrawArrays(GL_TRIANGLES, 0, slice_count*6));
        }

    }
    
    if(renderPass == KRNode::RENDER_PASS_ADDITIVE_PARTICLES) {
        if(m_flareTexture.size() && m_flareSize > 0.0f) {
            if(!m_pFlareTexture && m_flareTexture.size()) {
                m_pFlareTexture = getContext().getTextureManager()->getTexture(m_flareTexture.c_str());
            }
            
            if(m_pFlareTexture) {
                // Disable z-buffer test
                GLDEBUG(glDisable(GL_DEPTH_TEST));
                GLDEBUG(glDepthRangef(0.0, 1.0));
                
                // Render light flare on transparency pass
                KRShader *pShader = getContext().getShaderManager()->getShader("flare", pCamera, lights, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, renderPass);
                if(getContext().getShaderManager()->selectShader(pShader, viewport, getModelMatrix(), lights, renderPass)) {
                    GLDEBUG(glUniform1f(
                                        pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_FLARE_SIZE],
                                        m_flareSize
                                        ));
                    m_pContext->getTextureManager()->selectTexture(0, m_pFlareTexture);
                    m_pContext->getModelManager()->bindVBO((void *)KRENGINE_VBO_2D_SQUARE, KRENGINE_VBO_2D_SQUARE_SIZE, true, false, false, true, false);
                    GLDEBUG(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
                }
            }
        }
        
    }
}


void KRLight::allocateShadowBuffers(int cBuffers) {
    // First deallocate buffers no longer needed
    for(int iShadow = cBuffers; iShadow < KRENGINE_MAX_SHADOW_BUFFERS; iShadow++) {
        if (shadowDepthTexture[iShadow]) {
            GLDEBUG(glDeleteTextures(1, shadowDepthTexture + iShadow));
            shadowDepthTexture[iShadow] = 0;
        }
        
        if (shadowFramebuffer[iShadow]) {
            GLDEBUG(glDeleteFramebuffers(1, shadowFramebuffer + iShadow));
            shadowFramebuffer[iShadow] = 0;
        }
    }
    
    // Allocate newly required buffers
    for(int iShadow = 0; iShadow < cBuffers; iShadow++) {
        KRVector2 viewportSize = m_shadowViewports[iShadow].getSize();
        
        if(!shadowDepthTexture[iShadow]) {
            shadowValid[iShadow] = false;
            
            GLDEBUG(glGenFramebuffers(1, shadowFramebuffer + iShadow));
            GLDEBUG(glGenTextures(1, shadowDepthTexture + iShadow));
            // ===== Create offscreen shadow framebuffer object =====
            
            GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer[iShadow]));
            
            // ----- Create Depth Texture for shadowFramebuffer -----
            GLDEBUG( glBindTexture(GL_TEXTURE_2D, shadowDepthTexture[iShadow]));
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
#if GL_EXT_shadow_samplers
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_EXT, GL_COMPARE_REF_TO_TEXTURE_EXT)); // TODO - Detect GL_EXT_shadow_samplers and only activate if available
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_EXT, GL_LEQUAL)); // TODO - Detect GL_EXT_shadow_samplers and only activate if available
#endif
            GLDEBUG(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, viewportSize.x, viewportSize.y, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL));
            
            GLDEBUG(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepthTexture[iShadow], 0));
        }
    }
    
    m_cShadowBuffers = cBuffers;
}


void KRLight::deleteBuffers()
{
    // Called when this light wasn't used in the last frame, so we can free the resources for use by other lights
    allocateShadowBuffers(0);
}

void KRLight::invalidateShadowBuffers()
{
    memset(shadowValid, sizeof(bool) * KRENGINE_MAX_SHADOW_BUFFERS, 0);
}

int KRLight::configureShadowBufferViewports(const KRViewport &viewport)
{
    return 0;
}

void KRLight::renderShadowBuffers(KRCamera *pCamera)
{
    for(int iShadow=0; iShadow < m_cShadowBuffers; iShadow++) {
        glViewport(0, 0, m_shadowViewports[iShadow].getSize().x, m_shadowViewports[iShadow].getSize().y);
        
        GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer[iShadow]));
        
        GLDEBUG(glClearDepthf(0.0f));
        GLDEBUG(glClear(GL_DEPTH_BUFFER_BIT));
        
        glViewport(0, 0, m_shadowViewports[iShadow].getSize().x, m_shadowViewports[iShadow].getSize().y);
        
        GLDEBUG(glClearDepthf(1.0f));
        GLDEBUG(glClear(GL_DEPTH_BUFFER_BIT));
        
        glViewport(1, 1, m_shadowViewports[iShadow].getSize().x - 2, m_shadowViewports[iShadow].getSize().y - 2);
        
        GLDEBUG(glDisable(GL_DITHER));
        
        GLDEBUG(glCullFace(GL_BACK)); // Enable frontface culling, which eliminates some self-cast shadow artifacts
        GLDEBUG(glEnable(GL_CULL_FACE));
        
        // Enable z-buffer test
        GLDEBUG(glEnable(GL_DEPTH_TEST));
        GLDEBUG(glDepthFunc(GL_LESS));
        GLDEBUG(glDepthRangef(0.0, 1.0));
        
        // Disable alpha blending as we are using alpha channel for packed depth info
        GLDEBUG(glDisable(GL_BLEND));
        
        // Use shader program
        KRShader *shadowShader = m_pContext->getShaderManager()->getShader("ShadowShader", pCamera, std::vector<KRLight *>(), false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, KRNode::RENDER_PASS_FORWARD_TRANSPARENT);
        
        getContext().getShaderManager()->selectShader(shadowShader, m_shadowViewports[iShadow], KRMat4(), std::vector<KRLight *>(), KRNode::RENDER_PASS_SHADOWMAP);
        
        
        getScene().render(pCamera, m_shadowViewports[iShadow].getVisibleBounds(), m_shadowViewports[iShadow], KRNode::RENDER_PASS_SHADOWMAP);

    }
}

#endif

int KRLight::getShadowBufferCount()
{
    return m_cShadowBuffers;
}

GLuint *KRLight::getShadowTextures()
{
    return shadowDepthTexture;
}

KRViewport *KRLight::getShadowViewports()
{
    return m_shadowViewports;
}
