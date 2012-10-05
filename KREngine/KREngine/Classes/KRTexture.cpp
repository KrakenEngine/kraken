//
//  KRTexture.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-10-05.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KRTexture.h"

KRTexture::KRTexture(KRDataBlock *data, KRTextureManager *manager) {
    m_pData = data;
    m_pManager = manager;
}

KRTexture::~KRTexture() {
    
}
