//
//  KRTextureKTX.h
//  Kraken Engine
//
//  Copyright 2022 Kearwood Gilbert. All rights reserved.
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

#pragma once

#include "KRTexture2D.h"

class KRTextureKTX : public KRTexture2D
{
public:
    KRTextureKTX(KRContext &context, KRDataBlock *data, std::string name);
    KRTextureKTX(KRContext &context, std::string name, GLenum internal_format, GLenum base_internal_format, int width, int height, const std::list<KRDataBlock *> &blocks);
    virtual ~KRTextureKTX();
    virtual std::string getExtension();
    
    bool uploadTexture(GLenum target, int lod_max_dim, int &current_lod_max_dim, bool compress = false, bool premultiply_alpha = false);
    
    virtual long getMemRequiredForSize(int max_dim);
    
protected:
    
    std::list<KRDataBlock *> m_blocks;
    
    typedef struct _KTXHeader
    {
        __uint8_t identifier[12];
        __uint32_t endianness;
        __uint32_t glType;
        __uint32_t glTypeSize;
        __uint32_t glFormat;
        __uint32_t glInternalFormat;
        __uint32_t glBaseInternalFormat;
        __uint32_t pixelWidth;
        __uint32_t pixelHeight;
        __uint32_t pixelDepth;
        __uint32_t numberOfArrayElements;
        __uint32_t numberOfFaces;
        __uint32_t numberOfMipmapLevels;
        __uint32_t bytesOfKeyValueData;
    } KTXHeader;
    
    KTXHeader m_header;
};
