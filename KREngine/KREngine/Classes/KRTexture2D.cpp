//
//  KRTexture2D.cpp
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

#include "KRTexture2D.h"
#include "KRTextureManager.h"

#import <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#import <stdint.h>
#import <assert.h>

KRTexture2D::KRTexture2D(KRContext &context, KRDataBlock *data) : KRTexture(context) {
    m_current_lod_max_dim = 0;
    m_pData = data;
}

KRTexture2D::~KRTexture2D() {
    delete m_pData;
}

bool KRTexture2D::createGLTexture(int lod_max_dim) {
    m_current_lod_max_dim = 0;
    GLDEBUG(glGenTextures(1, &m_iHandle));
    if(m_iHandle == 0) {
        return false;
    }
    
    GLDEBUG(glBindTexture(GL_TEXTURE_2D, m_iHandle));
	if (hasMipmaps()) {
        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    } else {
        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    }

    if(!uploadTexture(GL_TEXTURE_2D, lod_max_dim, m_current_lod_max_dim, m_textureMemUsed)) {
        GLDEBUG(glDeleteTextures(1, &m_iHandle));
        m_iHandle = 0;
        m_current_lod_max_dim = 0;
        return false;
    }
    
    
    return true;
}

void KRTexture2D::bind() {
    GLuint handle = getHandle();
    
    GLDEBUG(glBindTexture(GL_TEXTURE_2D, handle));
    if(handle) {
        // TODO - These texture parameters should be assigned by the material or texture parameters
        GLDEBUG(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f));
        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    }
}

int KRTexture2D::getMaxMipMap() {
    return m_max_lod_max_dim;
}

int KRTexture2D::getMinMipMap() {
    return m_min_lod_max_dim;
}

bool KRTexture2D::hasMipmaps() {
    return m_max_lod_max_dim != m_min_lod_max_dim;
}