//
//  KRShaderManager.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 11-08-11.
//  Copyright 2011 Kearwood Software. All rights reserved.
//

#include "KRShaderManager.h"
#include <sstream> 

using namespace std;

KRShaderManager::KRShaderManager(const GLchar *szVertShaderSource, const GLchar *szFragShaderSource) {
    m_szFragShaderSource = new GLchar[strlen(szFragShaderSource)+1];
    m_szVertShaderSource = new GLchar[strlen(szVertShaderSource)+1];
    strcpy(m_szFragShaderSource, szFragShaderSource);
    strcpy(m_szVertShaderSource, szVertShaderSource);
}

KRShaderManager::~KRShaderManager() {
    delete m_szFragShaderSource;
    delete m_szVertShaderSource;
}


KRShader *KRShaderManager::getShader(KRCamera *pCamera, bool bDiffuseMap, bool bNormalMap, bool bSpecMap, int iShadowQuality) {

    char szKey[128];
    sprintf(szKey, "%d_%d_%d_%d_%d_%d_%d_%d_%d", pCamera->bEnablePerPixel, bDiffuseMap, bNormalMap, bSpecMap, pCamera->bDebugPSSM, iShadowQuality, /*pCamera->dAmbientR, pCamera->dAmbientG, pCamera->dAmbientB, pCamera->dSunR, pCamera->dSunG, pCamera->dSunB, */pCamera->bEnableAmbient, pCamera->bEnableDiffuse, pCamera->bEnableSpecular);
    
    
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
        stream << "\n#define HAS_NORMAL_MAP " << (bNormalMap ? "1" : "0");
        stream << "\n#define HAS_SPEC_MAP " << (bSpecMap ? "1" : "0");
        stream << "\n#define ENABLE_PER_PIXEL " << (pCamera->bEnablePerPixel ? "1" : "0");
        stream << "\n#define DEBUG_PSSM " << (pCamera->bDebugPSSM ? "1" : "0");
        stream << "\n#define SHADOW_QUALITY " << iShadowQuality;
        stream << "\n#define ENABLE_AMBIENT " << (pCamera->bEnableAmbient ? "1" : "0");
        stream << "\n#define ENABLE_DIFFUSE " << (pCamera->bEnableDiffuse ? "1" : "0");
        stream << "\n#define ENABLE_SPECULAR " << (pCamera->bEnableSpecular ? "1" : "0");
        stream.setf(ios::fixed,ios::floatfield);
        
        stream << "\n";
        std::string options = stream.str();
        
        pShader = new KRShader(options, m_szVertShaderSource, m_szFragShaderSource);

        m_shaders[szKey] = pShader;
    }
    return pShader;
}
