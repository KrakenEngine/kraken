//
//  KRShaderManager.cpp
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

#include "KREngine-common.h"

#include "KRShaderManager.h"
#include "KRLight.h"
#include "KRDirectionalLight.h"
#include "KRSpotLight.h"
#include "KRPointLight.h"

using namespace std;

KRShaderManager::KRShaderManager(KRContext &context) : KRContextObject(context) {
    m_active_shader = NULL;
}

KRShaderManager::~KRShaderManager() {

}


KRShader *KRShaderManager::getShader(const std::string &shader_name, KRCamera *pCamera, const std::vector<KRPointLight *> &point_lights, const std::vector<KRDirectionalLight *> &directional_lights, const std::vector<KRSpotLight *>&spot_lights, int bone_count, bool bDiffuseMap, bool bNormalMap, bool bSpecMap, bool bReflectionMap, bool bReflectionCubeMap, bool bLightMap, bool bDiffuseMapScale,bool bSpecMapScale, bool bNormalMapScale, bool bReflectionMapScale, bool bDiffuseMapOffset, bool bSpecMapOffset, bool bNormalMapOffset, bool bReflectionMapOffset, bool bAlphaTest, bool bAlphaBlend, KRNode::RenderPass renderPass) {
    
    int iShadowQuality = 0; // FINDME - HACK - Placeholder code, need to iterate through lights and dynamically build shader

    
    int light_directional_count = 0;
    int light_point_count = 0;
    int light_spot_count = 0;
    if(renderPass != KRNode::RENDER_PASS_DEFERRED_LIGHTS && renderPass != KRNode::RENDER_PASS_DEFERRED_GBUFFER && renderPass != KRNode::RENDER_PASS_DEFERRED_OPAQUE && renderPass != KRNode::RENDER_PASS_GENERATE_SHADOWMAPS) {
        light_directional_count = directional_lights.size();
        light_point_count = point_lights.size();
        light_spot_count = spot_lights.size();
        for(std::vector<KRDirectionalLight *>::const_iterator light_itr=directional_lights.begin(); light_itr != directional_lights.end(); light_itr++) {
            KRDirectionalLight *directional_light =(*light_itr);
            iShadowQuality = directional_light->getShadowBufferCount();
        }
    }
    
    if(iShadowQuality > pCamera->settings.m_cShadowBuffers) {
        iShadowQuality = pCamera->settings.m_cShadowBuffers;
    }
    
    std::pair<std::string, std::vector<int> > key;
    key.first = shader_name;
    key.second.push_back(light_directional_count);
    key.second.push_back(light_point_count);
    key.second.push_back(light_spot_count);
    key.second.push_back(pCamera->settings.fog_type);
    key.second.push_back(pCamera->settings.bEnablePerPixel);
    key.second.push_back(bAlphaTest);
    key.second.push_back(bAlphaBlend);
    key.second.push_back(bDiffuseMap);
    key.second.push_back(bNormalMap);
    key.second.push_back(bSpecMap);
    key.second.push_back(bReflectionMap);
    key.second.push_back(bone_count);
    key.second.push_back(bSpecMap);
    key.second.push_back(bReflectionMap);
    key.second.push_back(bReflectionCubeMap);
    key.second.push_back(pCamera->settings.bDebugPSSM);
    key.second.push_back(iShadowQuality);
    key.second.push_back(pCamera->settings.bEnableAmbient);
    key.second.push_back(pCamera->settings.bEnableDiffuse);
    key.second.push_back(pCamera->settings.bEnableSpecular);
    key.second.push_back(bLightMap);
    key.second.push_back(bDiffuseMapScale);
    key.second.push_back(bSpecMapScale);
    key.second.push_back(bReflectionMapScale);
    key.second.push_back(bNormalMapScale);
    key.second.push_back(bDiffuseMapOffset);
    key.second.push_back(bSpecMapOffset);
    key.second.push_back(bReflectionMapOffset);
    key.second.push_back(bNormalMapOffset);
    key.second.push_back(pCamera->settings.volumetric_environment_enable);
    key.second.push_back(pCamera->settings.volumetric_environment_downsample != 0);
    key.second.push_back(renderPass);
    key.second.push_back(pCamera->settings.dof_quality);
    key.second.push_back(pCamera->settings.bEnableFlash);
    key.second.push_back(pCamera->settings.bEnableVignette);
    key.second.push_back(pCamera->settings.dof_depth * 1000.0f);
    key.second.push_back(pCamera->settings.dof_falloff * 1000.0f);
    key.second.push_back(pCamera->settings.flash_depth * 1000.0f);
    key.second.push_back(pCamera->settings.flash_falloff * 1000.0f);
    key.second.push_back(pCamera->settings.flash_intensity * 1000.0f);
    key.second.push_back(pCamera->settings.vignette_radius * 1000.0f);
    key.second.push_back(pCamera->settings.vignette_falloff * 1000.0f);
              
                         
    KRShader *pShader = m_shaders[key];
    
    
    if(pShader == NULL) {
        if(m_shaders.size() > KRContext::KRENGINE_MAX_SHADER_HANDLES) {
            // Keep the size of the shader cache reasonable
            std::map<std::pair<std::string, std::vector<int> > , KRShader *>::iterator itr = m_shaders.begin();
            delete (*itr).second;
            m_shaders.erase(itr);
            fprintf(stderr, "Swapping shaders...\n");
        }
        
        
        std::string platform_shader_name = shader_name;
#if TARGET_OS_IPHONE
        platform_shader_name = shader_name;
#else
        platform_shader_name = shader_name + "_osx";
#endif
        
        stringstream stream;
        stream.precision(std::numeric_limits<long double>::digits10);

#if TARGET_OS_IPHONE
#else
        stream << "\n#version 120";
        stream << "\n#define lowp";
        stream << "\n#define mediump";
        stream << "\n#define highp";
#endif
        
        stream << "\n#define LIGHT_DIRECTIONAL_COUNT " << light_directional_count;
        stream << "\n#define LIGHT_POINT_COUNT " << light_point_count;
        stream << "\n#define LIGHT_SPOT_COUNT " << light_spot_count;
        stream << "\n#define BONE_COUNT " << bone_count;
        
        stream << "\n#define HAS_DIFFUSE_MAP " << (bDiffuseMap ? "1" : "0");
        stream << "\n#define HAS_DIFFUSE_MAP_SCALE " << (bDiffuseMapScale ? "1" : "0");
        stream << "\n#define HAS_DIFFUSE_MAP_OFFSET " << (bDiffuseMapOffset ? "1" : "0");
        
        stream << "\n#define HAS_SPEC_MAP " << (bSpecMap ? "1" : "0");
        stream << "\n#define HAS_SPEC_MAP_SCALE " << (bSpecMapScale ? "1" : "0");
        stream << "\n#define HAS_SPEC_MAP_OFFSET " << (bSpecMapOffset ? "1" : "0");
        
        stream << "\n#define HAS_NORMAL_MAP " << (bNormalMap ? "1" : "0");
        stream << "\n#define HAS_NORMAL_MAP_SCALE " << (bNormalMapScale ? "1" : "0");
        stream << "\n#define HAS_NORMAL_MAP_OFFSET " << (bNormalMapOffset ? "1" : "0");
        
        stream << "\n#define HAS_REFLECTION_MAP " << (bReflectionMap ? "1" : "0");
        stream << "\n#define HAS_REFLECTION_MAP_SCALE " << (bReflectionMapScale ? "1" : "0");
        stream << "\n#define HAS_REFLECTION_MAP_OFFSET " << (bReflectionMapOffset ? "1" : "0");
        
        stream << "\n#define HAS_LIGHT_MAP " << (bLightMap ? "1" : "0");
        stream << "\n#define HAS_REFLECTION_CUBE_MAP " << (bReflectionCubeMap ? "1" : "0");
        
        stream << "\n#define ALPHA_TEST " << (bAlphaTest ? "1" : "0");
        stream << "\n#define ALPHA_BLEND " << (bAlphaBlend ? "1" : "0");
        
        stream << "\n#define ENABLE_PER_PIXEL " << (pCamera->settings.bEnablePerPixel ? "1" : "0");
        stream << "\n#define DEBUG_PSSM " << (pCamera->settings.bDebugPSSM ? "1" : "0");
        stream << "\n#define SHADOW_QUALITY " << iShadowQuality;
        stream << "\n#define ENABLE_AMBIENT " << (pCamera->settings.bEnableAmbient ? "1" : "0");
        stream << "\n#define ENABLE_DIFFUSE " << (pCamera->settings.bEnableDiffuse ? "1" : "0");
        stream << "\n#define ENABLE_SPECULAR " << (pCamera->settings.bEnableSpecular ? "1" : "0");
        stream << "\n#define FOG_TYPE " << pCamera->settings.fog_type;
        switch(renderPass) {
            case KRNode::RENDER_PASS_DEFERRED_GBUFFER:
                stream << "\n#define GBUFFER_PASS " << 1;
                break;
            case KRNode::RENDER_PASS_DEFERRED_LIGHTS:
                stream << "\n#define GBUFFER_PASS " << 2;
                break;
            case KRNode::RENDER_PASS_DEFERRED_OPAQUE:
                stream << "\n#define GBUFFER_PASS " << 3;
                break;
            default:
                stream << "\n#define GBUFFER_PASS " << 0;
                break;
        }

        stream << "\n#define DOF_QUALITY " << pCamera->settings.dof_quality;
        stream << "\n#define ENABLE_FLASH " << (pCamera->settings.bEnableFlash ? "1" : "0");
        stream << "\n#define ENABLE_VIGNETTE " << (pCamera->settings.bEnableVignette ? "1" : "0");
        stream << "\n#define VOLUMETRIC_ENVIRONMENT_DOWNSAMPLED " << (pCamera->settings.volumetric_environment_enable && pCamera->settings.volumetric_environment_downsample != 0 ? "1" : "0");

        
        
        stream.setf(ios::fixed,ios::floatfield);
        
        stream.precision(std::numeric_limits<long double>::digits10);
        
        stream << "\n#define DOF_DEPTH " << pCamera->settings.dof_depth;
        stream << "\n#define DOF_FALLOFF " << pCamera->settings.dof_falloff;
        stream << "\n#define FLASH_DEPTH " << pCamera->settings.flash_depth;
        stream << "\n#define FLASH_FALLOFF " << pCamera->settings.flash_falloff;
        stream << "\n#define FLASH_INTENSITY " << pCamera->settings.flash_intensity;
        stream << "\n#define VIGNETTE_RADIUS " << pCamera->settings.vignette_radius;
        stream << "\n#define VIGNETTE_FALLOFF " << pCamera->settings.vignette_falloff;
        
        stream << "\n";
        std::string options = stream.str();
        
        std::string vertShaderSource = m_vertShaderSource[platform_shader_name];
        std::string fragShaderSource = m_fragShaderSource[platform_shader_name];
        
        if(vertShaderSource.length() == 0) {
            fprintf(stderr, "ERROR: Vertex Shader Missing: %s\n", platform_shader_name.c_str());
        }
        if(fragShaderSource.length() == 0) {
            fprintf(stderr, "ERROR: Fragment Shader Missing: %s\n", platform_shader_name.c_str());
        }
        
        char szKey[256];
        sprintf(szKey, "%i_%i_%i_%i_%i_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%i_%s_%i_%d_%d_%f_%f_%f_%f_%f_%f_%f", light_directional_count, light_point_count, light_spot_count, bone_count, pCamera->settings.fog_type, pCamera->settings.bEnablePerPixel,bAlphaTest, bAlphaBlend, bDiffuseMap, bNormalMap, bSpecMap, bReflectionMap, bReflectionCubeMap, pCamera->settings.bDebugPSSM, iShadowQuality, pCamera->settings.bEnableAmbient, pCamera->settings.bEnableDiffuse, pCamera->settings.bEnableSpecular, bLightMap, bDiffuseMapScale, bSpecMapScale, bReflectionMapScale, bNormalMapScale, bDiffuseMapOffset, bSpecMapOffset, bReflectionMapOffset, bNormalMapOffset,pCamera->settings.volumetric_environment_enable && pCamera->settings.volumetric_environment_downsample != 0, renderPass, shader_name.c_str(),pCamera->settings.dof_quality,pCamera->settings.bEnableFlash,pCamera->settings.bEnableVignette,pCamera->settings.dof_depth,pCamera->settings.dof_falloff,pCamera->settings.flash_depth,pCamera->settings.flash_falloff,pCamera->settings.flash_intensity,pCamera->settings.vignette_radius,pCamera->settings.vignette_falloff);
        
        pShader = new KRShader(getContext(), szKey, options, vertShaderSource, fragShaderSource);

        m_shaders[key] = pShader;
    }
    return pShader;
}

bool KRShaderManager::selectShader(const std::string &shader_name, KRCamera &camera, const std::vector<KRPointLight *> &point_lights, const std::vector<KRDirectionalLight *> &directional_lights, const std::vector<KRSpotLight *>&spot_lights, int bone_count, const KRViewport &viewport, const KRMat4 &matModel, bool bDiffuseMap, bool bNormalMap, bool bSpecMap, bool bReflectionMap, bool bReflectionCubeMap, bool bLightMap, bool bDiffuseMapScale,bool bSpecMapScale, bool bNormalMapScale, bool bReflectionMapScale, bool bDiffuseMapOffset, bool bSpecMapOffset, bool bNormalMapOffset, bool bReflectionMapOffset, bool bAlphaTest, bool bAlphaBlend, KRNode::RenderPass renderPass)
{
    KRShader *pShader = getShader(shader_name, &camera, point_lights, directional_lights, spot_lights, bone_count, bDiffuseMap, bNormalMap, bSpecMap, bReflectionMap, bReflectionCubeMap, bLightMap, bDiffuseMapScale, bSpecMapScale, bNormalMapScale, bReflectionMapScale, bDiffuseMapOffset, bSpecMapOffset, bNormalMapOffset, bReflectionMapOffset, bAlphaTest, bAlphaBlend, renderPass);
    return selectShader(camera, pShader, viewport, matModel, point_lights, directional_lights, spot_lights, bone_count, renderPass);
}

bool KRShaderManager::selectShader(KRCamera &camera, KRShader *pShader, const KRViewport &viewport, const KRMat4 &matModel, const std::vector<KRPointLight *> &point_lights, const std::vector<KRDirectionalLight *> &directional_lights, const std::vector<KRSpotLight *>&spot_lights, int bone_count, const KRNode::RenderPass &renderPass)
{
    if(pShader) {
        return pShader->bind(camera, viewport, matModel, point_lights, directional_lights, spot_lights, renderPass);
    } else {
        return false;
    }
}

void KRShaderManager::loadFragmentShader(const std::string &name, KRDataBlock *data) {
    m_fragShaderSource[name] = string((char *)data->getStart(), data->getSize());
    delete data;
}

void KRShaderManager::loadVertexShader(const std::string &name, KRDataBlock *data) {
    m_vertShaderSource[name] = string((char *)data->getStart(), data->getSize());
    delete data;
}

const std::string &KRShaderManager::getFragShaderSource(const std::string &name) {
    return m_fragShaderSource[name];
}

const std::string &KRShaderManager::getVertShaderSource(const std::string &name) {
    return m_vertShaderSource[name];
}

long KRShaderManager::getShaderHandlesUsed() {
    return m_shaders.size();
}
