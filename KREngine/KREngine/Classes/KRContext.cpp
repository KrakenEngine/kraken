//
//  KRContext.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-12.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include <iostream>

#include "KRContext.h"

KRContext::KRContext() {
     m_pShaderManager = new KRShaderManager();
     m_pTextureManager = new KRTextureManager();
     m_pMaterialManager = new KRMaterialManager(m_pTextureManager, m_pShaderManager);
     m_pModelManager = new KRModelManager();
     m_pSceneManager = new KRSceneManager();
}

KRContext::~KRContext() {
    if(m_pSceneManager) {
        delete m_pSceneManager;
        m_pSceneManager = NULL;
    }
    
    if(m_pModelManager) {
        delete m_pModelManager;
        m_pModelManager = NULL;
    }
    
    if(m_pTextureManager) {
        delete m_pTextureManager;
        m_pTextureManager = NULL;
    }
    
    if(m_pMaterialManager) {
        delete m_pMaterialManager;
        m_pMaterialManager = NULL;
    }
    
    if(m_pShaderManager) {
        delete m_pShaderManager;
        m_pShaderManager = NULL;
    }
}


KRSceneManager *KRContext::getSceneManager() {
    return m_pSceneManager;
}
KRTextureManager *KRContext::getTextureManager() {
    return m_pTextureManager;
}
KRMaterialManager *KRContext::getMaterialManager() {
    return m_pMaterialManager;
}
KRShaderManager *KRContext::getShaderManager() {
    return m_pShaderManager;
}
KRModelManager *KRContext::getModelManager() {
    return m_pModelManager;
}

void KRContext::loadResource(std::string path) {
    std::string name = KRResource::GetFileBase(path);
    std::string extension = KRResource::GetFileExtension(path);
    if(extension.compare("krobject") == 0) {
        m_pModelManager->loadModel(name.c_str(), path.c_str());
    } else if(extension.compare("krscene") == 0) {
        m_pSceneManager->loadScene(name.c_str(), path.c_str());
#if TARGET_OS_IPHONE
    } else if(extension.compare("pvr") == 0) {
        m_pTextureManager->loadTexture(name.c_str(), path.c_str());
#endif
    } else if(extension.compare("vsh") == 0) {
        m_pShaderManager->loadVertexShader(name.c_str(), path.c_str());
    } else if(extension.compare("fsh") == 0) {
        m_pShaderManager->loadFragmentShader(name.c_str(), path.c_str());
    } else if(extension.compare("mtl") == 0) {
        m_pMaterialManager->loadFile(path.c_str());
    } else {
        fprintf(stderr, "KRContext::loadResource - Unknown resource file type: %s\n", path.c_str());
    }
}