//
//  KRTexture2D.h
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

#import <stdint.h>
#import <list>
#import <string>

#import "KREngine-common.h"
#import "KRDataBlock.h"

using std::list;

#ifndef KRTEXTURE2D_H
#define KRTEXTURE2D_H

#include "KRTexture.h"

class KRTexture2D : public KRTexture {
public:
    KRTexture2D(KRContext &context, KRDataBlock *data);
    ~KRTexture2D();
    
    bool hasMipmaps();
    int getMaxMipMap();
    int getMinMipMap();
    
    bool uploadTexture(GLenum target, int lod_max_dim, int &current_lod_max_dim, size_t &textureMemUsed);
    virtual void bind(size_t &textureMemUsed, int max_dim, bool can_resize);
    
private:
    KRDataBlock *m_pData;
    
    virtual bool createGLTexture(int lod_max_dim);

    
    uint32_t  m_iWidth;
    uint32_t  m_iHeight;
    GLenum    m_internalFormat;
    bool      m_bHasAlpha;
    
    struct    dataBlockStruct {
        void *start;
        uint32_t length;
    };
    
    std::list<dataBlockStruct> m_blocks;
    
    bool load();

};

#endif
