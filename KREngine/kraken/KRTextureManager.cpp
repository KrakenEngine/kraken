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

#include "KREngine-common.h"
#include "KRTextureManager.h"
#include "KRContext.h"
#include "KRTexture2D.h"
#include "KRTexturePVR.h"
#include "KRTextureTGA.h"
#include "KRTextureCube.h"
#include "KRTextureAnimated.h"
#include "KRContext.h"

KRTextureManager::KRTextureManager(KRContext &context) : KRContextObject(context) {
    m_textureMemUsed = 0;

    for(int iTexture=0; iTexture<KRENGINE_MAX_TEXTURE_UNITS; iTexture++) {
        m_boundTextures[iTexture] = NULL;
    }
    m_memoryTransferredThisFrame = 0;
    
    
    _clearGLState();
}

KRTextureManager::~KRTextureManager() {
    for(unordered_map<std::string, KRTexture *>::iterator itr = m_textures.begin(); itr != m_textures.end(); ++itr){
        delete (*itr).second;
    }
}

void KRTextureManager::_clearGLState()
{
    for(int i=0; i < KRENGINE_MAX_TEXTURE_UNITS; i++) {
        m_wrapModeS[i] = 0;
        m_wrapModeT[i] = 0;
        m_maxAnisotropy[i] = -1.0f;
        selectTexture(i, NULL);
    }
    
    m_iActiveTexture = -1;
}

void KRTextureManager::_setActiveTexture(int i)
{
    if(m_iActiveTexture != i) {
        m_iActiveTexture = i;
        GLDEBUG(glActiveTexture(GL_TEXTURE0 + i));
    }
}

void KRTextureManager::_setWrapModeS(GLuint i, GLuint wrap_mode)
{
    if(m_wrapModeS[i] != wrap_mode) {
        _setActiveTexture(i);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_mode);
        m_wrapModeS[i] = wrap_mode;
    }
}

void KRTextureManager::_setMaxAnisotropy(int i, float max_anisotropy)
{
    if(m_maxAnisotropy[i] != max_anisotropy) {
        _setActiveTexture(i);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_anisotropy);
        m_maxAnisotropy[i] = max_anisotropy;
    }
}

void KRTextureManager::setMaxAnisotropy(float max_anisotropy)
{
    for(int i=0; i < KRENGINE_MAX_TEXTURE_UNITS; i++) {
        _setMaxAnisotropy(i, max_anisotropy);
    }
}

void KRTextureManager::_setWrapModeT(GLuint i, GLuint wrap_mode)
{
    if(m_wrapModeT[i] != wrap_mode) {
        _setActiveTexture(i);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_mode);
        m_wrapModeT[i] = wrap_mode;
    }
}

KRTexture *KRTextureManager::loadTexture(const char *szName, const char *szExtension, KRDataBlock *data) {
    KRTexture *pTexture = NULL; 
    
    std::string lowerName = szName;
    std::transform(lowerName.begin(), lowerName.end(),
                   lowerName.begin(), ::tolower);
    
    std::string lowerExtension = szExtension;
    std::transform(lowerExtension.begin(), lowerExtension.end(),
                   lowerExtension.begin(), ::tolower);

    
    if(strcmp(szExtension, "pvr") == 0) {
        pTexture = new KRTexturePVR(getContext(), data, szName);
    } else if(strcmp(szExtension, "tga") == 0) {
        pTexture = new KRTextureTGA(getContext(), data, szName);
    }
    
    if(pTexture) {
        m_textures[lowerName] = pTexture;
    }
    return pTexture;
}

KRTexture *KRTextureManager::getTextureCube(const char *szName) {
    std::string lowerName = szName;
    std::transform(lowerName.begin(), lowerName.end(),
                   lowerName.begin(), ::tolower);
    
    unordered_map<std::string, KRTexture *>::iterator itr = m_textures.find(lowerName);
    if(itr == m_textures.end()) {
        KRTextureCube *pTexture = new KRTextureCube(getContext(), lowerName);
        
        m_textures[lowerName] = pTexture;
        return pTexture;
    } else {
        return (*itr).second;
    }
}

KRTexture *KRTextureManager::getTexture(const std::string &name) {
    
    std::string lowerName = name;
    std::transform(lowerName.begin(), lowerName.end(),
                   lowerName.begin(), ::tolower);
    
    unordered_map<std::string, KRTexture *>::iterator itr = m_textures.find(lowerName);
    if(itr == m_textures.end()) {
        if(lowerName.length() <= 8) {
            return NULL;
        } else if(lowerName.substr(8).compare("animate:") == 0) {
            // This is an animated texture, create KRTextureAnimated's on-demand
            KRTextureAnimated *pTexture = new KRTextureAnimated(getContext(), lowerName);
            m_textures[lowerName] = pTexture;
            return pTexture;
        } else {
            // Not found
            //fprintf(stderr, "ERROR: Texture not found: %s\n", szName);
            return NULL;
        }
    } else {
        return (*itr).second;
    }
}

void KRTextureManager::selectTexture(int iTextureUnit, KRTexture *pTexture) {
    bool is_animated = false;
    if(pTexture) {
        if(pTexture->isAnimated()) is_animated = true;
    }
    
    if(m_boundTextures[iTextureUnit] != pTexture || is_animated) {
        _setActiveTexture(iTextureUnit);
        if(pTexture != NULL) {
            m_poolTextures.erase(pTexture);
            if(m_activeTextures.find(pTexture) == m_activeTextures.end()) {
                m_activeTextures.insert(pTexture);
            }
            pTexture->bind(iTextureUnit);

        } else {
            GLDEBUG(glBindTexture(GL_TEXTURE_2D, 0));
        }
        m_boundTextures[iTextureUnit] = pTexture;
    }

}

long KRTextureManager::getMemUsed() {
    return m_textureMemUsed;
}

long KRTextureManager::getMemActive() {
    long mem_active = 0;
    for(std::set<KRTexture *>::iterator itr=m_activeTextures.begin(); itr != m_activeTextures.end(); itr++) {
        KRTexture *activeTexture = *itr;
        mem_active += activeTexture->getMemSize();
    }
    
    return mem_active;
}

void KRTextureManager::startFrame(float deltaTime)
{
    _clearGLState();
    m_memoryTransferredThisFrame = 0;
    balanceTextureMemory();
    rotateBuffers();
}

void KRTextureManager::endFrame(float deltaTime)
{
    for(int iTexture=0; iTexture < KRENGINE_MAX_TEXTURE_UNITS; iTexture++) {
        if(m_boundTextures[iTexture]) {
            m_boundTextures[iTexture]->resetPoolExpiry(); // Even if the same texture is bound, ensure that they don't expire from the texture pool while in use
        }
    }
}

void KRTextureManager::balanceTextureMemory()
{
    // Balance texture memory by reducing and increasing the maximum mip-map level of both active and inactive textures
    // Favour performance over maximum texture resolution when memory is insufficient for textures at full resolution.
    
    // Determine the additional amount of memory required in order to resize all active textures to the maximum size
    long wantedTextureMem = 0;
    for(std::set<KRTexture *>::iterator itr=m_activeTextures.begin(); itr != m_activeTextures.end(); itr++) {
        KRTexture *activeTexture = *itr;
        
        wantedTextureMem = activeTexture->getMemRequiredForSize(getContext().KRENGINE_MAX_TEXTURE_DIM) - activeTexture->getMemSize();
    }
    
    // Determine how much memory we need to free up
    long memoryDeficit = wantedTextureMem - (getContext().KRENGINE_MAX_TEXTURE_MEM - getMemUsed());
    
    
    // Determine how many mip map levels we need to strip off of inactive textures to free the memory we need
    long maxDimInactive = getContext().KRENGINE_MAX_TEXTURE_DIM;
    long potentialMemorySaving = 0;
    while(potentialMemorySaving < memoryDeficit && maxDimInactive > getContext().KRENGINE_MIN_TEXTURE_DIM) {
        maxDimInactive = maxDimInactive >> 1;
        potentialMemorySaving = 0;
        
        for(std::set<KRTexture *>::iterator itr=m_poolTextures.begin(); itr != m_poolTextures.end(); itr++) {
            KRTexture *poolTexture = *itr;
            long potentialMemoryDelta = poolTexture->getMemRequiredForSize(maxDimInactive) - poolTexture->getMemSize();
            if(potentialMemoryDelta < 0) {
                potentialMemorySaving += -potentialMemoryDelta;
            }
        }
    }
    
    // Strip off mipmap levels of inactive textures to free up memory
    long inactive_texture_mem_used_target = 0;
    for(std::set<KRTexture *>::iterator itr=m_poolTextures.begin(); itr != m_poolTextures.end(); itr++) {
        KRTexture *poolTexture = *itr;
        long potentialMemoryDelta = poolTexture->getMemRequiredForSize(maxDimInactive) - poolTexture->getMemSize();
        if(potentialMemoryDelta < 0) {
            poolTexture->resize(maxDimInactive);
            inactive_texture_mem_used_target += poolTexture->getMemRequiredForSize(maxDimInactive);
        } else {
            inactive_texture_mem_used_target += poolTexture->getMemSize();
        }
    }
    
    // Determine the maximum mipmap level for the active textures we can achieve with the memory that is available
    long memory_available = 0;
    long maxDimActive = getContext().KRENGINE_MAX_TEXTURE_DIM;
    while(memory_available <= 0 && maxDimActive >= getContext().KRENGINE_MIN_TEXTURE_DIM) {
        memory_available = getContext().KRENGINE_MAX_TEXTURE_MEM - inactive_texture_mem_used_target;
        for(std::set<KRTexture *>::iterator itr=m_activeTextures.begin(); itr != m_activeTextures.end() && memory_available > 0; itr++) {
            KRTexture *activeTexture = *itr;
            memory_available -= activeTexture->getMemRequiredForSize(maxDimActive);
        }
        
        if(memory_available <= 0) {
            maxDimActive = maxDimActive >> 1; // Try the next smaller mipmap size
        }
    }
    
    // Resize active textures to balance the memory usage and mipmap levels
    for(std::set<KRTexture *>::iterator itr=m_activeTextures.begin(); itr != m_activeTextures.end() && memory_available > 0; itr++) {
        KRTexture *activeTexture = *itr;
        activeTexture->resize(maxDimActive);
    }
    
    //fprintf(stderr, "Active mipmap size: %i    Inactive mapmap size: %i\n", (int)maxDimActive, (int)maxDimInactive);
}

void KRTextureManager::rotateBuffers()
{
    const long KRENGINE_TEXTURE_EXPIRY_FRAMES = 120;
    
    // ----====---- Expire textures that haven't been used in a long time ----====----
    std::set<KRTexture *> expiredTextures;
    for(std::set<KRTexture *>::iterator itr=m_poolTextures.begin(); itr != m_poolTextures.end(); itr++) {
        KRTexture *poolTexture = *itr;
        if(poolTexture->getLastFrameUsed() + KRENGINE_TEXTURE_EXPIRY_FRAMES < getContext().getCurrentFrame()) {
            expiredTextures.insert(poolTexture);
            poolTexture->releaseHandle();
        }
    }
    for(std::set<KRTexture *>::iterator itr=expiredTextures.begin(); itr != expiredTextures.end(); itr++) {
        m_poolTextures.erase(*itr);
    }
    
    // ----====---- Swap the buffers ----====----
    m_poolTextures.insert(m_activeTextures.begin(), m_activeTextures.end());
    m_activeTextures.clear();
}

long KRTextureManager::getMemoryTransferedThisFrame()
{
    return m_memoryTransferredThisFrame;
}

void KRTextureManager::addMemoryTransferredThisFrame(long memoryTransferred)
{
    m_memoryTransferredThisFrame += memoryTransferred;
}

void KRTextureManager::memoryChanged(long memoryDelta)
{
    m_textureMemUsed += memoryDelta;
}

unordered_map<std::string, KRTexture *> &KRTextureManager::getTextures()
{
    return m_textures;
}

void KRTextureManager::compress()
{
    std::vector<KRTexture *> textures_to_remove;
    std::vector<KRTexture *> textures_to_add;
    
    for(unordered_map<std::string, KRTexture *>::iterator itr=m_textures.begin(); itr != m_textures.end(); itr++) {
        KRTexture *texture = (*itr).second;
        KRTexture *compressed_texture = texture->compress();
        if(compressed_texture) {
            textures_to_remove.push_back(texture);
            textures_to_add.push_back(compressed_texture);
        }
    }
    
    for(std::vector<KRTexture *>::iterator itr = textures_to_remove.begin(); itr != textures_to_remove.end(); itr++) {
        KRTexture *texture = *itr;
        std::string lowerName = texture->getName();
        std::transform(lowerName.begin(), lowerName.end(),
                       lowerName.begin(), ::tolower);
        m_textures.erase(lowerName);
        delete texture;
    }
    
    for(std::vector<KRTexture *>::iterator itr = textures_to_add.begin(); itr != textures_to_add.end(); itr++) {
        KRTexture *texture = *itr;
        std::string lowerName = texture->getName();
        std::transform(lowerName.begin(), lowerName.end(),
                       lowerName.begin(), ::tolower);
        m_textures[lowerName] = texture;
    }
}


std::set<KRTexture *> &KRTextureManager::getActiveTextures()
{
    return m_activeTextures;
}

std::set<KRTexture *> &KRTextureManager::getPoolTextures()
{
    return m_poolTextures;
}

