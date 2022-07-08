//
//  KRContext.h
//  Kraken Engine
//
//  Copyright 2022 Kearwood Gilbert. All rights reserved.
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

#pragma once

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
#include "KRSurfaceManager.h"
#include "KRDeviceManager.h"
#include "KRDevice.h"
#include "KRSurface.h"

class KRAudioManager;
class KRPresentationThread;
class KRStreamerThread;
class KRDeviceManager;

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

    KrResult createWindowSurface(const KrCreateWindowSurfaceInfo* createWindowSurfaceInfo);
    KrResult deleteWindowSurface(const KrDeleteWindowSurfaceInfo* deleteWindowSurfaceInfo);

    KrResult createBundle(const KrCreateBundleInfo* createBundleInfo);
    KrResult moveToBundle(const KrMoveToBundleInfo* moveToBundleInfo);
    KrResult loadResource(const KrLoadResourceInfo* loadResourceInfo);
    KrResult unloadResource(const KrUnloadResourceInfo* unloadResourceInfo);
    KrResult getResourceData(const KrGetResourceDataInfo* getResourceDataInfo, KrGetResourceDataCallback callback);
    KrResult mapResource(const KrMapResourceInfo* mapResourceInfo);
    KrResult unmapResource(const KrUnmapResourceInfo* unmapResourceInfo);
    KrResult saveResource(const KrSaveResourceInfo* saveResourceInfo);

    KrResult compileAllShaders(const KrCompileAllShadersInfo* pCompileAllShadersInfo);

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
    KRSurfaceManager* getSurfaceManager();
    KRDeviceManager* getDeviceManager();
    
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

    static std::mutex g_SurfaceInfoMutex;
    static std::mutex g_DeviceInfoMutex;

    void addResource(KRResource* resource, const std::string& name);
    void removeResource(KRResource* resource);
private:
    std::unique_ptr<KRBundleManager> m_pBundleManager;
    std::unique_ptr<KRSceneManager> m_pSceneManager;
    std::unique_ptr<KRTextureManager> m_pTextureManager;
    std::unique_ptr<KRMaterialManager> m_pMaterialManager;
    std::unique_ptr<KRPipelineManager> m_pPipelineManager;
    std::unique_ptr<KRMeshManager> m_pMeshManager;
    std::unique_ptr<KRAnimationManager> m_pAnimationManager;
    std::unique_ptr<KRAnimationCurveManager> m_pAnimationCurveManager;
    std::unique_ptr<KRAudioManager> m_pSoundManager;
    std::unique_ptr<KRUnknownManager> m_pUnknownManager;
    std::unique_ptr<KRShaderManager> m_pShaderManager;
    std::unique_ptr<KRSourceManager> m_pSourceManager;
    std::unique_ptr<KRDeviceManager> m_deviceManager;
    std::unique_ptr<KRSurfaceManager> m_surfaceManager;

    KRResource** m_resourceMap;
    size_t m_resourceMapSize;
    
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
    
    

    unordered_multimap<std::string, KRResource*> m_resources;

    
    std::unique_ptr<KRStreamerThread> m_streamerThread;
    std::unique_ptr<KRPresentationThread> m_presentationThread;

    unordered_map<KrSurfaceMapIndex, KrSurfaceHandle> m_surfaceHandleMap;
};
