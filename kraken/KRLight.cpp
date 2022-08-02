//
//  KRLight.cpp
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
#include "KRLight.h"

#include "KRNode.h"
#include "KRCamera.h"
#include "KRContext.h"

#include "KRPipelineManager.h"
#include "KRPipeline.h"
#include "KRDirectionalLight.h"
#include "KRSpotLight.h"
#include "KRPointLight.h"

/* static */
void KRLight::InitNodeInfo(KrNodeInfo* nodeInfo)
{
  KRNode::InitNodeInfo(nodeInfo);
  nodeInfo->light.casts_shadow = true;
  nodeInfo->light.color = Vector3::One();
  nodeInfo->light.decay_start = 0.0f;
  nodeInfo->light.dust_particle_density = 0.1f;
  nodeInfo->light.dust_particle_intensity = 1.0f;
  nodeInfo->light.dust_particle_size = 1.0f;
  nodeInfo->light.flare_occlusion_size = 0.05f;
  nodeInfo->light.flare_size = 0.0f;
  nodeInfo->light.flare_texture = -1;
  nodeInfo->light.intensity = 1.0f;
  nodeInfo->light.light_shafts = true;
}

KRLight::KRLight(KRScene &scene, std::string name) : KRNode(scene, name)
{
    m_intensity = 1.0f;
    m_dust_particle_intensity = 1.0f;
    m_color = Vector3::One();
    m_flareTexture = "";
    m_pFlareTexture = NULL;
    m_flareSize = 0.0f;
    m_flareOcclusionSize = 0.05f;
    m_casts_shadow = true;
    m_light_shafts = true;
    m_dust_particle_density = 0.1f;
    m_dust_particle_size = 1.0f;
    m_dust_particle_intensity = 1.0f;
    m_occlusionQuery = 0;
    m_decayStart = 0;
    
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
    m_color = Vector3::Create(x,y,z);
    
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
        m_flareOcclusionSize = 0.05f;
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

const Vector3 &KRLight::getColor() {
    return m_color;
}

void KRLight::setColor(const Vector3 &color) {
    m_color = color;
}

void KRLight::setDecayStart(float decayStart) {
    m_decayStart = decayStart;
}

float KRLight::getDecayStart() {
    return m_decayStart;
}

void KRLight::render(RenderInfo& ri) {

    if(m_lod_visible <= LOD_VISIBILITY_PRESTREAM) return;
    
    KRNode::render(ri);
    
    if(ri.renderPass == KRNode::RENDER_PASS_GENERATE_SHADOWMAPS && (ri.camera->settings.volumetric_environment_enable || ri.camera->settings.dust_particle_enable || (ri.camera->settings.m_cShadowBuffers > 0 && m_casts_shadow))) {
        allocateShadowBuffers(configureShadowBufferViewports(ri.viewport));
        renderShadowBuffers(ri);
    }
    
    if(ri.renderPass == KRNode::RENDER_PASS_ADDITIVE_PARTICLES && ri.camera->settings.dust_particle_enable) {
        // Render brownian particles for dust floating in air
        if(m_cShadowBuffers >= 1 && shadowValid[0] && m_dust_particle_density > 0.0f && m_dust_particle_size > 0.0f && m_dust_particle_intensity > 0.0f) {
            
            if(ri.viewport.visible(getBounds()) || true) { // FINDME, HACK need to remove "|| true"?
                
                float particle_range = 600.0f;
                
                int particle_count = (int)(m_dust_particle_density * pow(particle_range, 3));
                if (particle_count > KRMeshManager::KRENGINE_MAX_RANDOM_PARTICLES) {
                  particle_count = KRMeshManager::KRENGINE_MAX_RANDOM_PARTICLES;
                }

                Matrix4 particleModelMatrix;
                particleModelMatrix.scale(particle_range);  // Scale the box symetrically to ensure that we don't have an uneven distribution of particles for different angles of the view frustrum
                particleModelMatrix.translate(ri.viewport.getCameraPosition());
                
                std::vector<KRDirectionalLight *> this_directional_light;
                std::vector<KRSpotLight *> this_spot_light;
                std::vector<KRPointLight *> this_point_light;
                KRDirectionalLight *directional_light = dynamic_cast<KRDirectionalLight *>(this);
                KRSpotLight *spot_light = dynamic_cast<KRSpotLight *>(this);
                KRPointLight *point_light = dynamic_cast<KRPointLight *>(this);
                if(directional_light) {
                    this_directional_light.push_back(directional_light);
                }
                if(spot_light) {
                    this_spot_light.push_back(spot_light);
                }
                if(point_light) {
                    this_point_light.push_back(point_light);
                }
                
                PipelineInfo info{};
                std::string shader_name("dust_particle");
                info.shader_name = &shader_name;
                info.pCamera = ri.camera;
                info.point_lights = &this_point_light;
                info.directional_lights = &this_directional_light;
                info.spot_lights = &this_spot_light;
                info.renderPass = ri.renderPass;
                info.rasterMode = RasterMode::kAdditive;
                info.cullMode = CullMode::kCullNone;
                info.vertexAttributes = (1 << KRMesh::KRENGINE_ATTRIB_VERTEX) | (1 << KRMesh::KRENGINE_ATTRIB_TEXUVA);
                info.modelFormat = ModelFormat::KRENGINE_MODEL_FORMAT_TRIANGLES;
                KRPipeline *pParticleShader = m_pContext->getPipelineManager()->getPipeline(*ri.surface, info);

                pParticleShader->setUniform(KRPipeline::Uniform::KRENGINE_UNIFORM_LIGHT_COLOR, m_color * ri.camera->settings.dust_particle_intensity * m_dust_particle_intensity * m_intensity);
                pParticleShader->setUniform(KRPipeline::Uniform::KRENGINE_UNIFORM_PARTICLE_ORIGIN, Matrix4::DotWDiv(Matrix4::Invert(particleModelMatrix), Vector3::Zero()));
                pParticleShader->setUniform(KRPipeline::Uniform::KRENGINE_UNIFORM_FLARE_SIZE, m_dust_particle_size);
                pParticleShader->bind(ri.commandBuffer, *ri.camera, ri.viewport, particleModelMatrix, &this_point_light, &this_directional_light, &this_spot_light, ri.renderPass);
                
                m_pContext->getMeshManager()->bindVBO(ri.commandBuffer, &m_pContext->getMeshManager()->KRENGINE_VBO_DATA_RANDOM_PARTICLES, 1.0f);

                vkCmdDraw(ri.commandBuffer, particle_count * 3, 1, 0, 0);
            }
        }
    }

    
    if(ri.renderPass == KRNode::RENDER_PASS_VOLUMETRIC_EFFECTS_ADDITIVE && ri.camera->settings.volumetric_environment_enable && m_light_shafts) {
        std::string shader_name = ri.camera->settings.volumetric_environment_downsample != 0 ? "volumetric_fog_downsampled" : "volumetric_fog";
        
        std::vector<KRDirectionalLight *> this_directional_light;
        std::vector<KRSpotLight *> this_spot_light;
        std::vector<KRPointLight *> this_point_light;
        KRDirectionalLight *directional_light = dynamic_cast<KRDirectionalLight *>(this);
        KRSpotLight *spot_light = dynamic_cast<KRSpotLight *>(this);
        KRPointLight *point_light = dynamic_cast<KRPointLight *>(this);
        if(directional_light) {
            this_directional_light.push_back(directional_light);
        }
        if(spot_light) {
            this_spot_light.push_back(spot_light);
        }
        if(point_light) {
            this_point_light.push_back(point_light);
        }

        PipelineInfo info{};
        info.shader_name = &shader_name;
        info.pCamera = ri.camera;
        info.point_lights = &this_point_light;
        info.directional_lights = &this_directional_light;
        info.spot_lights = &this_spot_light;
        info.renderPass = KRNode::RENDER_PASS_ADDITIVE_PARTICLES;
        info.rasterMode = RasterMode::kAdditive;
        info.cullMode = CullMode::kCullNone;
        info.vertexAttributes = (1 << KRMesh::KRENGINE_ATTRIB_VERTEX);
        info.modelFormat = ModelFormat::KRENGINE_MODEL_FORMAT_TRIANGLES;
        
        KRPipeline *pFogShader = m_pContext->getPipelineManager()->getPipeline(*ri.surface, info);

        int slice_count = (int)(ri.camera->settings.volumetric_environment_quality * 495.0) + 5;
            
        float slice_near = -ri.camera->settings.getPerspectiveNearZ();
        float slice_far = -ri.camera->settings.volumetric_environment_max_distance;
        float slice_spacing = (slice_far - slice_near) / slice_count;
            
        pFogShader->setUniform(KRPipeline::Uniform::KRENGINE_UNIFORM_SLICE_DEPTH_SCALE, Vector2::Create(slice_near, slice_spacing));
        pFogShader->setUniform(KRPipeline::Uniform::KRENGINE_UNIFORM_LIGHT_COLOR, (m_color * ri.camera->settings.volumetric_environment_intensity * m_intensity * -slice_spacing / 1000.0f));
        pFogShader->bind(ri.commandBuffer, *ri.camera, ri.viewport, Matrix4(), &this_point_light, &this_directional_light, &this_spot_light, KRNode::RENDER_PASS_VOLUMETRIC_EFFECTS_ADDITIVE);
        
        m_pContext->getMeshManager()->bindVBO(ri.commandBuffer, &m_pContext->getMeshManager()->KRENGINE_VBO_DATA_VOLUMETRIC_LIGHTING, 1.0f);
        vkCmdDraw(ri.commandBuffer, slice_count * 6, 1, 0, 0);

    }
    
    if(ri.renderPass == KRNode::RENDER_PASS_PARTICLE_OCCLUSION) {
        if(m_flareTexture.size() && m_flareSize > 0.0f) {
            KRMesh* sphereModel = getContext().getMeshManager()->getMaxLODModel("__sphere");
            if (sphereModel) {
            
              Matrix4 occlusion_test_sphere_matrix = Matrix4();
              occlusion_test_sphere_matrix.scale(m_localScale * m_flareOcclusionSize);
              occlusion_test_sphere_matrix.translate(m_localTranslation);
              if(m_parentNode) {
                  occlusion_test_sphere_matrix *= m_parentNode->getModelMatrix();
              }

              PipelineInfo info{};
              std::string shader_name("occlusion_test");
              info.shader_name = &shader_name;
              info.pCamera = ri.camera;
              info.point_lights = &ri.point_lights;
              info.directional_lights = &ri.directional_lights;
              info.spot_lights = &ri.spot_lights;
              info.renderPass = ri.renderPass;
              info.rasterMode = RasterMode::kAdditive;
              info.cullMode = CullMode::kCullNone;
              info.modelFormat = sphereModel->getModelFormat();
              info.vertexAttributes = sphereModel->getVertexAttributes();

              KRPipeline* pPipeline = getContext().getPipelineManager()->getPipeline(*ri.surface, info);
              pPipeline->bind(ri.commandBuffer, *info.pCamera, ri.viewport, occlusion_test_sphere_matrix, info.point_lights, info.directional_lights, info.spot_lights, info.renderPass);

              GLDEBUG(glGenQueriesEXT(1, &m_occlusionQuery));
  #if TARGET_OS_IPHONE || defined(ANDROID)
              GLDEBUG(glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, m_occlusionQuery));
  #else
              GLDEBUG(glBeginQuery(GL_SAMPLES_PASSED, m_occlusionQuery));
  #endif

              sphereModel->renderNoMaterials(ri.commandBuffer, ri.renderPass, getName(), "occlusion_test", 1.0f);

#if TARGET_OS_IPHONE || defined(ANDROID)
              GLDEBUG(glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT));
#else
              GLDEBUG(glEndQuery(GL_SAMPLES_PASSED));
#endif
            }

        }
    }
    
    if(ri.renderPass == KRNode::RENDER_PASS_ADDITIVE_PARTICLES) {
        if(m_flareTexture.size() && m_flareSize > 0.0f) {
            
            if(m_occlusionQuery) {
                GLuint params = 0;
                GLDEBUG(glGetQueryObjectuivEXT(m_occlusionQuery, GL_QUERY_RESULT_EXT, &params));
                GLDEBUG(glDeleteQueriesEXT(1, &m_occlusionQuery));
                
                if(params) {
                    
                    if(!m_pFlareTexture && m_flareTexture.size()) {
                        m_pFlareTexture = getContext().getTextureManager()->getTexture(m_flareTexture);
                    }
                    
                    if(m_pFlareTexture) {
                        KRMeshManager::KRVBOData& vertices = getContext().getMeshManager()->KRENGINE_VBO_DATA_2D_SQUARE_VERTICES;

                        // Render light flare on transparency pass
                        PipelineInfo info{};
                        std::string shader_name("flare");
                        info.shader_name = &shader_name;
                        info.pCamera = ri.camera;
                        info.point_lights = &ri.point_lights;
                        info.directional_lights = &ri.directional_lights;
                        info.spot_lights = &ri.spot_lights;
                        info.renderPass = ri.renderPass;
                        info.rasterMode = RasterMode::kAdditiveNoTest;
                        info.cullMode = CullMode::kCullNone;
                        info.vertexAttributes = vertices.getVertexAttributes();
                        info.modelFormat = ModelFormat::KRENGINE_MODEL_FORMAT_STRIP;


                        KRPipeline *pShader = getContext().getPipelineManager()->getPipeline(*ri.surface, info);
                        pShader->setUniform(KRPipeline::Uniform::KRENGINE_UNIFORM_MATERIAL_ALPHA, 1.0f);
                        pShader->setUniform(KRPipeline::Uniform::KRENGINE_UNIFORM_FLARE_SIZE, m_flareSize);
                        pShader->bind(ri.commandBuffer, *ri.camera, ri.viewport, getModelMatrix(), &ri.point_lights, &ri.directional_lights, &ri.spot_lights, ri.renderPass);

                        m_pContext->getTextureManager()->selectTexture(0, m_pFlareTexture, 0.0f, KRTexture::TEXTURE_USAGE_LIGHT_FLARE);
                        m_pContext->getMeshManager()->bindVBO(ri.commandBuffer, &vertices, 1.0f);
                        vkCmdDraw(ri.commandBuffer, 4, 1, 0, 0);
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
        Vector2 viewportSize = m_shadowViewports[iShadow].getSize();
        
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
            m_pContext->getTextureManager()->_setWrapModeS(shadowDepthTexture[iShadow], GL_CLAMP_TO_EDGE);
            m_pContext->getTextureManager()->_setWrapModeT(shadowDepthTexture[iShadow], GL_CLAMP_TO_EDGE);
#if GL_EXT_shadow_samplers
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_EXT, GL_COMPARE_REF_TO_TEXTURE_EXT)); // TODO - Detect GL_EXT_shadow_samplers and only activate if available
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_EXT, GL_LEQUAL)); // TODO - Detect GL_EXT_shadow_samplers and only activate if available
#endif
            GLDEBUG(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, (GLsizei)viewportSize.x, (GLsizei)viewportSize.y, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL));
            
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

void KRLight::renderShadowBuffers(RenderInfo& ri)
{
    for(int iShadow=0; iShadow < m_cShadowBuffers; iShadow++) {
        if(!shadowValid[iShadow]) {
            shadowValid[iShadow] = true;
            
            GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer[iShadow]));
            GLDEBUG(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepthTexture[iShadow], 0));
            
            GLDEBUG(glViewport(0, 0, (GLsizei)m_shadowViewports[iShadow].getSize().x, (GLsizei)m_shadowViewports[iShadow].getSize().y));
            
            GLDEBUG(glClearDepthf(0.0f));
            GLDEBUG(glClear(GL_DEPTH_BUFFER_BIT));
            
            GLDEBUG(glViewport(1, 1, (GLsizei)m_shadowViewports[iShadow].getSize().x - 2, (GLsizei)m_shadowViewports[iShadow].getSize().y - 2));

            GLDEBUG(glClearDepthf(1.0f));
            
            GLDEBUG(glClear(GL_DEPTH_BUFFER_BIT));
            
            GLDEBUG(glDisable(GL_DITHER));
            
            // Use shader program
            PipelineInfo info{};
            std::string shader_name("ShadowShader");
            info.shader_name = &shader_name;
            info.pCamera = ri.camera;
            info.renderPass = KRNode::RENDER_PASS_FORWARD_TRANSPARENT;
            info.rasterMode = RasterMode::kOpaqueLessTest; // TODO - This is sub-optimal.  Evaluate increasing depth buffer resolution instead of disabling depth test.
            info.cullMode = CullMode::kCullNone; // Disabling culling, which eliminates some self-cast shadow artifacts
            KRPipeline *shadowShader = m_pContext->getPipelineManager()->getPipeline(*ri.surface, info);

            shadowShader->bind(ri.commandBuffer, *ri.camera, m_shadowViewports[iShadow], Matrix4(), nullptr, nullptr, nullptr, KRNode::RENDER_PASS_SHADOWMAP);
            
            getScene().render(ri.commandBuffer, *ri.surface, ri.camera, m_shadowViewports[iShadow].getVisibleBounds(), m_shadowViewports[iShadow], KRNode::RENDER_PASS_SHADOWMAP, true);
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
