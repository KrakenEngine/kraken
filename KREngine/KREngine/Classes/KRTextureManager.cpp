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
#include "KRContext.h"
#include <string.h>

KRTextureManager::KRTextureManager(KRContext &context) : KRContextObject(context) {
    m_textureMemUsed = 0;
    for(int iTexture=0; iTexture<KRENGINE_MAX_TEXTURE_UNITS; iTexture++) {
        m_boundTextures[iTexture] = NULL;
    }
}

KRTextureManager::~KRTextureManager() {
    for(map<std::string, KRTexture *>::iterator itr = m_textures.begin(); itr != m_textures.end(); ++itr){
        delete (*itr).second;
    }
}

#if TARGET_OS_IPHONE

KRTexture *KRTextureManager::loadTexture(const char *szName, KRDataBlock *data) {
    KRTexture *pTexture = new KRTexture(data, this);
    
    std::string lowerName = szName;
    std::transform(lowerName.begin(), lowerName.end(),
                   lowerName.begin(), ::tolower);
    

    
    m_textures[lowerName] = pTexture;
    return pTexture;
}

#endif

KRTexture *KRTextureManager::getTexture(const char *szName) {
    std::string lowerName = szName;
    std::transform(lowerName.begin(), lowerName.end(),
                   lowerName.begin(), ::tolower);
    
    map<std::string, KRTexture *>::iterator itr = m_textures.find(lowerName);
    if(itr == m_textures.end()) {
        // Not found
        //fprintf(stderr, "ERROR: Texture not found: %s\n", szName);
        return NULL;
    } else {
        return (*itr).second;
    }

}

void KRTextureManager::selectTexture(int iTextureUnit, KRTexture *pTexture) {

    if(m_boundTextures[iTextureUnit] != pTexture) {
        GLDEBUG(glActiveTexture(GL_TEXTURE0 + iTextureUnit));
        if(pTexture != NULL) {
            m_poolTextures.erase(pTexture);
            if(m_activeTextures.find(pTexture) == m_activeTextures.end()) {
                m_activeTextures.insert(pTexture);
            }
            GLDEBUG(glBindTexture(GL_TEXTURE_2D, pTexture->getHandle(m_textureMemUsed)));
            // TODO - These texture parameters should be assigned by the material or texture parameters
            GLDEBUG(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f));
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
        } else {
            GLDEBUG(glBindTexture(GL_TEXTURE_2D, 0));
        }
        m_boundTextures[iTextureUnit] = pTexture;
        while(m_activeTextures.size() + m_poolTextures.size() > KRENGINE_MAX_TEXTURE_HANDLES || m_textureMemUsed > KRENGINE_MAX_TEXTURE_MEM) {
            if(m_poolTextures.empty()) {
                m_pContext->rotateBuffers();
            }
            // Keep texture size within limits
            KRTexture *droppedTexture = (*m_poolTextures.begin());
            droppedTexture->releaseHandle(m_textureMemUsed);
            m_poolTextures.erase(droppedTexture);
            //fprintf(stderr, "Texture Swapping...\n");
        } 
    }
    
//    fprintf(stderr, "VBO Mem: %i Kbyte    Texture Mem: %i Kbyte\n", (int)m_pContext->getModelManager()->getMemUsed() / 1024, (int)m_pContext->getTextureManager()->getMemUsed() / 1024);
}

long KRTextureManager::getMemUsed() {
    return m_textureMemUsed;
}


void KRTextureManager::rotateBuffers()
{
    m_poolTextures.insert(m_activeTextures.begin(), m_activeTextures.end());
    m_activeTextures.clear();
    
    for(int iTexture=0; iTexture < KRENGINE_MAX_TEXTURE_UNITS; iTexture++) {
        KRTexture *pBoundTexture = m_boundTextures[iTexture];
        if(pBoundTexture != NULL) {
            m_poolTextures.erase(pBoundTexture);
            if(m_activeTextures.find(pBoundTexture) == m_activeTextures.end()) {
                m_activeTextures.insert(pBoundTexture);
            }
        }
    }
}
