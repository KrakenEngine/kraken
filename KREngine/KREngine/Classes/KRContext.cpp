//
//  KRContext.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-12.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include <iostream>

#include "KRContext.h"
#include "KRCamera.h"

int KRContext::KRENGINE_MAX_VBO_HANDLES;
int KRContext::KRENGINE_MAX_VBO_MEM;
int KRContext::KRENGINE_MAX_SHADER_HANDLES;
int KRContext::KRENGINE_MAX_TEXTURE_HANDLES;
int KRContext::KRENGINE_MAX_TEXTURE_MEM;
int KRContext::KRENGINE_TARGET_TEXTURE_MEM_MAX;
int KRContext::KRENGINE_TARGET_TEXTURE_MEM_MIN;
int KRContext::KRENGINE_MAX_TEXTURE_DIM;
int KRContext::KRENGINE_MIN_TEXTURE_DIM;

KRContext::KRContext() {
    m_pBundleManager = new KRBundleManager(*this);
    m_pShaderManager = new KRShaderManager(*this);
    m_pTextureManager = new KRTextureManager(*this);
    m_pMaterialManager = new KRMaterialManager(*this, m_pTextureManager, m_pShaderManager);
    m_pModelManager = new KRModelManager(*this);
    m_pSceneManager = new KRSceneManager(*this);
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
    
    // The bundles must be destroyed last, as the other objects may be using mmap'ed data from bundles
    if(m_pBundleManager) {
        delete m_pBundleManager;
        m_pBundleManager = NULL;
    }
}

KRBundleManager *KRContext::getBundleManager() {
    return m_pBundleManager;
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

void KRContext::loadResource(const std::string &file_name, KRDataBlock *data) {
    std::string name = KRResource::GetFileBase(file_name);
    std::string extension = KRResource::GetFileExtension(file_name);
    
//    fprintf(stderr, "KRContext::loadResource - Loading: %s\n", file_name.c_str());
    
    if(extension.compare("krbundle") == 0) {
        m_pBundleManager->loadBundle(name.c_str(), data);
    } else if(extension.compare("krobject") == 0) {
        m_pModelManager->loadModel(name.c_str(), data);
    } else if(extension.compare("krscene") == 0) {
        m_pSceneManager->loadScene(name.c_str(), data);
    } else if(extension.compare("pvr") == 0) {
        m_pTextureManager->loadTexture(name.c_str(), data);
    } else if(extension.compare("vsh") == 0) {
        m_pShaderManager->loadVertexShader(name.c_str(), data);
    } else if(extension.compare("fsh") == 0) {
        m_pShaderManager->loadFragmentShader(name.c_str(), data);
    } else if(extension.compare("mtl") == 0) {
        m_pMaterialManager->load(name.c_str(), data);
    } else {
        fprintf(stderr, "KRContext::loadResource - Unknown resource file type: %s\n", file_name.c_str());
        delete data;
    }
    
    
}

void KRContext::loadResource(std::string path) {
    KRDataBlock *data = new KRDataBlock();
    if(data->load(path)) {
        loadResource(path, data);
    } else {
        fprintf(stderr, "KRContext::loadResource - Failed to open file: %s\n", path.c_str());
        delete data;
    }
}

void KRContext::rotateBuffers(bool new_frame) {
    //fprintf(stderr, "Rotating Buffers...\n");
    GLDEBUG(glFinish());

    m_pModelManager->rotateBuffers(new_frame);
    m_pTextureManager->rotateBuffers(new_frame);
}