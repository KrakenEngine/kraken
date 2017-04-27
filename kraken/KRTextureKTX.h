//
//  KRTextureKTX.h
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-10-23.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KRTEXTUREKTX_H
#define KRTEXTUREKTX_H

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

#endif
