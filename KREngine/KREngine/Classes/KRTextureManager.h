//
//  KRTextureManager.h
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

#define KRENGINE_MAX_TEXTURE_UNITS 8
#define KRENGINE_MAX_TEXTURE_HANDLES 10000
#define KRENGINE_MAX_TEXTURE_MEM 96000000
#define KRENGINE_TARGET_TEXTURE_MEM_MAX 64000000
#define KRENGINE_TARGET_TEXTURE_MEM_MIN 32000000
#define KRENGINE_MAX_TEXTURE_DIM 2048
#define KRENGINE_MIN_TEXTURE_DIM 16

#ifndef KRTEXTUREMANAGER_H
#define KRTEXTUREMANAGER_H

#include "KRTexture.h"
#include "KRContextObject.h"

#include <map>
#import <string>

using std::map;

class KRTextureManager : public KRContextObject {
public:
    KRTextureManager(KRContext &context);
    virtual ~KRTextureManager();
    
    void rotateBuffers(bool new_frame);
    
    void selectTexture(int iTextureUnit, KRTexture *pTexture, int lod_max_dim);
    
#if TARGET_OS_IPHONE
    
    KRTexture *loadTexture(const char *szName, KRDataBlock *data);
    
#endif
    
    KRTexture *getTexture(const char *szFile);
    
    long getMemUsed();
    long getActiveMemUsed();
    
    int getLODDimCap();
    
private:
    void decreaseLODCap();
    void increaseLODCap();
    
    std::map<std::string, KRTexture *> m_textures;
    
    KRTexture *m_boundTextures[KRENGINE_MAX_TEXTURE_UNITS];
    std::set<KRTexture *> m_activeTextures;
    std::set<KRTexture *> m_poolTextures;
    
    long m_textureMemUsed;
    long m_activeTextureMemUsed;
    
    int m_lod_max_dim_cap;
};

#endif
