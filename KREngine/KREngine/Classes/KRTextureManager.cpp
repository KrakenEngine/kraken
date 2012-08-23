//
//  KRTextureManager.cpp
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

#include "KRTextureManager.h"
#include <string.h>

KRTextureManager::KRTextureManager(KRContext &context) : KRContextObject(context) {
    
}

KRTextureManager::~KRTextureManager() {
    for(map<std::string, KRTexture *>::iterator itr = m_textures.begin(); itr != m_textures.end(); ++itr){
        delete (*itr).second;
    }
}

#if TARGET_OS_IPHONE

KRTexture *KRTextureManager::loadTexture(const char *szName, const char *szPath) {
    KRTexture *pTexture = new KRTexture();
    if(!pTexture->loadFromFile(szPath)) {
        delete pTexture;
        return NULL;
    }
    
    if(!pTexture->createGLTexture()) {
        if(!pTexture->createGLTexture()) { // FINDME - HACK!  The first texture fails with 0x501 return code but loads on second try
            delete pTexture;
            return NULL;
        }
    }
    
    std::string lowerName = szName;
    std::transform(lowerName.begin(), lowerName.end(),
                   lowerName.begin(), ::tolower);
    

    
    m_textures[lowerName] = pTexture;
    return pTexture;
}

#endif

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