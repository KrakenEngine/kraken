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


KRShader *KRShaderManager::getShader(std::string shader_name, KRCamera *pCamera, bool bDiffuseMap, bool bNormalMap, bool bSpecMap, int iShadowQuality, bool bLightMap, bool bDiffuseMapScale,bool bSpecMapScale, bool bNormalMapScale, bool bDiffuseMapOffset, bool bSpecMapOffset, bool bNormalMapOffset, bool bAlphaTest, bool bAlphaBlend, KRNode::RenderPass renderPass) {

    char szKey[256];
    sprintf(szKey, "%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%i_%s", pCamera->bEnablePerPixel, bAlphaTest, bAlphaBlend, bDiffuseMap, bNormalMap, bSpecMap, pCamera->bDebugPSSM, iShadowQuality, pCamera->bEnableAmbient, pCamera->bEnableDiffuse, pCamera->bEnableSpecular, bLightMap, bDiffuseMapScale, bSpecMapScale, bNormalMapScale, bDiffuseMapOffset, bSpecMapOffset, bNormalMapOffset, renderPass, shader_name.c_str());
    
    
    /*
    const char *options = "#define HAS_DIFFUSE_MAP 1\n#define HAS_NORMAL_MAP 0\n#define HAS_SPEC_MAP 0\n#define ENABLE_PER_PIXEL 0\n#define DEBUG_PSSM 0\n#define SHADOW_QUALITY 0\n#define SUN_INTENSITY 1.5\n#define AMBIENT_INTENSITY_R 0.25\n#define AMBIENT_INTENSITY_G 0.25\n#define AMBIENT_INTENSITY_B 0.25\n";
    */
    KRShader *pShader = m_shaders[szKey];
    
    if(pShader == NULL) {
        if(m_shaders.size() > KRENGINE_MAX_SHADER_HANDLES) {
            // Keep the size of the shader cache reasonable
            std::map<std::string, KRShader *>::iterator itr = m_shaders.begin();
            delete (*itr).second;
            m_shaders.erase(itr);
        }
        
        stringstream stream;
        stream.precision(std::numeric_limits<long double>::digits10);
        
        stream << "#define HAS_DIFFUSE_MAP " << (bDiffuseMap ? "1" : "0");
        stream << "\n#define ALPHA_TEST " << (bAlphaTest ? "1" : "0");
        stream << "\n#define ALPHA_BLEND " << (bAlphaBlend ? "1" : "0");
        stream << "\n#define HAS_NORMAL_MAP " << (bNormalMap ? "1" : "0");
        stream << "\n#define HAS_SPEC_MAP " << (bSpecMap ? "1" : "0");
        stream << "\n#define HAS_LIGHT_MAP " << (bLightMap ? "1" : "0");
        stream << "\n#define HAS_NORMAL_MAP_SCALE " << (bNormalMapScale ? "1" : "0");
        stream << "\n#define HAS_SPEC_MAP_SCALE " << (bSpecMapScale ? "1" : "0");
        stream << "\n#define HAS_DIFFUSE_MAP_SCALE " << (bDiffuseMapScale ? "1" : "0");
        stream << "\n#define HAS_NORMAL_MAP_OFFSET " << (bNormalMapOffset ? "1" : "0");
        stream << "\n#define HAS_SPEC_MAP_OFFSET " << (bSpecMapOffset ? "1" : "0");
        stream << "\n#define HAS_DIFFUSE_MAP_OFFSET " << (bDiffuseMapOffset ? "1" : "0");
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
        
        
        stream.setf(ios::fixed,ios::floatfield);
        
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
