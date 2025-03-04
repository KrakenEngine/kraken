//
//  KRMeshManager.h
//  Kraken Engine
//
//  Copyright 2024 Kearwood Gilbert. All rights reserved.
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

#include "resources/KRResourceManager.h"
#include "KRContextObject.h"
#include "block.h"
#include "nodes/KRNode.h"

class KRContext;
class KRMesh;
enum RenderPassType : uint8_t;

class KRMeshManager : public KRResourceManager
{
public:
  static const int KRENGINE_MAX_VOLUMETRIC_PLANES = 500;
  static const int KRENGINE_MAX_RANDOM_PARTICLES = 150000;

  KRMeshManager(KRContext& context);
  void init();
  virtual ~KRMeshManager();

  virtual KRResource* loadResource(const std::string& name, const std::string& extension, mimir::Block* data) override;
  virtual KRResource* getResource(const std::string& name, const std::string& extension) override;

  void startFrame(float deltaTime);
  void endFrame(float deltaTime);

  KRMesh* loadModel(const char* szName, mimir::Block* pData);
  std::vector<KRMesh*> getModel(const char* szName);
  KRMesh* getMaxLODModel(const char* szName);
  void addModel(KRMesh* model);

  std::vector<std::string> getModelNames();
  unordered_multimap<std::string, KRMesh*>& getModels();

  class KRVBOData
  {

  public:

    typedef enum
    {
      STREAMING,
      // STREAMING data is loaded asynchronously, with transfer queues in the streamer thread.

      CONSTANT,
      // CONSTANT data is loaded asyncrhronously, with transfer queues in the streamer thread, but is not unloaded.

      IMMEDIATE
      // IMMEDIATE data is loaded synchronously, with graphics queues in the presentation threads.
      // IMMEDIATE data is available for use immediately on the same frame it is generated.
    } vbo_type;

    KRVBOData();
    KRVBOData(KRMeshManager* manager, mimir::Block* data, mimir::Block* index_data, int vertex_attrib_flags, bool static_vbo, vbo_type t
#if KRENGINE_DEBUG_GPU_LABELS
        , const char* debug_label
#endif
    );
    void init(KRMeshManager* manager, mimir::Block* data, mimir::Block* index_data, int vertex_attrib_flags, bool static_vbo, vbo_type t
#if KRENGINE_DEBUG_GPU_LABELS
      , const char* debug_label
#endif
    );
    ~KRVBOData();


    mimir::Block* m_data;
    mimir::Block* m_index_data;

    bool isVBOLoaded()
    {
      return m_is_vbo_loaded;
    }
    bool isVBOReady() const
    {
      return m_is_vbo_ready;
    }
    void load();
    void load(VkCommandBuffer& commandBuffer);
    void unload();
    void bind(VkCommandBuffer& commandBuffer);

    // KRMeshManager depends on the address of KRVBOData's being constant
    // after allocation.  This is enforced by deleted copy constructors.
    KRVBOData(const KRVBOData& o) = delete;
    KRVBOData& operator=(const KRVBOData& o) = delete;

    long getSize()
    {
      return (long)m_size;
    }

    void resetPoolExpiry(float lodCoverage);
    long getLastFrameUsed()
    {
      return m_last_frame_used;
    }

    vbo_type getType()
    {
      return m_type;
    }

    float getStreamPriority();

    void _swapHandles();

    VkBuffer& getVertexBuffer();
    VkBuffer& getIndexBuffer();
    uint32_t getVertexAttributes();

  private:
    KRMeshManager* m_manager;
    int m_vertex_attrib_flags;
    long m_size;

    long m_last_frame_used;
    float m_last_frame_max_lod_coverage;
    vbo_type m_type;
    bool m_static_vbo;
    bool m_is_vbo_loaded;
    bool m_is_vbo_ready;

    typedef struct
    {
      KrDeviceHandle device;
      VkBuffer vertex_buffer;
      VmaAllocation vertex_allocation;
      VkBuffer index_buffer;
      VmaAllocation index_allocation;
    } AllocationInfo;

    AllocationInfo m_allocations[KRENGINE_MAX_GPU_COUNT];

#if KRENGINE_DEBUG_GPU_LABELS
    char m_debugLabel[KRENGINE_DEBUG_GPU_LABEL_MAX_LEN];
#endif
  };

  void bindVBO(VkCommandBuffer& commandBuffer, KRVBOData* vbo_data, float lodCoverage);
  long getMemUsed();
  long getMemActive();

  typedef struct
  {
    hydra::Vector3 vertex;
    hydra::Vector2 uva;
  } RandomParticleVertexData;

  typedef struct
  {
    hydra::Vector3 vertex;
  } VolumetricLightingVertexData;

  long getMemoryTransferedThisFrame();

  size_t getActiveVBOCount();

  struct draw_call_info
  {
    RenderPassType pass;
    char object_name[256];
    char material_name[256];
    int vertex_count;
  };

  void log_draw_call(RenderPassType pass, const std::string& object_name, const std::string& material_name, int vertex_count);
  std::vector<draw_call_info> getDrawCalls();



  KRVBOData KRENGINE_VBO_DATA_3D_CUBE_VERTICES;
  KRVBOData KRENGINE_VBO_DATA_2D_SQUARE_VERTICES;
  KRVBOData KRENGINE_VBO_DATA_RANDOM_PARTICLES;
  KRVBOData KRENGINE_VBO_DATA_VOLUMETRIC_LIGHTING;


  void doStreaming(long& memoryRemaining, long& memoryRemainingThisFrame);

private:
  mimir::Block KRENGINE_VBO_3D_CUBE_VERTICES;
  __int32_t KRENGINE_VBO_3D_CUBE_ATTRIBS;
  mimir::Block KRENGINE_VBO_2D_SQUARE_VERTICES;
  __int32_t KRENGINE_VBO_2D_SQUARE_ATTRIBS;

  unordered_multimap<std::string, KRMesh*> m_models; // Multiple models with the same name/key may be inserted, representing multiple LOD levels of the model

  long m_vboMemUsed;
  KRVBOData* m_currentVBO;

  unordered_map<mimir::Block*, KRVBOData*> m_vbosActive;
  std::vector<std::pair<float, KRVBOData*> > m_activeVBOs_streamer;
  std::vector<std::pair<float, KRVBOData*> > m_activeVBOs_streamer_copy;

  mimir::Block m_randomParticleVertexData;
  mimir::Block m_volumetricLightingVertexData;

  long m_memoryTransferredThisFrame;

  std::vector<draw_call_info> m_draw_calls;
  bool m_draw_call_logging_enabled;
  bool m_draw_call_log_used;

  std::mutex m_streamerFenceMutex;
  bool m_streamerComplete;

  void balanceVBOMemory(long& memoryRemaining, long& memoryRemainingThisFrame);

  void primeVBO(KRVBOData* vbo_data);

  void initRandomParticles();
  void initVolumetricLightingVertexes();

};
