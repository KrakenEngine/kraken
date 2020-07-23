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
#include "KRPipelineManager.h"
#include "KRMeshManager.h"
#include "KRAnimationManager.h"
#include "KRAnimationCurveManager.h"
#include "KRUnknownManager.h"
#include "KRShaderManager.h"
#include "KRSourceManager.h"
#include "KRStreamer.h"

class KRAudioManager;

class KRContext {
public:
    static int KRENGINE_MAX_PIPELINE_HANDLES;
    static int KRENGINE_GPU_MEM_MAX;
    static int KRENGINE_GPU_MEM_TARGET;
    static int KRENGINE_MAX_TEXTURE_DIM;
    static int KRENGINE_MIN_TEXTURE_DIM;
    static int KRENGINE_PRESTREAM_DISTANCE;
    static int KRENGINE_SYS_ALLOCATION_GRANULARITY;
    static int KRENGINE_SYS_PAGE_SIZE;
    
    
    KRContext(const KrInitializeInfo* initializeInfo);
    ~KRContext();

    KrResult createBundle(const KrCreateBundleInfo* createBundleInfo);
    KrResult moveToBundle(const KrMoveToBundleInfo* moveToBundleInfo);
    KrResult loadResource(const KrLoadResourceInfo* loadResourceInfo);
    KrResult unloadResource(const KrUnloadResourceInfo* unloadResourceInfo);
    KrResult mapResource(const KrMapResourceInfo* mapResourceInfo);
    KrResult unmapResource(const KrUnmapResourceInfo* unmapResourceInfo);
    KrResult saveResource(const KrSaveResourceInfo* saveResourceInfo);

    KrResult createScene(const KrCreateSceneInfo* createSceneInfo);
    KrResult findNodeByName(const KrFindNodeByNameInfo* pFindNodeByNameInfo);
    KrResult findAdjacentNodes(const KrFindAdjacentNodesInfo* pFindAdjacentNodesInfo);
    KrResult setNodeLocalTransform(const KrSetNodeLocalTransformInfo* pSetNodeLocalTransform);
    KrResult setNodeWorldTransform(const KrSetNodeWorldTransformInfo* pSetNodeWorldTransform);
    KrResult deleteNode(const KrDeleteNodeInfo* pDeleteNodeInfo);
    KrResult deleteNodeChildren(const KrDeleteNodeChildrenInfo* pDeleteNodeChildrenInfo);
    KrResult appendBeforeNode(const KrAppendBeforeNodeInfo* pAppendBeforeNodeInfo);
    KrResult appendAfterNode(const KrAppendAfterNodeInfo* pAppendAfterNodeInfo);
    KrResult appendFirstChildNode(const KrAppendFirstChildNodeInfo* pAppendFirstChildNodeInfo);
    KrResult appendLastChildNode(const KrAppendLastChildNodeInfo* pAppendLastChildNodeInfo);
    KrResult updateNode(const KrUpdateNodeInfo* pUpdateNodeInfo);


    KRResource* loadResource(const std::string &file_name, KRDataBlock *data);
    
    
    KRBundleManager *getBundleManager();
    KRSceneManager *getSceneManager();
    KRTextureManager *getTextureManager();
    KRMaterialManager *getMaterialManager();
    KRPipelineManager *getPipelineManager();
    KRMeshManager *getMeshManager();
    KRAnimationManager *getAnimationManager();
    KRAnimationCurveManager *getAnimationCurveManager();
    KRAudioManager *getAudioManager();
    KRUnknownManager *getUnknownManager();
    KRShaderManager *getShaderManager();
    KRSourceManager *getSourceManager();
    
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
    static void Log(log_level level, const std::string message_format, ...);
    
    void doStreaming();
    void receivedMemoryWarning();

    static void activateStreamerContext();
    static void activateRenderContext();
    
#if TARGET_OS_MAC
    static void attachToView(void *view);
#endif
    void addResource(KRResource* resource, const std::string& name);
    void removeResource(KRResource* resource);
private:
    KRBundleManager *m_pBundleManager;
    KRSceneManager *m_pSceneManager;
    KRTextureManager *m_pTextureManager;
    KRMaterialManager *m_pMaterialManager;
    KRPipelineManager *m_pPipelineManager;
    KRMeshManager *m_pMeshManager;
    KRAnimationManager *m_pAnimationManager;
    KRAnimationCurveManager *m_pAnimationCurveManager;
    KRAudioManager *m_pSoundManager;
    KRUnknownManager *m_pUnknownManager;
    KRShaderManager *m_pShaderManager;
    KRSourceManager *m_pSourceManager;

    KRResource** m_resourceMap;
    size_t m_resourceMapSize;
    
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
    VkInstance m_vulkanInstance;
    
    void createDeviceContexts();
    void destroyDeviceContexts();

    unordered_multimap<std::string, KRResource*> m_resources;
};

#endif
