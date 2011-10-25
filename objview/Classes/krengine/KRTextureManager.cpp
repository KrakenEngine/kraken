//
//  KRTextureManager.cpp
//  gldemo
//
//  Created by Kearwood Gilbert on 10-10-14.
//  Copyright (c) 2010 Kearwood Software. All rights reserved.
//

#include "KRTextureManager.h"
#include <string.h>

KRTextureManager::KRTextureManager() {
    
}

KRTextureManager::~KRTextureManager() {
    for(map<std::string, KRTexture *>::iterator itr = m_textures.begin(); itr != m_textures.end(); ++itr){
        delete (*itr).second;
    }
}

KRTexture *KRTextureManager::loadTexture(const char *szName, const char *szPath) {
    KRTexture *pTexture = new KRTexture();
    if(!pTexture->loadFromFile(szPath)) {
        delete pTexture;
        return NULL;
    }
    
    if(!pTexture->createGLTexture()) {
        delete pTexture;
        return NULL;
    }
    
    std::string lowerName = szName;
    std::transform(lowerName.begin(), lowerName.end(),
                   lowerName.begin(), ::tolower);
    

    
    m_textures[lowerName] = pTexture;
    return pTexture;
}

GLuint KRTextureManager::getTextureName(const char *szName) {
    KRTexture *pTexture = getTexture(szName);
    if(pTexture) {
        return pTexture->getName();
    } else {
        return NULL;
    }
}

KRTexture *KRTextureManager::getTexture(const char *szName) {
    std::string lowerName = szName;
    std::transform(lowerName.begin(), lowerName.end(),
                   lowerName.begin(), ::tolower);
    
    map<std::string, KRTexture *>::iterator itr = m_textures.find(lowerName);
    if(itr == m_textures.end()) {
        // Not found
        return NULL;
    } else {
        return (*itr).second;
    }

}