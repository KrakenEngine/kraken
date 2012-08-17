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

KRContext::KRContext() {
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

void KRContext::registerNotified(KRNotified *pNotified)
{
    m_notifiedObjects.insert(pNotified);
}

void KRContext::unregisterNotified(KRNotified *pNotified)
{
    m_notifiedObjects.erase(pNotified);
}


void KRContext::notify_sceneGraphCreate(KRNode *pNode)
{
    for(std::set<KRNotified *>::iterator itr = m_notifiedObjects.begin(); itr != m_notifiedObjects.end(); itr++) {
        (*itr)->notify_sceneGraphCreate(pNode);
    }
}
void KRContext::notify_sceneGraphDelete(KRNode *pNode)
{
    
}
void KRContext::notify_sceneGraphModify(KRNode *pNode)
{
    
}