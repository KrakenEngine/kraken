//
//  KRDevice.h
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
#include "KRContextObject.h"

#pragma once

namespace mimir {
class Block;
}

class KRDevice : public KRContextObject
{
public:
  KRDevice(KRContext& context, const VkPhysicalDevice& device);
  virtual ~KRDevice();

  KRDevice(const KRDevice&) = delete;
  KRDevice& operator=(const KRDevice&) = delete;

  void destroy();
  bool initialize(const std::vector<const char*>& deviceExtensions);

#if KRENGINE_DEBUG_GPU_LABELS
  void setDebugLabel(uint64_t objectHandle, VkObjectType objectType, const char* debugLabel);
  void setDebugLabel(const VkImage& image, const char* debugLabel);
  void setDebugLabel(const VkBuffer& buffer, const char* debugLabel);
  void setDebugLabel(const VkQueue& queue, const char* debugLabel);
  void setDebugLabel(const VkCommandBuffer& commandBuffer, const char* debugLabel);
  void setDebugLabel(const VkRenderPass& renderPass, const char* debugLabel);
  void setDebugLabel(const VkDevice& device, const char* debugLabel);
#endif // KRENGINE_DEBUG_GPU_LABELS

  VmaAllocator getAllocator();
  bool createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VmaAllocation* allocation
#if KRENGINE_DEBUG_GPU_LABELS  
    , const char* debug_label
#endif
  );

  KrResult selectSurfaceFormat(VkSurfaceKHR& surface, VkSurfaceFormatKHR& surfaceFormat) const;
  KrResult selectDepthFormat(VkFormat& selectedDepthFormat) const;
  KrResult selectPresentMode(VkSurfaceKHR& surface, VkPresentModeKHR& selectedPresentMode) const;

  void streamStart();
  void streamUpload(mimir::Block& data, VkBuffer destination);
  void streamUpload(mimir::Block& data, VkImage destination, size_t offset, size_t size, hydra::Vector3i dimensions, uint32_t baseMipLevel, uint32_t levelCount);
  void streamUpload(void* data, size_t size, VkBuffer destination);
  void streamUpload(void* data, size_t size, hydra::Vector3i dimensions, VkImage destination);
  void streamEnd();

  void graphicsUpload(VkCommandBuffer& commandBuffer, mimir::Block& data, VkBuffer destination);
  void graphicsUpload(VkCommandBuffer& commandBuffer, void* data, size_t size, VkBuffer destination);

  void createDescriptorSets(const std::vector<VkDescriptorSetLayout>& layouts, std::vector<VkDescriptorSet>& descriptorSets);

  VkPhysicalDevice m_device;
  VkDevice m_logicalDevice;
  VkPhysicalDeviceProperties m_deviceProperties;
  VkPhysicalDeviceFeatures m_deviceFeatures;
  uint32_t m_graphicsFamilyQueueIndex;
  VkQueue m_graphicsQueue;
  uint32_t m_computeFamilyQueueIndex;
  VkQueue m_computeQueue;
  uint32_t m_transferFamilyQueueIndex;
  VkQueue m_transferQueue;
  VkCommandPool m_graphicsCommandPool;
  VkCommandPool m_computeCommandPool;
  VkCommandPool m_transferCommandPool;
  std::vector<VkCommandBuffer> m_graphicsCommandBuffers;
  std::vector<VkCommandBuffer> m_computeCommandBuffers;
  std::vector<VkCommandBuffer> m_transferCommandBuffers;
  VmaAllocator m_allocator;
  VkDescriptorPool m_descriptorPool;

  struct StagingBufferInfo
  {
    VkBuffer buffer;
    VmaAllocation allocation;
    size_t size;
    size_t usage;
    void* data;
    bool started;

    void destroy(VmaAllocator& allocator);
  };

  // Staging buffer for uploading with the transfer queue
  // This will be used for asynchronous asset streaming in the streamer thread.
  // TODO - We should allocate at least two of these and double-buffer for increased CPU-GPU concurrency
  StagingBufferInfo m_streamingStagingBuffer;

  // Staging buffer for uploading with the graphics queue
  // This will be used for uploading assets procedurally generated while recording the graphics command buffer.
  // TODO - We should allocate at least two of these and double-buffer for increased CPU-GPU concurrency
  StagingBufferInfo m_graphicsStagingBuffer;

  void getQueueFamiliesForSharing(uint32_t* queueFamilyIndices, uint32_t* familyCount, VkSharingMode* sharingMode);
private:
  void checkFlushStreamBuffer(size_t size);

  // Initialization helper functions
  bool getAndCheckDeviceCapabilities(const std::vector<const char*>& deviceExtensions);
  bool selectQueueFamilies();
  bool initDeviceAndQueues(const std::vector<const char*>& deviceExtensions);
  bool initCommandPools();
  bool initCommandBuffers();
  bool initAllocator();
  bool initStagingBuffers();
  bool initStagingBuffer(VkDeviceSize size, StagingBufferInfo* info
#if KRENGINE_DEBUG_GPU_LABELS
    , const char* debug_label
#endif // KRENGINE_DEBUG_GPU_LABELS
  );
  bool initDescriptorPool();
  void streamUploadImpl(size_t size, hydra::Vector3i dimensions, VkImage destination, uint32_t baseMipLevel, uint32_t levelCount);
};
