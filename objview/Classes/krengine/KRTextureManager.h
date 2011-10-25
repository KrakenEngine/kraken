//
//  KRTextureManager.h
//  gldemo
//
//  Created by Kearwood Gilbert on 10-10-14.
//  Copyright (c) 2010 Kearwood Software. All rights reserved.
//

#ifndef KRTEXTUREMANAGER_H
#define KRTEXTUREMANAGER_H

#include "KRTexture.h"

#include <map>
using std::map;

class KRTextureManager {
public:
    KRTextureManager();
    ~KRTextureManager();
    
    KRTexture *loadTexture(const char *szName, const char *szPath);
    GLuint getTextureName(const char *szName);
    KRTexture *getTexture(const char *szFile);
    
private:
    map<std::string, KRTexture *> m_textures;
};

#endif
