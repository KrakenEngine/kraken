//
//  KRContext.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-12.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KREngine_KRContext_h
#define KREngine_KRContext_h

#import "KREngine-common.h"
#import "KRBundleManager.h"
#import "KRSceneManager.h"
#import "KRTextureManager.h"
#import "KRMaterialManager.h"
#import "KRShaderManager.h"
#import "KRModelManager.h"
#import "KRAnimationManager.h"
#import "KRAnimationCurveManager.h"
#import "KRUnknownManager.h"

class KRContext {
public:
    static int KRENGINE_MAX_VBO_HANDLES;
    static int KRENGINE_MAX_VBO_MEM;
    static int KRENGINE_MAX_SHADER_HANDLES;
    static int KRENGINE_MAX_TEXTURE_HANDLES;
    static int KRENGINE_MAX_TEXTURE_MEM;
    static int KRENGINE_TARGET_TEXTURE_MEM_MAX;
    static int KRENGINE_TARGET_TEXTURE_MEM_MIN;
    static int KRENGINE_MAX_TEXTURE_DIM;
    static int KRENGINE_MIN_TEXTURE_DIM;
    static int KRENGINE_MAX_TEXTURE_THROUGHPUT;
    
    
    KRContext();
    ~KRContext();
    
    void loadResource(const std::string &file_name, KRDataBlock *data);
    void loadResource(std::string path);
    
    KRBundleManager *getBundleManager();
    KRSceneManager *getSceneManager();
    KRTextureManager *getTextureManager();
    KRMaterialManager *getMaterialManager();
    KRShaderManager *getShaderManager();
    KRModelManager *getModelManager();
    KRAnimationManager *getAnimationManager();
    KRAnimationCurveManager *getAnimationCurveManager();
    KRUnknownManager *getUnknownManager();
    
    KRCamera *createCamera(int width, int height);
    
    void rotateBuffers(bool new_frame);
    
    enum {
        KRENGINE_GL_EXT_texture_storage,
        KRENGINE_NUM_EXTENSIONS
    };
    
    static const char * extension_names[KRENGINE_NUM_EXTENSIONS];
    static bool extension_available[KRENGINE_NUM_EXTENSIONS];
    
    void startFrame(float deltaTime);
    void endFrame(float deltaTime);
    
    long getCurrentFrame() const;
    float getAbsoluteTime() const;
private:
    KRBundleManager *m_pBundleManager;
    KRSceneManager *m_pSceneManager;
    KRTextureManager *m_pTextureManager;
    KRMaterialManager *m_pMaterialManager;
    KRShaderManager *m_pShaderManager;
    KRModelManager *m_pModelManager;
    KRAnimationManager *m_pAnimationManager;
    KRAnimationCurveManager *m_pAnimationCurveManager;
    KRUnknownManager *m_pUnknownManager;
    
    void detectExtensions();
    bool m_bDetectedExtensions;
    
    long m_current_frame;
    float m_absolute_time;
};

#endif
