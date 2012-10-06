//
//  KRTexture.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-10-05.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KRTexture.h"
#include "KRDataBlock.h"
#include <assert.h>

KRTexture::KRTexture(KRContext &context) : KRContextObject(context)
{
    
    m_iHandle = 0;
    m_textureMemUsed = 0;
}

KRTexture::~KRTexture()
{
    long textureMemFreed = 0;
    releaseHandle(textureMemFreed);
}

void KRTexture::releaseHandle(long &textureMemUsed) {
    if(m_iHandle != 0) {
        textureMemUsed -= getMemSize();
        GLDEBUG(glDeleteTextures(1, &m_iHandle));
        m_iHandle = 0;
        m_textureMemUsed = 0;
    }
}

long KRTexture::getMemSize() {
    return m_textureMemUsed; // TODO - This is not 100% accurate, as loaded format may differ in size while in GPU memory
}

GLuint KRTexture::getHandle(long &textureMemUsed, int max_dim, bool can_resize) {
    // Constrain target LOD to be within mipmap levels of texture
    int target_dim = max_dim;
    if(target_dim < m_min_lod_max_dim) target_dim = m_min_lod_max_dim;
    if(target_dim > m_max_lod_max_dim) target_dim = m_max_lod_max_dim;
    
    if(can_resize && m_current_lod_max_dim != target_dim) {
        releaseHandle(textureMemUsed);
    }
    if(m_iHandle == 0) {
        if(createGLTexture(target_dim, m_textureMemUsed)) {
            textureMemUsed += getMemSize();
        } else {
            assert(false);
        }
    }
    return m_iHandle;
}
