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

#include "KRShaderManager.h"
#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

KRShaderManager::KRShaderManager(KRContext &context) : KRContextObject(context) {

}

KRShaderManager::~KRShaderManager() {

}


KRShader *KRShaderManager::getShader(const std::string &shader_name, const KRCamera *pCamera, bool bDiffuseMap, bool bNormalMap, bool bSpecMap, bool bReflectionMap, bool bReflectionCubeMap, int iShadowQuality, bool bLightMap, bool bDiffuseMapScale,bool bSpecMapScale, bool bNormalMapScale, bool bReflectionMapScale, bool bDiffuseMapOffset, bool bSpecMapOffset, bool bNormalMapOffset, bool bReflectionMapOffset, bool bAlphaTest, bool bAlphaBlend, KRNode::RenderPass renderPass) {

    char szKey[256];
    sprintf(szKey, "%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%i_%s_%i_%d_%d_%f_%f_%f_%f_%f_%f_%f", pCamera->bEnablePerPixel, bAlphaTest, bAlphaBlend, bDiffuseMap, bNormalMap, bSpecMap, bReflectionMap, bReflectionCubeMap, pCamera->bDebugPSSM, iShadowQuality, pCamera->bEnableAmbient, pCamera->bEnableDiffuse, pCamera->bEnableSpecular, bLightMap, bDiffuseMapScale, bSpecMapScale, bReflectionMapScale, bNormalMapScale, bDiffuseMapOffset, bSpecMapOffset, bReflectionMapOffset, bNormalMapOffset,pCamera->volumetric_environment_enable, renderPass, shader_name.c_str(),pCamera->dof_quality,pCamera->bEnableFlash,pCamera->bEnableVignette,pCamera->dof_depth,pCamera->dof_falloff,pCamera->flash_depth,pCamera->flash_falloff,pCamera->flash_intensity,pCamera->vignette_radius,pCamera->vignette_falloff);
    
    KRShader *pShader = m_shaders[szKey];
    
    if(pShader == NULL) {
        if(m_shaders.size() > KRContext::KRENGINE_MAX_SHADER_HANDLES) {
            // Keep the size of the shader cache reasonable
            std::map<std::string, KRShader *>::iterator itr = m_shaders.begin();
            delete (*itr).second;
            m_shaders.erase(itr);
            fprintf(stderr, "Swapping shaders...\n");
        }
        
        stringstream stream;
        stream.precision(std::numeric_limits<long double>::digits10);
        
        stream << "#define HAS_DIFFUSE_MAP " << (bDiffuseMap ? "1" : "0");
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
        
        stream << "\n#define ENABLE_PER_PIXEL " << (pCamera->bEnablePerPixel ? "1" : "0");
        stream << "\n#define DEBUG_PSSM " << (pCamera->bDebugPSSM ? "1" : "0");
        stream << "\n#define SHADOW_QUALITY " << iShadowQuality;
        stream << "\n#define ENABLE_AMBIENT " << (pCamera->bEnableAmbient ? "1" : "0");
        stream << "\n#define ENABLE_DIFFUSE " << (pCamera->bEnableDiffuse ? "1" : "0");
        stream << "\n#define ENABLE_SPECULAR " << (pCamera->bEnableSpecular ? "1" : "0");
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

        
        stream << "\n#define DOF_QUALITY " << pCamera->dof_quality;
        stream << "\n#define ENABLE_FLASH " << (pCamera->bEnableFlash ? "1" : "0");
        stream << "\n#define ENABLE_VIGNETTE " << (pCamera->bEnableVignette ? "1" : "0");
        stream << "\n#define ENABLE_VOLUMETRIC_ENVIRONMENT " << (pCamera->volumetric_environment_enable ? "1" : "0");

        
        
        stream.setf(ios::fixed,ios::floatfield);
        
        stream.precision(std::numeric_limits<long double>::digits10);
        
        stream << "\n#define DOF_DEPTH " << pCamera->dof_depth;
        stream << "\n#define DOF_FALLOFF " << pCamera->dof_falloff;
        stream << "\n#define FLASH_DEPTH " << pCamera->flash_depth;
        stream << "\n#define FLASH_FALLOFF " << pCamera->flash_falloff;
        stream << "\n#define FLASH_INTENSITY " << pCamera->flash_intensity;
        stream << "\n#define VIGNETTE_RADIUS " << pCamera->vignette_radius;
        stream << "\n#define VIGNETTE_FALLOFF " << pCamera->vignette_falloff;
        
        stream << "\n";
        std::string options = stream.str();
        
        pShader = new KRShader(szKey, options, m_vertShaderSource[shader_name], m_fragShaderSource[shader_name]);

        m_shaders[szKey] = pShader;
    }
    return pShader;
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
