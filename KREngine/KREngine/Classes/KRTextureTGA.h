//
//  KRTextureTGA.h
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-10-23.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KRTEXTURETGA_H
#define KRTEXTURETGA_H

#include "KRTexture2D.h"

class KRTextureTGA : public KRTexture2D
{
public:
    KRTextureTGA(KRContext &context, KRDataBlock *data);
    virtual ~KRTextureTGA();
    
    bool uploadTexture(GLenum target, int lod_max_dim, int &current_lod_max_dim, size_t &textureMemUsed);
};

#endif
