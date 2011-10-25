//
//  KRMaterialManager.h
//  gldemo
//
//  Created by Kearwood Gilbert on 10-10-24.
//  Copyright (c) 2010 Kearwood Software. All rights reserved.
//

#ifndef KRMATERIALMANAGER_H
#define KRMATERIALMANAGER_H

#include "KRMaterial.h"
#include "KRTextureManager.h"

#include <map>
using std::map;

class KRMaterialManager {
public:
    KRMaterialManager(KRTextureManager *pTextureManager);
    ~KRMaterialManager();
    
    bool loadFile(const char *szPath);
    KRMaterial *getMaterial(const char *szName);
    
private:
    map<std::string, KRMaterial *> m_materials;
    KRTextureManager *m_pTextureManager;
};

#endif

