//
//  KRModelManager.cpp
//  gldemo
//
//  Created by Kearwood Gilbert on 10-12-31.
//  Copyright 2010 Kearwood Software. All rights reserved.
//

#include "KRModelManager.h"

KRModelManager::KRModelManager(KRMaterialManager *pMaterialManager) {
    m_pMaterialManager = pMaterialManager;
}

KRModelManager::~KRModelManager() {
    for(map<std::string, KRModel *>::iterator itr = m_models.begin(); itr != m_models.end(); ++itr){
        delete (*itr).second;
    }
    m_models.empty();
}

KRModel *KRModelManager::loadModel(const char *szName, const char *szPath) {
    KRModel *pModel = new KRModel(szPath, m_pMaterialManager);
    m_models[szName] = pModel;
    return pModel;
}

KRModel *KRModelManager::getModel(const char *szName) {
    return m_models[szName];
}

KRModel *KRModelManager::getFirstModel() {
    static std::map<std::string, KRModel *>::iterator model_itr = m_models.begin();
    return (*model_itr).second;
}
