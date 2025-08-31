//
//  KRPipelineManager.cpp
//  Kraken Engine
//
//  Copyright 2025 Kearwood Gilbert. All rights reserved.
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

#include "KRPipelineManager.h"
#include "nodes/KRLight.h"
#include "nodes/KRDirectionalLight.h"
#include "nodes/KRSpotLight.h"
#include "nodes/KRPointLight.h"
#include "KRSwapchain.h"
#include "KRRenderPass.h"

#ifndef ANDROID
#include "glslang/Public/ShaderLang.h"
#endif

using namespace std;

KRPipelineManager::KRPipelineManager(KRContext& context) : KRContextObject(context)
{
  m_active_pipeline = NULL;
#ifndef ANDROID
  bool success = glslang::InitializeProcess();
  if (success) {
    printf("GLSLang Initialized.\n");
  } else {
    printf("Failed to initialize GLSLang.\n");
  }
#endif // ANDROID
}

KRPipelineManager::~KRPipelineManager()
{
#ifndef ANDROID
  glslang::FinalizeProcess();
#endif // ANDROID
}

KRPipeline* KRPipelineManager::getPipeline(KRSurface& surface, const PipelineInfo& info)
{
  std::pair<std::string, std::vector<int> > key;
  key.first = *info.shader_name;
  key.second.push_back(surface.m_deviceHandle);
  key.second.push_back(surface.m_swapChain->m_imageFormat);
  key.second.push_back(surface.m_swapChain->m_extent.width);
  key.second.push_back(surface.m_swapChain->m_extent.height);
  key.second.push_back(info.vertexAttributes);
  key.second.push_back((int)info.modelFormat);
  // TODO - Add renderPass unique identifier to key
  PipelineMap::iterator itr = m_pipelines.find(key);
  if (itr != m_pipelines.end()) {
    return itr->second;
  }

  std::vector<KRShader*> shaders;
  shaders.push_back(m_pContext->getShaderManager()->get(*info.shader_name + ".vert", "spv"));
  shaders.push_back(m_pContext->getShaderManager()->get(*info.shader_name + ".frag", "spv"));
  KRPipeline* pipeline = new KRPipeline(*m_pContext, surface.m_deviceHandle, info.renderPass, surface.getDimensions(), surface.getDimensions(), info, info.shader_name->c_str(), shaders, info.vertexAttributes, info.modelFormat);

  m_pipelines[key] = pipeline;

  return pipeline;
}

/*
// TODO - Vulkan Refactoring, merge with Vulkan version
KRPipeline *KRPipelineManager::getPipeline(KRSurface& surface, const PipelineInfo &info) {

    int iShadowQuality = 0; // FINDME - HACK - Placeholder code, need to iterate through lights and dynamically build shader


    int light_directional_count = 0;
    int light_point_count = 0;
    int light_spot_count = 0;
    if(info.renderPass != RenderPassType::RENDER_PASS_DEFERRED_LIGHTS && info.renderPass != RenderPassType::RENDER_PASS_DEFERRED_GBUFFER && info.renderPass != RenderPassType::RENDER_PASS_DEFERRED_OPAQUE && info.renderPass != RenderPassType::RENDER_PASS_SHADOWMAP) {
        if (info.directional_lights) {
          light_directional_count = (int)info.directional_lights->size();
        }

        if (info.point_lights) {
          light_point_count = (int)info.point_lights->size();
        }
        if (info.spot_lights) {
          light_spot_count = (int)info.spot_lights->size();
        }
        for(std::vector<KRDirectionalLight *>::const_iterator light_itr=info.directional_lights->begin(); light_itr != info.directional_lights->end(); light_itr++) {
            KRDirectionalLight *directional_light =(*light_itr);
            iShadowQuality = directional_light->getShadowBufferCount();
        }
    }

    if(iShadowQuality > info.pCamera->settings.m_cShadowBuffers) {
        iShadowQuality = info.pCamera->settings.m_cShadowBuffers;
    }

    bool bFadeColorEnabled = info.pCamera->getFadeColor().w >= 0.0f;

    std::pair<std::string, std::vector<int> > key;
    key.first = *info.shader_name;
    key.second.push_back(light_directional_count);
    key.second.push_back(light_point_count);
    key.second.push_back(light_spot_count);
    key.second.push_back(info.pCamera->settings.fog_type);
    key.second.push_back(info.pCamera->settings.bEnablePerPixel);
    key.second.push_back((int)info.rasterMode);
    key.second.push_back((int)info.cullMode);
    key.second.push_back(info.vertexAttributes);
    key.second.push_back((int)info.modelFormat);
    key.second.push_back(info.bAlphaTest);
    key.second.push_back(info.bDiffuseMap);
    key.second.push_back(info.bNormalMap);
    key.second.push_back(info.bSpecMap);
    key.second.push_back(info.bReflectionMap);
    key.second.push_back(info.bone_count);
    key.second.push_back(info.bSpecMap);
    key.second.push_back(info.bReflectionMap);
    key.second.push_back(info.bReflectionCubeMap);
    key.second.push_back(info.pCamera->settings.bDebugPSSM);
    key.second.push_back(iShadowQuality);
    key.second.push_back(info.pCamera->settings.bEnableAmbient);
    key.second.push_back(info.pCamera->settings.bEnableDiffuse);
    key.second.push_back(info.pCamera->settings.bEnableSpecular);
    key.second.push_back(info.bLightMap);
    key.second.push_back(info.bDiffuseMapScale);
    key.second.push_back(info.bSpecMapScale);
    key.second.push_back(info.bReflectionMapScale);
    key.second.push_back(info.bNormalMapScale);
    key.second.push_back(info.bDiffuseMapOffset);
    key.second.push_back(info.bSpecMapOffset);
    key.second.push_back(info.bReflectionMapOffset);
    key.second.push_back(info.bNormalMapOffset);
    key.second.push_back(info.pCamera->settings.volumetric_environment_enable);
    key.second.push_back(info.pCamera->settings.volumetric_environment_downsample != 0);
    key.second.push_back(info.renderPass);
    key.second.push_back(info.pCamera->settings.dof_quality);
    key.second.push_back(info.pCamera->settings.bEnableFlash);
    key.second.push_back(info.pCamera->settings.bEnableVignette);
    key.second.push_back((int)(info.pCamera->settings.dof_depth * 1000.0f));
    key.second.push_back((int)(info.pCamera->settings.dof_falloff * 1000.0f));
    key.second.push_back((int)(info.pCamera->settings.flash_depth * 1000.0f));
    key.second.push_back((int)(info.pCamera->settings.flash_falloff * 1000.0f));
    key.second.push_back((int)(info.pCamera->settings.flash_intensity * 1000.0f));
    key.second.push_back((int)(info.pCamera->settings.vignette_radius * 1000.0f));
    key.second.push_back((int)(info.pCamera->settings.vignette_falloff * 1000.0f));
    key.second.push_back(bFadeColorEnabled);

    KRPipeline *pPipeline = m_pipelines[key];


    if(pPipeline == NULL) {
        if(m_pipelines.size() > KRContext::KRENGINE_MAX_PIPELINE_HANDLES) {
            // Keep the size of the pipeline cache reasonable
            std::map<std::pair<std::string, std::vector<int> > , KRPipeline *>::iterator itr = m_pipelines.begin();
            delete (*itr).second;
            m_pipelines.erase(itr);
            KRContext::Log(KRContext::LOG_LEVEL_INFORMATION, "Swapping pipelines...\n");
        }

        stringstream stream;
        stream.precision(std::numeric_limits<long double>::digits10);

#if TARGET_OS_IPHONE
#else
        stream << "\n#version 150";
        stream << "\n#define lowp";
        stream << "\n#define mediump";
        stream << "\n#define highp";
#endif

        stream << "\n#define LIGHT_DIRECTIONAL_COUNT " << light_directional_count;
        stream << "\n#define LIGHT_POINT_COUNT " << light_point_count;
        stream << "\n#define LIGHT_SPOT_COUNT " << light_spot_count;
        stream << "\n#define BONE_COUNT " << info.bone_count;

        stream << "\n#define HAS_DIFFUSE_MAP " << (info.bDiffuseMap ? "1" : "0");
        stream << "\n#define HAS_DIFFUSE_MAP_SCALE " << (info.bDiffuseMapScale ? "1" : "0");
        stream << "\n#define HAS_DIFFUSE_MAP_OFFSET " << (info.bDiffuseMapOffset ? "1" : "0");

        stream << "\n#define HAS_SPEC_MAP " << (info.bSpecMap ? "1" : "0");
        stream << "\n#define HAS_SPEC_MAP_SCALE " << (info.bSpecMapScale ? "1" : "0");
        stream << "\n#define HAS_SPEC_MAP_OFFSET " << (info.bSpecMapOffset ? "1" : "0");

        stream << "\n#define HAS_NORMAL_MAP " << (info.bNormalMap ? "1" : "0");
        stream << "\n#define HAS_NORMAL_MAP_SCALE " << (info.bNormalMapScale ? "1" : "0");
        stream << "\n#define HAS_NORMAL_MAP_OFFSET " << (info.bNormalMapOffset ? "1" : "0");

        stream << "\n#define HAS_REFLECTION_MAP " << (info.bReflectionMap ? "1" : "0");
        stream << "\n#define HAS_REFLECTION_MAP_SCALE " << (info.bReflectionMapScale ? "1" : "0");
        stream << "\n#define HAS_REFLECTION_MAP_OFFSET " << (info.bReflectionMapOffset ? "1" : "0");

        stream << "\n#define HAS_LIGHT_MAP " << (info.bLightMap ? "1" : "0");
        stream << "\n#define HAS_REFLECTION_CUBE_MAP " << (info.bReflectionCubeMap ? "1" : "0");

        stream << "\n#define ALPHA_TEST " << (info.bAlphaTest ? "1" : "0");
        stream << "\n#define ALPHA_BLEND " << ((info.rasterMode == RasterMode::kAlphaBlend
          || info.rasterMode == RasterMode::kAlphaBlendNoTest
          || info.rasterMode == RasterMode::kAdditive
          || info.rasterMode == RasterMode::kAdditiveNoTest) ? "1" : "0");

        stream << "\n#define ENABLE_PER_PIXEL " << (info.pCamera->settings.bEnablePerPixel ? "1" : "0");
        stream << "\n#define DEBUG_PSSM " << (info.pCamera->settings.bDebugPSSM ? "1" : "0");
        stream << "\n#define SHADOW_QUALITY " << iShadowQuality;
        stream << "\n#define ENABLE_AMBIENT " << (info.pCamera->settings.bEnableAmbient ? "1" : "0");
        stream << "\n#define ENABLE_DIFFUSE " << (info.pCamera->settings.bEnableDiffuse ? "1" : "0");
        stream << "\n#define ENABLE_SPECULAR " << (info.pCamera->settings.bEnableSpecular ? "1" : "0");
        stream << "\n#define ENABLE_FADE_COLOR " << (bFadeColorEnabled ? "1" : "0");
        stream << "\n#define FOG_TYPE " << info.pCamera->settings.fog_type;
        switch(info.renderPass) {
            case RenderPassType::RENDER_PASS_DEFERRED_GBUFFER:
                stream << "\n#define GBUFFER_PASS " << 1;
                break;
            case RenderPassType::RENDER_PASS_DEFERRED_LIGHTS:
                stream << "\n#define GBUFFER_PASS " << 2;
                break;
            case RenderPassType::RENDER_PASS_DEFERRED_OPAQUE:
                stream << "\n#define GBUFFER_PASS " << 3;
                break;
            default:
                stream << "\n#define GBUFFER_PASS " << 0;
                break;
        }

        stream << "\n#define DOF_QUALITY " << info.pCamera->settings.dof_quality;
        stream << "\n#define ENABLE_FLASH " << (info.pCamera->settings.bEnableFlash ? "1" : "0");
        stream << "\n#define ENABLE_VIGNETTE " << (info.pCamera->settings.bEnableVignette ? "1" : "0");
        stream << "\n#define VOLUMETRIC_ENVIRONMENT_DOWNSAMPLED " << (info.pCamera->settings.volumetric_environment_enable && info.pCamera->settings.volumetric_environment_downsample != 0 ? "1" : "0");


        stream.setf(ios::fixed,ios::floatfield);

        stream.precision(std::numeric_limits<long double>::digits10);

        stream << "\n#define DOF_DEPTH " << info.pCamera->settings.dof_depth;
        stream << "\n#define DOF_FALLOFF " << info.pCamera->settings.dof_falloff;
        stream << "\n#define FLASH_DEPTH " << info.pCamera->settings.flash_depth;
        stream << "\n#define FLASH_FALLOFF " << info.pCamera->settings.flash_falloff;
        stream << "\n#define FLASH_INTENSITY " << info.pCamera->settings.flash_intensity;
        stream << "\n#define VIGNETTE_RADIUS " << info.pCamera->settings.vignette_radius;
        stream << "\n#define VIGNETTE_FALLOFF " << info.pCamera->settings.vignette_falloff;

        stream << "\n";
        std::string options = stream.str();

        KRSourceManager *sourceManager = m_pContext->getSourceManager();
        KRSource *vertSource = sourceManager->get(info.shader_name->c_str(), "vert");
        KRSource *fragSource = sourceManager->get(info.shader_name->c_str(), "frag");

        if(vertSource == nullptr) {
            KRContext::Log(KRContext::LOG_LEVEL_ERROR, "Vertex Shader Missing: %s", info.shader_name->c_str());
        }
        if(fragSource == nullptr) {
            KRContext::Log(KRContext::LOG_LEVEL_ERROR, "Fragment Shader Missing: %s", info.shader_name->c_str());
        }

        Vector4 fade_color = info.pCamera->getFadeColor();

        char szKey[256];
        sprintf(szKey, "%i_%i_%i_%i_%i_%i_%i_%i_%i_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%i_%s_%i_%d_%d_%f_%f_%f_%f_%f_%f_%f_%f_%f_%f_%f", (int)info.rasterMode, (int)info.cullMode, (int)info.modelFormat, (int)info.vertexAttributes, light_directional_count, light_point_count, light_spot_count, info.bone_count, info.pCamera->settings.fog_type, info.pCamera->settings.bEnablePerPixel, info.bAlphaTest, info.bDiffuseMap, info.bNormalMap, info.bSpecMap, info.bReflectionMap, info.bReflectionCubeMap, info.pCamera->settings.bDebugPSSM, iShadowQuality, info.pCamera->settings.bEnableAmbient, info.pCamera->settings.bEnableDiffuse, info.pCamera->settings.bEnableSpecular, info.bLightMap, info.bDiffuseMapScale, info.bSpecMapScale, info.bReflectionMapScale, info.bNormalMapScale, info.bDiffuseMapOffset, info.bSpecMapOffset, info.bReflectionMapOffset, info.bNormalMapOffset, info.pCamera->settings.volumetric_environment_enable && info.pCamera->settings.volumetric_environment_downsample != 0, info.renderPass, info.shader_name->c_str(), info.pCamera->settings.dof_quality, info.pCamera->settings.bEnableFlash, info.pCamera->settings.bEnableVignette, info.pCamera->settings.dof_depth, info.pCamera->settings.dof_falloff, info.pCamera->settings.flash_depth, info.pCamera->settings.flash_falloff, info.pCamera->settings.flash_intensity, info.pCamera->settings.vignette_radius, info.pCamera->settings.vignette_falloff, fade_color.x, fade_color.y, fade_color.z, fade_color.w);

        pPipeline = new KRPipeline(getContext(), szKey, options, vertSource->getData()->getString(), fragSource->getData()->getString());

        m_pipelines[key] = pPipeline;
    }
    return pPipeline;
}
*/

size_t KRPipelineManager::getPipelineHandlesUsed()
{
  return m_pipelines.size();
}


KRPipeline* KRPipelineManager::get(const char* name)
{
  std::pair<std::string, std::vector<int> > key;
  key.first = name;
  auto itr = m_pipelines.find(key);
  if (itr == m_pipelines.end()) {
    return nullptr;
  }
  return (*itr).second;
}
