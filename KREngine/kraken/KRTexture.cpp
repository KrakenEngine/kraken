//
//  KRTexture.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-10-05.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KREngine-common.h"
#include "KRTexture.h"
#include "KRDataBlock.h"
#include "KRContext.h"
#include "KRTextureManager.h"

KRTexture::KRTexture(KRContext &context, std::string name) : KRResource(context, name)
{
    m_iHandle = 0;
    m_iNewHandle = 0;
    m_textureMemUsed = 0;
    m_newTextureMemUsed = 0;
    m_last_frame_used = 0;
    m_last_frame_bound = 0;
    m_handle_lock.clear();
}

KRTexture::~KRTexture()
{
    releaseHandles();
}

void KRTexture::releaseHandles() {
    long mem_size = getMemSize();
    
    while(m_handle_lock.test_and_set()); // Spin lock
    
    if(m_iNewHandle != 0) {
        GLDEBUG(glDeleteTextures(1, &m_iNewHandle));
        m_iNewHandle = 0;
        m_newTextureMemUsed = 0;
    }
    if(m_iHandle != 0) {
        GLDEBUG(glDeleteTextures(1, &m_iHandle));
        m_iHandle = 0;
        m_textureMemUsed = 0;
    }
    
    m_handle_lock.clear();
    
    getContext().getTextureManager()->memoryChanged(-mem_size);
}

long KRTexture::getMemSize() {
    return m_textureMemUsed + m_newTextureMemUsed; // TODO - This is not 100% accurate, as loaded format may differ in size while in GPU memory
}

long KRTexture::getReferencedMemSize() {
    // Return the amount of memory used by other textures referenced by this texture (for cube maps and animated textures)
    return 0;
}

void KRTexture::resize(int max_dim)
{
    if(!m_handle_lock.test_and_set())
    {
        if(m_iHandle == m_iNewHandle) {
            if(max_dim == 0) {
                m_iNewHandle = 0;
            } else {
                int target_dim = max_dim;
                if(target_dim < m_min_lod_max_dim) target_dim = m_min_lod_max_dim;

                if(m_current_lod_max_dim != target_dim || (m_iHandle == 0 && m_iNewHandle == 0)) {
                    assert(m_newTextureMemUsed == 0);
                    m_newTextureMemUsed = getMemRequiredForSize(target_dim);
                    
                    getContext().getTextureManager()->memoryChanged(m_newTextureMemUsed);
                    getContext().getTextureManager()->addMemoryTransferredThisFrame(m_newTextureMemUsed);
                    
                    if(!createGLTexture(target_dim)) {
                        getContext().getTextureManager()->memoryChanged(-m_newTextureMemUsed);
                        m_newTextureMemUsed = 0;
                        assert(false);  // Failed to create the texture
                    }
                }
            }
        }
        
        m_handle_lock.clear();
    }
}

GLuint KRTexture::getHandle() {
    resetPoolExpiry();
    return m_iHandle;
}

void KRTexture::resetPoolExpiry()
{
    m_last_frame_used = getContext().getCurrentFrame();
}


kraken_stream_level KRTexture::getStreamLevel(bool prime)
{
    if(prime) {
        resetPoolExpiry();
    }
    
    if(m_current_lod_max_dim == 0) {
        return kraken_stream_level::STREAM_LEVEL_OUT;
    } else if(m_current_lod_max_dim == m_max_lod_max_dim) {
        return kraken_stream_level::STREAM_LEVEL_IN_HQ;
    } else {
        return kraken_stream_level::STREAM_LEVEL_IN_LQ;
    }
}

long KRTexture::getLastFrameUsed()
{
    return m_last_frame_used;
}
bool KRTexture::isAnimated()
{
    return false;
}

KRTexture *KRTexture::compress(bool premultiply_alpha)
{
    return NULL;
}

int KRTexture::getCurrentLodMaxDim() {
    return m_current_lod_max_dim;
}

int KRTexture::getMaxMipMap() {
    return m_max_lod_max_dim;
}

int KRTexture::getMinMipMap() {
    return m_min_lod_max_dim;
}

bool KRTexture::hasMipmaps() {
    return m_max_lod_max_dim != m_min_lod_max_dim;
}

void KRTexture::bind(GLuint texture_unit) {
    m_last_frame_bound = getContext().getCurrentFrame();
}

bool KRTexture::canStreamOut() const {
    return (m_last_frame_bound + 2 > getContext().getCurrentFrame());
}

void KRTexture::_swapHandles()
{
    if(!m_handle_lock.test_and_set()) {
        if(m_iHandle != m_iNewHandle) {
            if(m_iHandle != 0) {
                GLDEBUG(glDeleteTextures(1, &m_iHandle));
                getContext().getTextureManager()->memoryChanged(-m_textureMemUsed);
            }
            m_textureMemUsed = (long)m_newTextureMemUsed;
            m_newTextureMemUsed = 0;
            m_iHandle = m_iNewHandle;
        }
        m_handle_lock.clear();
    }
}

