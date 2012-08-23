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
#import "KRNotified.h"

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
    
    KRCamera *createCamera(int width, int height);
    
    void registerNotified(KRNotified *pNotified);
    void unregisterNotified(KRNotified *pNotified);
    
    void notify_sceneGraphCreate(KRNode *pNode);
    void notify_sceneGraphDelete(KRNode *pNode);
    void notify_sceneGraphModify(KRNode *pNode);
    
private:
    KRSceneManager *m_pSceneManager;
    KRTextureManager *m_pTextureManager;
    KRMaterialManager *m_pMaterialManager;
    KRShaderManager *m_pShaderManager;
    KRModelManager *m_pModelManager;
    
    std::set<KRNotified *> m_notifiedObjects;
    std::set<KRNode *> m_allNodes;
};

#endif
