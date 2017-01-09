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

#include "KREngine-common.h"
#include "KRTexture2D.h"
#include "KRTextureManager.h"

KRTexture2D::KRTexture2D(KRContext &context, KRDataBlock *data, std::string name) : KRTexture(context, name) {
    m_pData = data;
}                

KRTexture2D::~KRTexture2D() {
    delete m_pData;
}

bool KRTexture2D::createGLTexture(int lod_max_dim) {
    if(m_iHandle != m_iNewHandle) {
        return true;
    }
    
    bool success = true;
    int prev_lod_max_dim = m_new_lod_max_dim;

    
    m_iNewHandle = 0;
    m_new_lod_max_dim = 0;
    GLDEBUG(glGenTextures(1, &m_iNewHandle));
    
    if(m_iNewHandle == 0) {
        success = false;
    } else {
    
        GLDEBUG(glBindTexture(GL_TEXTURE_2D, m_iNewHandle));
        if (hasMipmaps()) {
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
        } else {
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        }

        if(!uploadTexture(GL_TEXTURE_2D, lod_max_dim, m_new_lod_max_dim)) {
            GLDEBUG(glDeleteTextures(1, &m_iNewHandle));
            m_iNewHandle = m_iHandle;
            m_new_lod_max_dim = prev_lod_max_dim;
            success = false;
        }
    }
    
    return success;
}

void KRTexture2D::bind(GLuint texture_unit) {
    KRTexture::bind(texture_unit);
    GLuint handle = getHandle();
    
    if(m_pContext->getTextureManager()->selectTexture(GL_TEXTURE_2D, texture_unit, handle)) {
        if(handle) {
            // TODO - These texture parameters should be assigned by the material or texture parameters
            m_pContext->getTextureManager()->_setWrapModeS(texture_unit, GL_REPEAT);
            m_pContext->getTextureManager()->_setWrapModeT(texture_unit, GL_REPEAT);
        }
    }
}

bool KRTexture2D::save(const std::string& path)
{
    if(m_pData) {
        return m_pData->save(path);
    } else {
        return false;
    }
}

bool KRTexture2D::save(KRDataBlock &data) {
    if(m_pData) {
        data.append(*m_pData);
        return true;
    } else {
        return false;
    }
}
