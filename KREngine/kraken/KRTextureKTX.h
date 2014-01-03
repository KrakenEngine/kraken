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
    
    bool uploadTexture(GLenum target, int lod_max_dim, int &current_lod_max_dim, int prev_lod_max_dim, bool compress = false);
    
    virtual long getMemRequiredForSize(int max_dim);
    
protected:
    
    std::list<KRDataBlock *> m_blocks;
    
    typedef struct _KTXHeader
    {
        Byte identifier[12];
        UInt32 endianness;
        UInt32 glType;
        UInt32 glTypeSize;
        UInt32 glFormat;
        UInt32 glInternalFormat;
        UInt32 glBaseInternalFormat;
        UInt32 pixelWidth;
        UInt32 pixelHeight;
        UInt32 pixelDepth;
        UInt32 numberOfArrayElements;
        UInt32 numberOfFaces;
        UInt32 numberOfMipmapLevels;
        UInt32 bytesOfKeyValueData;
    } KTXHeader;
    
    KTXHeader m_header;
};

#endif
