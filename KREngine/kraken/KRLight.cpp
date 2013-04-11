//
//  KRLight.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-05.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//


#include "KREngine-common.h"
#include "KRLight.h"

#include "KRNode.h"
#include "KRMat4.h"
#include "KRVector3.h"
#include "KRCamera.h"
#include "KRContext.h"

#include "KRShaderManager.h"
#include "KRShader.h"
#include "KRStockGeometry.h"


KRLight::KRLight(KRScene &scene, std::string name) : KRNode(scene, name)
{
    m_intensity = 1.0f;
    m_dust_particle_intensity = 1.0f;
    m_color = KRVector3::One();
    m_flareTexture = "";
    m_pFlareTexture = NULL;
    m_flareSize = 0.0;
    m_flareOcclusionSize = 0.05;
    m_casts_shadow = true;
    m_light_shafts = true;
    m_dust_particle_density = 0.1f;
    m_dust_particle_size = 1.0f;
    m_occlusionQuery = 0;
    
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
    if(m_occlusionQuery) {
        GLDEBUG(glDeleteQueriesEXT(1, &m_occlusionQuery));
        m_occlusionQuery = 0;
    }
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
    e->SetAttribute("flare_occlusion_size", m_flareOcclusionSize);
    e->SetAttribute("flare_texture", m_flareTexture.c_str());
    e->SetAttribute("casts_shadow", m_casts_shadow ? "true" : "false");
    e->SetAttribute("light_shafts", m_light_shafts ? "true" : "false");
    e->SetAttribute("dust_particle_density", m_dust_particle_density);
    e->SetAttribute("dust_particle_size", m_dust_particle_size);
    e->SetAttribute("dust_particle_intensity", m_dust_particle_intensity);
    return e;
}

void KRLight::loadXML(tinyxml2::XMLElement *e) {
    KRNode::loadXML(e);
    float x=1.0f,y=1.0f,z=1.0f;
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
    
    if(e->QueryFloatAttribute("flare_occlusion_size", &m_flareOcclusionSize) != tinyxml2::XML_SUCCESS) {
        m_flareOcclusionSize = 0.05;
    }
    
    if(e->QueryBoolAttribute("casts_shadow", &m_casts_shadow) != tinyxml2::XML_SUCCESS) {
        m_casts_shadow = true;
    }
    
    if(e->QueryBoolAttribute("light_shafts", &m_light_shafts) != tinyxml2::XML_SUCCESS) {
        m_light_shafts = true;
    }
    
    m_dust_particle_density = 0.1f;
    if(e->QueryFloatAttribute("dust_particle_density", &m_dust_particle_density) != tinyxml2::XML_SUCCESS) {
        m_dust_particle_density = 0.1f;
    }
    
    m_dust_particle_size = 1.0f;
    if(e->QueryFloatAttribute("dust_particle_size", &m_dust_particle_size) != tinyxml2::XML_SUCCESS) {
        m_dust_particle_size = 1.0f;
    }
    
    m_dust_particle_intensity = 1.0f;
    if(e->QueryFloatAttribute("dust_particle_intensity", &m_dust_particle_intensity) != tinyxml2::XML_SUCCESS) {
        m_dust_particle_intensity = 1.0f;
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

void KRLight::setFlareOcclusionSize(float occlusion_size) {
    m_flareOcclusionSize = occlusion_size;
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

void KRLight::render(KRCamera *pCamera, std::vector<KRLight *> &lights, const KRViewport &viewport, KRNode::RenderPass renderPass) {

    KRNode::render(pCamera, lights, viewport, renderPass);
    
    if(renderPass == KRNode::RENDER_PASS_GENERATE_SHADOWMAPS && (pCamera->settings.volumetric_environment_enable || pCamera->settings.dust_particle_enable || (pCamera->settings.m_cShadowBuffers > 0 && m_casts_shadow))) {
        allocateShadowBuffers(configureShadowBufferViewports(viewport));
        renderShadowBuffers(pCamera);
    }
    
    if(renderPass == KRNode::RENDER_PASS_ADDITIVE_PARTICLES && pCamera->settings.dust_particle_enable) {
        // Render brownian particles for dust floating in air
        if(m_cShadowBuffers >= 1 && shadowValid[0] && m_dust_particle_density > 0.0f && m_dust_particle_size > 0.0f && m_dust_particle_intensity > 0.0f) {
            float lod_coverage = getBounds().coverage(viewport.getViewProjectionMatrix(), viewport.getSize()); // This also checks the view frustrum culling
            if(lod_coverage > 0.0f || true) {
                
                float particle_range = 600.0f;
                
                int particle_count = m_dust_particle_density * pow(particle_range, 3);
                if(particle_count > KRMeshManager::KRENGINE_MAX_RANDOM_PARTICLES) particle_count = KRMeshManager::KRENGINE_MAX_RANDOM_PARTICLES;
                
                // Enable z-buffer test
                GLDEBUG(glEnable(GL_DEPTH_TEST));
                GLDEBUG(glDepthRangef(0.0, 1.0));

                KRMat4 particleModelMatrix;
                particleModelMatrix.scale(particle_range);  // Scale the box symetrically to ensure that we don't have an uneven distribution of particles for different angles of the view frustrum
                particleModelMatrix.translate(viewport.getCameraPosition());
                
                std::vector<KRLight *> this_light;
                this_light.push_back(this);
                
                KRShader *pParticleShader = m_pContext->getShaderManager()->getShader("dust_particle", pCamera, this_light, 0, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, renderPass);
                
                if(getContext().getShaderManager()->selectShader(*pCamera, pParticleShader, viewport, particleModelMatrix, this_light, 0, renderPass)) {
                    
                    (m_color * pCamera->settings.dust_particle_intensity * m_dust_particle_intensity * m_intensity).setUniform(pParticleShader->m_uniforms[KRShader::KRENGINE_UNIFORM_LIGHT_COLOR]);
                    
                    KRMat4::DotWDiv(KRMat4::Invert(particleModelMatrix), KRVector3::Zero()).setUniform(pParticleShader->m_uniforms[KRShader::KRENGINE_UNIFORM_PARTICLE_ORIGIN]);
                    
                    GLDEBUG(glUniform1f(pParticleShader->m_uniforms[KRShader::KRENGINE_UNIFORM_FLARE_SIZE], m_dust_particle_size));
                    
                    m_pContext->getModelManager()->bindVBO((void *)m_pContext->getModelManager()->getRandomParticles(), KRMeshManager::KRENGINE_MAX_RANDOM_PARTICLES * 3 * sizeof(KRMeshManager::RandomParticleVertexData), NULL, 0, true, false, false, true, false, false, false, true);
                    GLDEBUG(glDrawArrays(GL_TRIANGLES, 0, particle_count*3));
                }
            }
        }
    }

    
    if(renderPass == KRNode::RENDER_PASS_VOLUMETRIC_EFFECTS_ADDITIVE && pCamera->settings.volumetric_environment_enable && m_light_shafts) {
        std::string shader_name = pCamera->settings.volumetric_environment_downsample != 0 ? "volumetric_fog_downsampled" : "volumetric_fog";
        
        std::vector<KRLight *> this_light;
        this_light.push_back(this);
        
        KRShader *pFogShader = m_pContext->getShaderManager()->getShader(shader_name, pCamera, this_light, 0, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, KRNode::RENDER_PASS_ADDITIVE_PARTICLES);
        
        
        if(getContext().getShaderManager()->selectShader(*pCamera, pFogShader, viewport, KRMat4(), this_light, 0, KRNode::RENDER_PASS_VOLUMETRIC_EFFECTS_ADDITIVE)) {
            int slice_count = (int)(pCamera->settings.volumetric_environment_quality * 495.0) + 5;
            
            float slice_near = -pCamera->settings.getPerspectiveNearZ();
            float slice_far = -pCamera->settings.volumetric_environment_max_distance;
            float slice_spacing = (slice_far - slice_near) / slice_count;
            
            KRVector2(slice_near, slice_spacing).setUniform(pFogShader->m_uniforms[KRShader::KRENGINE_UNIFORM_SLICE_DEPTH_SCALE]);
            (m_color * pCamera->settings.volumetric_environment_intensity * m_intensity * -slice_spacing / 1000.0f).setUniform(pFogShader->m_uniforms[KRShader::KRENGINE_UNIFORM_LIGHT_COLOR]);
            
            m_pContext->getModelManager()->bindVBO((void *)m_pContext->getModelManager()->getVolumetricLightingVertexes(), KRMeshManager::KRENGINE_MAX_VOLUMETRIC_PLANES * 6 * sizeof(KRMeshManager::VolumetricLightingVertexData), NULL, 0, true, false, false, false, false, false, false, true);
            GLDEBUG(glDrawArrays(GL_TRIANGLES, 0, slice_count*6));
        }

    }
    
    if(renderPass == KRNode::RENDER_PASS_PARTICLE_OCCLUSION) {
        if(m_flareTexture.size() && m_flareSize > 0.0f) {
            
            
            KRMat4 occlusion_test_sphere_matrix = KRMat4();
            occlusion_test_sphere_matrix.scale(m_localScale * m_flareOcclusionSize);
            occlusion_test_sphere_matrix.translate(m_localTranslation);
            if(m_parentNode) {
                occlusion_test_sphere_matrix *= m_parentNode->getModelMatrix();
            }

            if(getContext().getShaderManager()->selectShader("occlusion_test", *pCamera, lights, 0, viewport, occlusion_test_sphere_matrix, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, renderPass)) {

                GLDEBUG(glGenQueriesEXT(1, &m_occlusionQuery));
#if TARGET_OS_IPHONE
                GLDEBUG(glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, m_occlusionQuery));
#else
                GLDEBUG(glBeginQuery(GL_SAMPLES_PASSED, m_occlusionQuery));
#endif

                std::vector<KRMesh *> sphereModels = getContext().getModelManager()->getModel("__sphere");
                if(sphereModels.size()) {
                    for(int i=0; i < sphereModels[0]->getSubmeshCount(); i++) {
                        sphereModels[0]->renderSubmesh(i, renderPass, getName(), "occlusion_test");
                    }
                }

#if TARGET_OS_IPHONE
                GLDEBUG(glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT));
#else
                GLDEBUG(glEndQuery(GL_SAMPLES_PASSED));
#endif

            }
        }
    }
    
    if(renderPass == KRNode::RENDER_PASS_ADDITIVE_PARTICLES) {
        if(m_flareTexture.size() && m_flareSize > 0.0f) {
            
            if(m_occlusionQuery) {
                GLuint hasBeenTested = 0;
                while(!hasBeenTested) {
                    GLDEBUG(glGetQueryObjectuivEXT(m_occlusionQuery, GL_QUERY_RESULT_AVAILABLE_EXT, &hasBeenTested)); // FINDME, HACK!!  This needs to be replaced with asynchonous logic
                }
                GLuint params = 0;
                GLDEBUG(glGetQueryObjectuivEXT(m_occlusionQuery, GL_QUERY_RESULT_EXT, &params));
                GLDEBUG(glDeleteQueriesEXT(1, &m_occlusionQuery));
                
                if(params) {
                    
                    if(!m_pFlareTexture && m_flareTexture.size()) {
                        m_pFlareTexture = getContext().getTextureManager()->getTexture(m_flareTexture.c_str());
                    }
                    
                    if(m_pFlareTexture) {
                        // Disable z-buffer test
                        GLDEBUG(glDisable(GL_DEPTH_TEST));
                        GLDEBUG(glDepthRangef(0.0, 1.0));
                        
                        // Render light flare on transparency pass
                        KRShader *pShader = getContext().getShaderManager()->getShader("flare", pCamera, lights, 0, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, renderPass);
                        if(getContext().getShaderManager()->selectShader(*pCamera, pShader, viewport, getModelMatrix(), lights, 0, renderPass)) {
                            GLDEBUG(glUniform1f(
                                                pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_FLARE_SIZE],
                                                m_flareSize
                                                ));
                            m_pContext->getTextureManager()->selectTexture(0, m_pFlareTexture);
                            m_pContext->getModelManager()->bindVBO((void *)KRENGINE_VBO_2D_SQUARE, KRENGINE_VBO_2D_SQUARE_SIZE, NULL, 0, true, false, false, true, false, false, false, true);
                            GLDEBUG(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
                        }
                    }
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
    for(int iShadow=0; iShadow < m_cShadowBuffers; iShadow++) {
        shadowValid[iShadow] = false;
    }
}

int KRLight::configureShadowBufferViewports(const KRViewport &viewport)
{
    return 0;
}

void KRLight::renderShadowBuffers(KRCamera *pCamera)
{
    for(int iShadow=0; iShadow < m_cShadowBuffers; iShadow++) {
        if(!shadowValid[iShadow]) {
            shadowValid[iShadow] = true;
            
            GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer[iShadow]));
            GLDEBUG(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepthTexture[iShadow], 0));
            
            GLDEBUG(glViewport(0, 0, m_shadowViewports[iShadow].getSize().x, m_shadowViewports[iShadow].getSize().y));
            
            GLDEBUG(glClearDepthf(0.0f));
            GLDEBUG(glClear(GL_DEPTH_BUFFER_BIT));
            
            GLDEBUG(glViewport(1, 1, m_shadowViewports[iShadow].getSize().x - 2, m_shadowViewports[iShadow].getSize().y - 2));

            GLDEBUG(glClearDepthf(1.0f));
            
            GLDEBUG(glClear(GL_DEPTH_BUFFER_BIT));
            
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
            KRShader *shadowShader = m_pContext->getShaderManager()->getShader("ShadowShader", pCamera, std::vector<KRLight *>(), 0, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, KRNode::RENDER_PASS_FORWARD_TRANSPARENT);
            
            getContext().getShaderManager()->selectShader(*pCamera, shadowShader, m_shadowViewports[iShadow], KRMat4(), std::vector<KRLight *>(), 0, KRNode::RENDER_PASS_SHADOWMAP);
            
            
            getScene().render(pCamera, m_shadowViewports[iShadow].getVisibleBounds(), m_shadowViewports[iShadow], KRNode::RENDER_PASS_SHADOWMAP, true);
        }
    }
}



int KRLight::getShadowBufferCount()
{
    int cBuffers=0;
    for(int iBuffer=0; iBuffer < m_cShadowBuffers; iBuffer++) {
        if(shadowValid[iBuffer]) {
            cBuffers++;
        } else {
            break;
        }
    }
    return cBuffers;
}

GLuint *KRLight::getShadowTextures()
{
    return shadowDepthTexture;
}

KRViewport *KRLight::getShadowViewports()
{
    return m_shadowViewports;
}
