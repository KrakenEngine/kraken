//
//  KRMeshManager.cpp
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

#include "KREngine-common.h"

#include "KRMeshManager.h"

#include "KRMesh.h"
#include "KRMeshCube.h"
#include "KRMeshQuad.h"
#include "KRMeshSphere.h"
#include "KRRenderPass.h"

using namespace mimir;

KRMeshManager::KRMeshManager(KRContext& context)
  : KRResourceManager(context)
  , m_currentVBO(NULL)
  , m_vboMemUsed(0)
  , m_memoryTransferredThisFrame(0)
  , m_streamerComplete(true)
  , m_draw_call_logging_enabled(false)
  , m_draw_call_log_used(false)
{

}

void KRMeshManager::init()
{
  addModel(new KRMeshCube(*m_pContext));
  addModel(new KRMeshQuad(*m_pContext));
  addModel(new KRMeshSphere(*m_pContext));

  // ----  Initialize stock models ----
  static const float _KRENGINE_VBO_3D_CUBE_VERTEX_DATA[] = {
      1.0, 1.0, 1.0,
      -1.0, 1.0, 1.0,
      1.0,-1.0, 1.0,
      -1.0,-1.0, 1.0,
      -1.0,-1.0,-1.0,
      -1.0, 1.0, 1.0,
      -1.0, 1.0,-1.0,
      1.0, 1.0, 1.0,
      1.0, 1.0,-1.0,
      1.0,-1.0, 1.0,
      1.0,-1.0,-1.0,
      -1.0,-1.0,-1.0,
      1.0, 1.0,-1.0,
      -1.0, 1.0,-1.0
  };

  KRENGINE_VBO_3D_CUBE_ATTRIBS = (1 << KRMesh::KRENGINE_ATTRIB_VERTEX);
  KRENGINE_VBO_3D_CUBE_VERTICES.expand(sizeof(float) * 3 * 14);
  KRENGINE_VBO_3D_CUBE_VERTICES.lock();
  memcpy(KRENGINE_VBO_3D_CUBE_VERTICES.getStart(), _KRENGINE_VBO_3D_CUBE_VERTEX_DATA, sizeof(float) * 3 * 14);
  KRENGINE_VBO_3D_CUBE_VERTICES.unlock();

  KRENGINE_VBO_DATA_3D_CUBE_VERTICES.init(this, &KRENGINE_VBO_3D_CUBE_VERTICES, nullptr, KRENGINE_VBO_3D_CUBE_ATTRIBS, false, KRVBOData::CONSTANT
#if KRENGINE_DEBUG_GPU_LABELS
    , "Cube Mesh [built-in]"
#endif
  );

  initRandomParticles();
  initVolumetricLightingVertexes();

  static const float _KRENGINE_VBO_2D_SQUARE_VERTEX_DATA[] = {
      -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
      1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
      -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
      1.0f,  1.0f, 0.0f, 1.0f, 1.0f
  };
  KRENGINE_VBO_2D_SQUARE_ATTRIBS = (1 << KRMesh::KRENGINE_ATTRIB_VERTEX) | (1 << KRMesh::KRENGINE_ATTRIB_TEXUVA);
  KRENGINE_VBO_2D_SQUARE_VERTICES.expand(sizeof(float) * 5 * 4);
  KRENGINE_VBO_2D_SQUARE_VERTICES.lock();
  memcpy(KRENGINE_VBO_2D_SQUARE_VERTICES.getStart(), _KRENGINE_VBO_2D_SQUARE_VERTEX_DATA, sizeof(float) * 5 * 4);
  KRENGINE_VBO_2D_SQUARE_VERTICES.unlock();

  KRENGINE_VBO_DATA_2D_SQUARE_VERTICES.init(this, &KRENGINE_VBO_2D_SQUARE_VERTICES, nullptr, KRENGINE_VBO_2D_SQUARE_ATTRIBS, false, KRVBOData::CONSTANT
#if KRENGINE_DEBUG_GPU_LABELS
    , "Square Mesh [built-in]"
#endif
  );

}

KRMeshManager::~KRMeshManager()
{
  for (unordered_multimap<std::string, KRMesh*>::iterator itr = m_models.begin(); itr != m_models.end(); ++itr) {
    delete (*itr).second;
  }
  m_models.clear();
}

KRResource* KRMeshManager::loadResource(const std::string& name, const std::string& extension, Block* data)
{
  if (extension.compare("krmesh") == 0) {
    return loadModel(name.c_str(), data);
  }
  return nullptr;
}
KRResource* KRMeshManager::getResource(const std::string& name, const std::string& extension)
{
  if (extension.compare("krmesh") == 0) {
    std::string lodBaseName;
    int lodCoverage;
    KRMesh::parseName(name, lodBaseName, lodCoverage);
    std::vector<KRMesh*> models = getModel(lodBaseName.c_str());
    for (KRMesh* mesh : models) {
      if (mesh->getLODCoverage() == lodCoverage) {
        return mesh;
      }
    }
  }
  return nullptr;
}

KRMesh* KRMeshManager::loadModel(const char* szName, Block* pData)
{
  KRMesh* pModel = new KRMesh(*m_pContext, szName, pData);
  addModel(pModel);
  return pModel;
}

void KRMeshManager::addModel(KRMesh* model)
{
  std::string lowerName = model->getLODBaseName();
  std::transform(lowerName.begin(), lowerName.end(),
                 lowerName.begin(), ::tolower);

  m_models.insert(std::pair<std::string, KRMesh*>(lowerName, model));
}

KRMesh* KRMeshManager::getMaxLODModel(const char* szName)
{
  std::vector<KRMesh*> models = getModel(szName);
  // models are always in order of highest LOD first
  if (models.size()) {
    return models[0];
  }
  return nullptr;
}

std::vector<KRMesh*> KRMeshManager::getModel(const char* szName)
{
  std::string lowerName = szName;
  std::transform(lowerName.begin(), lowerName.end(),
                 lowerName.begin(), ::tolower);


  std::vector<KRMesh*> matching_models;

  std::pair<unordered_multimap<std::string, KRMesh*>::iterator, unordered_multimap<std::string, KRMesh*>::iterator> range = m_models.equal_range(lowerName);
  for (unordered_multimap<std::string, KRMesh*>::iterator itr_match = range.first; itr_match != range.second; itr_match++) {
    matching_models.push_back(itr_match->second);
  }

  std::sort(matching_models.begin(), matching_models.end(), KRMesh::lod_sort_predicate);

  if (matching_models.size() == 0) {
    KRContext::Log(KRContext::LOG_LEVEL_INFORMATION, "Model not found: %s", lowerName.c_str());
  }

  return matching_models;
}

unordered_multimap<std::string, KRMesh*>& KRMeshManager::getModels()
{
  return m_models;
}

void KRMeshManager::bindVBO(VkCommandBuffer& commandBuffer, KRVBOData* vbo_data, float lodCoverage)
{
  vbo_data->resetPoolExpiry(lodCoverage);

  bool vbo_changed = false;
  if (m_currentVBO == NULL) {
    vbo_changed = true;
  } else if (m_currentVBO->m_data != vbo_data->m_data) {
    vbo_changed = true;
  }

  if (vbo_changed) {

    if (m_vbosActive.find(vbo_data->m_data) != m_vbosActive.end()) {
      m_currentVBO = m_vbosActive[vbo_data->m_data];
    } else {
      m_currentVBO = vbo_data;

      m_vbosActive[vbo_data->m_data] = m_currentVBO;
    }

    m_currentVBO->bind(commandBuffer);
  }
}

void KRMeshManager::startFrame(float deltaTime)
{
  m_memoryTransferredThisFrame = 0;
  if (m_draw_call_log_used) {
    // Only log draw calls on the next frame if the draw call log was used on last frame
    m_draw_call_log_used = false;
    m_draw_call_logging_enabled = true;
  }
  m_draw_calls.clear();

  // TODO - Implement proper double-buffering to reduce copy operations
  m_streamerFenceMutex.lock();

  if (m_streamerComplete) {
    assert(m_activeVBOs_streamer_copy.size() == 0); // The streamer should have emptied this if it really did complete

    const long KRENGINE_VBO_EXPIRY_FRAMES = 1;

    std::set<KRVBOData*> expiredVBOs;
    for (auto itr = m_vbosActive.begin(); itr != m_vbosActive.end(); itr++) {
      KRVBOData* activeVBO = (*itr).second;
      activeVBO->_swapHandles();
      if (activeVBO->getType() == KRVBOData::CONSTANT) {
        // Ensure that CONSTANT data is always loaded
        float priority = std::numeric_limits<float>::max();
        m_activeVBOs_streamer_copy.push_back(std::pair<float, KRVBOData*>(priority, activeVBO));
      } else if (activeVBO->getLastFrameUsed() + KRENGINE_VBO_EXPIRY_FRAMES < getContext().getCurrentFrame()) {
        // Expire VBO's that haven't been used in a long time

        switch (activeVBO->getType()) {
        case KRVBOData::STREAMING:
        case KRVBOData::IMMEDIATE:
          activeVBO->unload();
          break;
        case KRVBOData::CONSTANT:
          // CONSTANT VBO's are not unloaded
          break;
        }

        expiredVBOs.insert(activeVBO);
      } else if (activeVBO->getType() == KRVBOData::STREAMING) {
        float priority = activeVBO->getStreamPriority();
        m_activeVBOs_streamer_copy.push_back(std::pair<float, KRVBOData*>(priority, activeVBO));
      }
    }
    for (std::set<KRVBOData*>::iterator itr = expiredVBOs.begin(); itr != expiredVBOs.end(); itr++) {
      m_vbosActive.erase((*itr)->m_data);
    }

    if (m_activeVBOs_streamer_copy.size() > 0) {
      m_streamerComplete = false;
    }
  }
  m_streamerFenceMutex.unlock();

}

void KRMeshManager::endFrame(float deltaTime)
{
  m_currentVBO = nullptr;
}

void KRMeshManager::doStreaming(long& memoryRemaining, long& memoryRemainingThisFrame)
{

  // TODO - Implement proper double-buffering to reduce copy operations
  m_streamerFenceMutex.lock();
  m_activeVBOs_streamer = std::move(m_activeVBOs_streamer_copy);
  m_streamerFenceMutex.unlock();

  if (m_activeVBOs_streamer.size() > 0) {
    balanceVBOMemory(memoryRemaining, memoryRemainingThisFrame);

    m_streamerFenceMutex.lock();
    m_streamerComplete = true;
    m_streamerFenceMutex.unlock();
  } else {
    memoryRemaining -= getMemUsed();
  }
}

void KRMeshManager::balanceVBOMemory(long& memoryRemaining, long& memoryRemainingThisFrame)
{
  std::sort(m_activeVBOs_streamer.begin(), m_activeVBOs_streamer.end(), std::greater<std::pair<float, KRVBOData*>>());


  for (auto vbo_itr = m_activeVBOs_streamer.begin(); vbo_itr != m_activeVBOs_streamer.end(); vbo_itr++) {
    KRVBOData* vbo_data = (*vbo_itr).second;
    long vbo_size = vbo_data->getSize();
    if (!vbo_data->isVBOLoaded()) {
      if (memoryRemainingThisFrame > vbo_size) {
        vbo_data->load();
        memoryRemainingThisFrame -= vbo_size;
      }
    }
    memoryRemaining -= vbo_size;
  }
}

long KRMeshManager::getMemUsed()
{
  return m_vboMemUsed;
}

long KRMeshManager::getMemActive()
{
  long mem_active = 0;
  for (unordered_map<Block*, KRVBOData*>::iterator itr = m_vbosActive.begin(); itr != m_vbosActive.end(); itr++) {
    mem_active += (*itr).second->getSize();
  }
  return mem_active;
}

void KRMeshManager::initVolumetricLightingVertexes()
{
  if (m_volumetricLightingVertexData.getSize() == 0) {
    m_volumetricLightingVertexData.expand(sizeof(VolumetricLightingVertexData) * KRENGINE_MAX_VOLUMETRIC_PLANES * 6);
    m_volumetricLightingVertexData.lock();
    VolumetricLightingVertexData* vertex_data = (VolumetricLightingVertexData*)m_volumetricLightingVertexData.getStart();
    int iVertex = 0;
    for (int iPlane = 0; iPlane < KRENGINE_MAX_VOLUMETRIC_PLANES; iPlane++) {
      vertex_data[iVertex].vertex.x = -1.0f;
      vertex_data[iVertex].vertex.y = -1.0f;
      vertex_data[iVertex].vertex.z = (float)iPlane;
      iVertex++;

      vertex_data[iVertex].vertex.x = 1.0f;
      vertex_data[iVertex].vertex.y = -1.0f;
      vertex_data[iVertex].vertex.z = (float)iPlane;
      iVertex++;

      vertex_data[iVertex].vertex.x = -1.0f;
      vertex_data[iVertex].vertex.y = 1.0f;
      vertex_data[iVertex].vertex.z = (float)iPlane;
      iVertex++;

      vertex_data[iVertex].vertex.x = -1.0f;
      vertex_data[iVertex].vertex.y = 1.0f;
      vertex_data[iVertex].vertex.z = (float)iPlane;
      iVertex++;

      vertex_data[iVertex].vertex.x = 1.0f;
      vertex_data[iVertex].vertex.y = -1.0f;
      vertex_data[iVertex].vertex.z = (float)iPlane;
      iVertex++;

      vertex_data[iVertex].vertex.x = 1.0f;
      vertex_data[iVertex].vertex.y = 1.0f;
      vertex_data[iVertex].vertex.z = (float)iPlane;
      iVertex++;

    }

    KRENGINE_VBO_DATA_VOLUMETRIC_LIGHTING.init(this, &m_volumetricLightingVertexData, nullptr, (1 << KRMesh::KRENGINE_ATTRIB_VERTEX), false, KRVBOData::CONSTANT
#if KRENGINE_DEBUG_GPU_LABELS
      , "Volumetric Lighting Planes [built-in]"
#endif
    );

    m_volumetricLightingVertexData.unlock();
  }
}

void KRMeshManager::initRandomParticles()
{
  if (m_randomParticleVertexData.getSize() == 0) {
    m_randomParticleVertexData.expand(sizeof(RandomParticleVertexData) * KRENGINE_MAX_RANDOM_PARTICLES * 3);
    m_randomParticleVertexData.lock();
    RandomParticleVertexData* vertex_data = (RandomParticleVertexData*)m_randomParticleVertexData.getStart();

    // Generate vertices for randomly placed equilateral triangles with a side length of 1 and an origin point centered so that an inscribed circle can be efficiently rendered without wasting fill

    float equilateral_triangle_height = sqrt(3.0f) * 0.5f;
    float inscribed_circle_radius = 1.0f / (2.0f * sqrt(3.0f));

    int iVertex = 0;
    for (int iParticle = 0; iParticle < KRENGINE_MAX_RANDOM_PARTICLES; iParticle++) {
      vertex_data[iVertex].vertex.x = (float)(rand() % 2000) / 1000.0f - 1000.0f;
      vertex_data[iVertex].vertex.y = (float)(rand() % 2000) / 1000.0f - 1000.0f;
      vertex_data[iVertex].vertex.z = (float)(rand() % 2000) / 1000.0f - 1000.0f;
      vertex_data[iVertex].uva.x = -0.5f;
      vertex_data[iVertex].uva.y = -inscribed_circle_radius;
      iVertex++;

      vertex_data[iVertex].vertex.x = vertex_data[iVertex - 1].vertex.x;
      vertex_data[iVertex].vertex.y = vertex_data[iVertex - 1].vertex.y;
      vertex_data[iVertex].vertex.z = vertex_data[iVertex - 1].vertex.z;
      vertex_data[iVertex].uva.x = 0.5f;
      vertex_data[iVertex].uva.y = -inscribed_circle_radius;
      iVertex++;

      vertex_data[iVertex].vertex.x = vertex_data[iVertex - 1].vertex.x;
      vertex_data[iVertex].vertex.y = vertex_data[iVertex - 1].vertex.y;
      vertex_data[iVertex].vertex.z = vertex_data[iVertex - 1].vertex.z;
      vertex_data[iVertex].uva.x = 0.0f;
      vertex_data[iVertex].uva.y = -inscribed_circle_radius + equilateral_triangle_height;
      iVertex++;
    }

    KRENGINE_VBO_DATA_RANDOM_PARTICLES.init(this, &m_randomParticleVertexData, nullptr, (1 << KRMesh::KRENGINE_ATTRIB_VERTEX) | (1 << KRMesh::KRENGINE_ATTRIB_TEXUVA), false, KRVBOData::CONSTANT
#if KRENGINE_DEBUG_GPU_LABELS
      , "Random Particles [built-in]"
#endif
    );

    m_randomParticleVertexData.unlock();
  }
}

long KRMeshManager::getMemoryTransferedThisFrame()
{
  return m_memoryTransferredThisFrame;
}


size_t KRMeshManager::getActiveVBOCount()
{
  return m_vbosActive.size();
}

void KRMeshManager::log_draw_call(RenderPassType pass, const std::string& object_name, const std::string& material_name, int vertex_count)
{
  if (m_draw_call_logging_enabled) {
    draw_call_info info;
    info.pass = pass;
    strncpy(info.object_name, object_name.c_str(), 256);
    strncpy(info.material_name, material_name.c_str(), 256);
    info.vertex_count = vertex_count;
    m_draw_calls.push_back(info);
  }
}

std::vector<KRMeshManager::draw_call_info> KRMeshManager::getDrawCalls()
{
  m_draw_call_log_used = true;
  return m_draw_calls;
}

KRMeshManager::KRVBOData::KRVBOData()
{
  m_debugLabel[0] = '\0';
  m_is_vbo_loaded = false;
  m_is_vbo_ready = false;
  m_manager = NULL;
  m_type = STREAMING;
  m_data = NULL;
  m_index_data = NULL;
  m_vertex_attrib_flags = 0;
  m_size = 0;

  m_last_frame_used = 0;
  m_last_frame_max_lod_coverage = 0.0f;

  memset(m_allocations, 0, sizeof(AllocationInfo) * KRENGINE_MAX_GPU_COUNT);
}

KRMeshManager::KRVBOData::KRVBOData(KRMeshManager* manager, Block* data, Block* index_data, int vertex_attrib_flags, bool static_vbo, vbo_type t
#if KRENGINE_DEBUG_GPU_LABELS
  , const char* debug_label
#endif
)
{
  m_debugLabel[0] = '\0';
  memset(m_allocations, 0, sizeof(AllocationInfo) * KRENGINE_MAX_GPU_COUNT);
  m_is_vbo_loaded = false;
  m_is_vbo_ready = false;
  init(manager, data, index_data, vertex_attrib_flags, static_vbo, t
#if KRENGINE_DEBUG_GPU_LABELS
    , debug_label
#endif
  );
}

void KRMeshManager::KRVBOData::init(KRMeshManager* manager, Block* data, Block* index_data, int vertex_attrib_flags, bool static_vbo, vbo_type t
#if KRENGINE_DEBUG_GPU_LABELS
  , const char* debug_label
#endif
)
{
#if KRENGINE_DEBUG_GPU_LABELS
  snprintf(m_debugLabel, KRENGINE_DEBUG_GPU_LABEL_MAX_LEN, debug_label);
#endif //KRENGINE_DEBUG_GPU_LABELS
  m_manager = manager;
  m_type = t;
  m_static_vbo = static_vbo;
  m_data = data;
  m_index_data = index_data;
  m_vertex_attrib_flags = vertex_attrib_flags;

  m_size = m_data->getSize();
  if (m_index_data != NULL) {
    m_size += m_index_data->getSize();
  }

  if (t == KRVBOData::CONSTANT) {
    m_manager->primeVBO(this);
  }
}

KRMeshManager::KRVBOData::~KRVBOData()
{
  // TODO - This needs to be done by the streamer thread, and asserted here...
  unload();
}

void KRMeshManager::KRVBOData::load()
{
  // TODO - This is a bit messy.  Clean up after Vulkan refactor.
  VkCommandBuffer noCommandBuffer = VK_NULL_HANDLE;
  load(noCommandBuffer);
}

void KRMeshManager::KRVBOData::load(VkCommandBuffer& commandBuffer)
{
  // TODO - We should load on each GPU only if there is a surface using the mesh
  if (isVBOLoaded()) {
    return;
  }

  KRDeviceManager* deviceManager = m_manager->getContext().getDeviceManager();
  int iAllocation = 0;

  for (auto deviceItr = deviceManager->getDevices().begin(); deviceItr != deviceManager->getDevices().end() && iAllocation < KRENGINE_MAX_GPU_COUNT; deviceItr++, iAllocation++) {
    KRDevice& device = *(*deviceItr).second;
    KrDeviceHandle deviceHandle = (*deviceItr).first;
    VmaAllocator allocator = device.getAllocator();
    AllocationInfo& allocation = m_allocations[iAllocation];
    allocation.device = deviceHandle;

#if KRENGINE_DEBUG_GPU_LABELS
    char debug_label[KRENGINE_DEBUG_GPU_LABEL_MAX_LEN];

    const char* type_label = "";

    switch (m_type) {
    case vbo_type::STREAMING:
      type_label = "Streaming";
      break;
    case vbo_type::CONSTANT:
      type_label = "Constant";
      break;
    case vbo_type::IMMEDIATE:
      type_label = "Immediate";
      break;
    default:
      assert(false);
    }

    snprintf(debug_label, KRENGINE_DEBUG_GPU_LABEL_MAX_LEN, "%s Vertices: %s", type_label, m_debugLabel);
#endif // KRENGINE_DEBUG_GPU_LABELS

    device.createBuffer(
      m_data->getSize(),
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      &allocation.vertex_buffer,
      &allocation.vertex_allocation
#if KRENGINE_DEBUG_GPU_LABELS
      , debug_label
#endif // KRENGINE_DEBUG_GPU_LABELS
    );
    if (m_type == vbo_type::IMMEDIATE) {
      device.graphicsUpload(commandBuffer, *m_data, allocation.vertex_buffer);
    } else {
      device.streamUpload(*m_data, allocation.vertex_buffer);
    }


    if (m_index_data && m_index_data->getSize() > 0) {
#if KRENGINE_DEBUG_GPU_LABELS
      snprintf(debug_label, KRENGINE_DEBUG_GPU_LABEL_MAX_LEN, "%s Indexes: %s", type_label, m_debugLabel);
#endif // KRENGINE_DEBUG_GPU_LABELS
      device.createBuffer(
        m_index_data->getSize(),
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &allocation.index_buffer,
        &allocation.index_allocation
#if KRENGINE_DEBUG_GPU_LABELS
        , debug_label
#endif
      );
      if (m_type == vbo_type::IMMEDIATE) {
        device.graphicsUpload(commandBuffer, *m_index_data, allocation.index_buffer);
      } else {
        device.streamUpload(*m_index_data, allocation.index_buffer);
      }
    }
  }

  m_is_vbo_loaded = true;

  m_manager->m_vboMemUsed += getSize();
  m_manager->m_memoryTransferredThisFrame += getSize();

  if (m_type != STREAMING) {
    _swapHandles();
  }
}

void KRMeshManager::KRVBOData::unload()
{
  KRDeviceManager* deviceManager = m_manager->getContext().getDeviceManager();
  for (int i = 0; i < KRENGINE_MAX_GPU_COUNT; i++) {
    AllocationInfo& allocation = m_allocations[i];
    if (allocation.device) {
      std::unique_ptr<KRDevice>& device = deviceManager->getDevice(allocation.device);
      if (device) {
        VmaAllocator allocator = device->getAllocator();
        vmaDestroyBuffer(allocator, allocation.vertex_buffer, allocation.vertex_allocation);
        if (allocation.index_buffer) {
          vmaDestroyBuffer(allocator, allocation.index_buffer, allocation.index_allocation);
        }
      }
    }
    memset(&allocation, 0, sizeof(AllocationInfo));
  }

  if (isVBOLoaded()) {
    m_manager->m_vboMemUsed -= getSize();
  }

  m_is_vbo_loaded = false;
  m_is_vbo_ready = false;
}

void KRMeshManager::KRVBOData::bind(VkCommandBuffer& commandBuffer)
{
  VkBuffer vertexBuffers[] = { getVertexBuffer() };
  VkDeviceSize offsets[] = { 0 };
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

  if (m_index_data && m_index_data->getSize() > 0) {
    // TODO - Support 32-bit index buffers
    vkCmdBindIndexBuffer(commandBuffer, getIndexBuffer(), 0, VK_INDEX_TYPE_UINT16);
  }
}

void KRMeshManager::KRVBOData::resetPoolExpiry(float lodCoverage)
{
  long current_frame = m_manager->getContext().getCurrentFrame();
  if (current_frame != m_last_frame_used) {
    m_last_frame_used = current_frame;
    m_last_frame_max_lod_coverage = 0.0f;

    m_manager->primeVBO(this);
  }
  m_last_frame_max_lod_coverage = KRMAX(lodCoverage, m_last_frame_max_lod_coverage);
}


float KRMeshManager::KRVBOData::getStreamPriority()
{
  long current_frame = m_manager->getContext().getCurrentFrame();
  if (current_frame > m_last_frame_used + 5) {
    return 1.0f - KRCLAMP((float)(current_frame - m_last_frame_used) / 60.0f, 0.0f, 1.0f);
  } else {
    return 10000.0f + m_last_frame_max_lod_coverage * 10.0f;
  }
}

void KRMeshManager::KRVBOData::_swapHandles()
{
  m_is_vbo_ready = m_is_vbo_loaded;
}

void KRMeshManager::primeVBO(KRVBOData* vbo_data)
{
  if (m_vbosActive.find(vbo_data->m_data) == m_vbosActive.end()) {
    m_vbosActive[vbo_data->m_data] = vbo_data;
  }
}


VkBuffer& KRMeshManager::KRVBOData::getVertexBuffer()
{
  assert(m_is_vbo_ready);
  return m_allocations->vertex_buffer;
}

VkBuffer& KRMeshManager::KRVBOData::getIndexBuffer()
{
  assert(m_is_vbo_ready);
  return m_allocations->index_buffer;
}


uint32_t KRMeshManager::KRVBOData::getVertexAttributes()
{
  return m_vertex_attrib_flags;
}
