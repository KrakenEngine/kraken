//
//  KRContext.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-12.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KREngine_KRContext_h
#define KREngine_KRContext_h

#include "KREngine-common.h"
#include "KRBundleManager.h"
#include "KRSceneManager.h"
#include "KRTextureManager.h"
#include "KRMaterialManager.h"
#include "KRShaderManager.h"
#include "KRMeshManager.h"
#include "KRAnimationManager.h"
#include "KRAnimationCurveManager.h"
#include "KRUnknownManager.h"
#include "KRStreamer.h"

class KRAudioManager;

class KRContext {
public:
    static int KRENGINE_MAX_SHADER_HANDLES;
    static int KRENGINE_GPU_MEM_MAX;
    static int KRENGINE_GPU_MEM_TARGET;
    static int KRENGINE_MAX_TEXTURE_DIM;
    static int KRENGINE_MIN_TEXTURE_DIM;
    static int KRENGINE_PRESTREAM_DISTANCE;
    
    
    KRContext();
    ~KRContext();
    
    void loadResource(const std::string &file_name, KRDataBlock *data);
    void loadResource(std::string path);
    
    KRBundleManager *getBundleManager();
    KRSceneManager *getSceneManager();
    KRTextureManager *getTextureManager();
    KRMaterialManager *getMaterialManager();
    KRShaderManager *getShaderManager();
    KRMeshManager *getMeshManager();
    KRAnimationManager *getAnimationManager();
    KRAnimationCurveManager *getAnimationCurveManager();
    KRAudioManager *getAudioManager();
    KRUnknownManager *getUnknownManager();
    
    KRCamera *createCamera(int width, int height);
    
    enum {
        KRENGINE_GL_EXT_texture_storage,
        KRENGINE_NUM_EXTENSIONS
    };
    
    static const char * extension_names[KRENGINE_NUM_EXTENSIONS];
    static bool extension_available[KRENGINE_NUM_EXTENSIONS];
    
    void startFrame(float deltaTime);
    void endFrame(float deltaTime);
    
    long getCurrentFrame() const;
    long getLastFullyStreamedFrame() const;
    float getAbsoluteTime() const;
    
    long getAbsoluteTimeMilliseconds();
    
    std::vector<KRResource *> getResources();
    bool getStreamingEnabled();
    void setStreamingEnabled(bool enable);
    
#if TARGET_OS_IPHONE || TARGET_OS_MAC
    // XXX This doesn't belong here, and might not actually be needed at all
    void getMemoryStats(long &free_memory);
#endif

    typedef enum {
        LOG_LEVEL_INFORMATION,
        LOG_LEVEL_WARNING,
        LOG_LEVEL_ERROR
    } log_level;
    
    typedef void log_callback(void *userdata, const std::string &message, log_level level);
    
    static void SetLogCallback(log_callback *log_callback, void *user_data);
    static void Log(log_level level, const std::string &message_format, ...);
    
    void doStreaming();
    void receivedMemoryWarning();

    static void activateStreamerContext();
    static void activateRenderContext();
    
#if TARGET_OS_MAC
    static void attachToView(void *view);
#endif
    
private:
    KRBundleManager *m_pBundleManager;
    KRSceneManager *m_pSceneManager;
    KRTextureManager *m_pTextureManager;
    KRMaterialManager *m_pMaterialManager;
    KRShaderManager *m_pShaderManager;
    KRMeshManager *m_pMeshManager;
    KRAnimationManager *m_pAnimationManager;
    KRAnimationCurveManager *m_pAnimationCurveManager;
    KRAudioManager *m_pSoundManager;
    KRUnknownManager *m_pUnknownManager;
    
    void detectExtensions();
    bool m_bDetectedExtensions;
    
    long m_current_frame; // TODO - Does this need to be atomic?
    long m_last_memory_warning_frame; // TODO - Does this need to be atomic?
    long m_last_fully_streamed_frame; // TODO - Does this need to be atomic?
    float m_absolute_time;
    
#ifdef __APPLE__
    mach_timebase_info_data_t    m_timebase_info;
#endif

    std::atomic<bool> m_streamingEnabled;
    
    
    static log_callback *s_log_callback;
    static void *s_log_callback_user_data;
    
    KRStreamer m_streamer;
    
    static void createDeviceContexts();
    void destroyDeviceContexts();
};

#endif
