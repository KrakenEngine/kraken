//
//  KRTextureCube.cpp
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

#include "KRTextureCube.h"
#include "KRTexture2D.h"
#include "KRContext.h"

KRTextureCube::KRTextureCube(KRContext &context, std::string name) : KRTexture(context, name)
{
    
    m_max_lod_max_dim = 2048;
    m_min_lod_max_dim = 64;
    
    for(int i=0; i<6; i++) {
        m_textures[i] = NULL;
        std::string faceName = getName() + SUFFIXES[i];
        m_textures[i] = (KRTexture2D *)getContext().getTextureManager()->getTexture(faceName);
        if(m_textures[i]) {
            if(m_textures[i]->getMaxMipMap() < m_max_lod_max_dim) m_max_lod_max_dim = m_textures[i]->getMaxMipMap();
            if(m_textures[i]->getMinMipMap() > m_min_lod_max_dim) m_min_lod_max_dim = m_textures[i]->getMinMipMap();
        }
    }
}

KRTextureCube::~KRTextureCube()
{
}

bool KRTextureCube::createGLTexture(int lod_max_dim)
{
    assert(m_iNewHandle == m_iHandle); // Only allow one resize per frame
    
    bool success = true;
    int prev_lod_max_dim = 0;
#if GL_APPLE_copy_texture_levels && GL_EXT_texture_storage
    
    if(m_iHandle != 0) {
        prev_lod_max_dim = m_current_lod_max_dim;
    }
    
    
#endif
    
    m_iNewHandle = 0;
    GLDEBUG(glGenTextures(1, &m_iNewHandle));
    assert(m_iNewHandle != 0);
    
    m_current_lod_max_dim = 0;
    GLDEBUG(glBindTexture(GL_TEXTURE_CUBE_MAP, m_iNewHandle));
    
    bool bMipMaps = false;

    for(int i=0; i<6; i++) {
        std::string faceName = getName() + SUFFIXES[i];
        if(m_textures[i]) {
            if(m_textures[i]->hasMipmaps()) bMipMaps = true;
            m_textures[i]->uploadTexture(TARGETS[i], lod_max_dim, m_current_lod_max_dim, prev_lod_max_dim);
        }
    }
    
    if(bMipMaps) {
        GLDEBUG(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    } else {
        GLDEBUG(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        // GLDEBUG(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
        // GLDEBUG(glGenerateMipmap(GL_TEXTURE_CUBE_MAP));
    }

    
    return success;
}

long KRTextureCube::getMemRequiredForSize(int max_dim)
{
    int target_dim = max_dim;
    if(target_dim < m_min_lod_max_dim) target_dim = m_min_lod_max_dim;
    
    long memoryRequired = 0;
    for(int i=0; i<6; i++) {
        if(m_textures[i]) {
            memoryRequired += m_textures[i]->getMemRequiredForSize(target_dim);
        }
    }
    return memoryRequired;
}


void KRTextureCube::resetPoolExpiry(float lodCoverage, texture_usage_t textureUsage)
{
    KRTexture::resetPoolExpiry(lodCoverage, textureUsage);
    for(int i=0; i<6; i++) {
        if(m_textures[i]) {
            m_textures[i]->resetPoolExpiry(lodCoverage, textureUsage); // Ensure that side of cube maps do not expire from the texture pool prematurely, as they are referenced indirectly
        }
    }
}

void KRTextureCube::bind(GLuint texture_unit)
{
    KRTexture::bind(texture_unit);
    GLuint handle = getHandle();
    GLDEBUG(glBindTexture(GL_TEXTURE_CUBE_MAP, handle));
    if(handle) {
        GLDEBUG(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GLDEBUG(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    }
}

std::string KRTextureCube::getExtension()
{
    return ""; // Cube maps are just references; there are no files to output
}

bool KRTextureCube::save(const std::string &path)
{
    return true; // Cube maps are just references; there are no files to output
}

bool KRTextureCube::save(KRDataBlock &data)
{
    return true; // Cube maps are just references; there are no files to output
}
