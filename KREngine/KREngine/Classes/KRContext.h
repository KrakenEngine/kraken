//
//  KRContext.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-12.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KREngine_KRContext_h
#define KREngine_KRContext_h

#import "KRSceneManager.h"
#import "KRTextureManager.h"
#import "KRMaterialManager.h"
#import "KRShaderManager.h"
#import "KRModelManager.h"

class KRContext {
public:
    KRContext();
    ~KRContext();
    
    void loadResource(std::string path);
    
    KRSceneManager *getSceneManager();
    KRTextureManager *getTextureManager();
    KRMaterialManager *getMaterialManager();
    KRShaderManager *getShaderManager();
    KRModelManager *getModelManager();
    
private:
    KRSceneManager *m_pSceneManager;
    KRTextureManager *m_pTextureManager;
    KRMaterialManager *m_pMaterialManager;
    KRShaderManager *m_pShaderManager;
    KRModelManager *m_pModelManager;
};

#endif
