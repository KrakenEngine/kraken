//
//  KRDevice.cpp
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

#include "KRDevice.h"
#include "KRDeviceManager.h"

using namespace mimir;
using namespace hydra;

KRDevice::KRDevice(KRContext& context, const VkPhysicalDevice& device)
  : KRContextObject(context)
  , m_device(device)
  , m_logicalDevice(VK_NULL_HANDLE)
  , m_deviceProperties{}
  , m_deviceFeatures{}
  , m_graphicsFamilyQueueIndex(0)
  , m_graphicsQueue(VK_NULL_HANDLE)
  , m_computeFamilyQueueIndex(0)
  , m_computeQueue(VK_NULL_HANDLE)
  , m_transferFamilyQueueIndex(0)
  , m_transferQueue(VK_NULL_HANDLE)
  , m_graphicsCommandPool(VK_NULL_HANDLE)
  , m_computeCommandPool(VK_NULL_HANDLE)
  , m_allocator(VK_NULL_HANDLE)
  , m_streamingStagingBuffer{}
  , m_graphicsStagingBuffer{}
  , m_descriptorPool(VK_NULL_HANDLE)
{

}

KRDevice::~KRDevice()
{
  destroy();
}

void KRDevice::StagingBufferInfo::destroy(VmaAllocator& allocator)
{
  if (data) {
    vmaUnmapMemory(allocator, allocation);
    data = nullptr;
  }
  if (buffer) {
    vmaDestroyBuffer(allocator, buffer, allocation);
    size = 0;
    buffer = VK_NULL_HANDLE;
    allocation = VK_NULL_HANDLE;
  }
}

void KRDevice::destroy()
{
  if (m_descriptorPool != VK_NULL_HANDLE) {
    vkDestroyDescriptorPool(m_logicalDevice, m_descriptorPool, nullptr);
    m_descriptorPool = VK_NULL_HANDLE;
  }
  m_streamingStagingBuffer.destroy(m_allocator);
  m_graphicsStagingBuffer.destroy(m_allocator);

  if (m_graphicsCommandPool != VK_NULL_HANDLE) {
    vkDestroyCommandPool(m_logicalDevice, m_graphicsCommandPool, nullptr);
    m_graphicsCommandPool = VK_NULL_HANDLE;
  }

  if (m_computeCommandPool != VK_NULL_HANDLE) {
    vkDestroyCommandPool(m_logicalDevice, m_computeCommandPool, nullptr);
    m_computeCommandPool = VK_NULL_HANDLE;
  }

  if (m_transferCommandPool != VK_NULL_HANDLE) {
    vkDestroyCommandPool(m_logicalDevice, m_transferCommandPool, nullptr);
    m_transferCommandPool = VK_NULL_HANDLE;
  }

  if (m_logicalDevice != VK_NULL_HANDLE) {
    vkDestroyDevice(m_logicalDevice, nullptr);
    m_logicalDevice = VK_NULL_HANDLE;
  }

  if (m_allocator != VK_NULL_HANDLE) {
    vmaDestroyAllocator(m_allocator);
    m_allocator = VK_NULL_HANDLE;
  }
}

bool KRDevice::getAndCheckDeviceCapabilities(const std::vector<const char*>& deviceExtensions)
{

  vkGetPhysicalDeviceProperties(m_device, &m_deviceProperties);
  vkGetPhysicalDeviceFeatures(m_device, &m_deviceFeatures);
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(m_device, nullptr, &extensionCount, nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(m_device, nullptr, &extensionCount, availableExtensions.data());

  std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

  for (const auto& extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }
  if (!requiredExtensions.empty()) {
    // Missing a required extension
    return false;
  }
  if (!m_deviceFeatures.samplerAnisotropy) {
    // Anisotropy feature required
    return false;
  }
  return true;
}

bool KRDevice::selectQueueFamilies()
{
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(m_device, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(m_device, &queueFamilyCount, queueFamilies.data());

  uint32_t graphicsFamilyQueue = -1;
  uint32_t computeFamilyQueue = -1;
  uint32_t transferFamilyQueue = -1;

  // First, select the transfer queue
  for (int i = 0; i < queueFamilies.size(); i++) {
    const VkQueueFamilyProperties& queueFamily = queueFamilies[i];
    if ((queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) == 0) {
      // This queue does not support transfers.  Skip it.
      continue;
    }
    if (transferFamilyQueue == -1) {
      // If we don't already have a transfer queue, take anything that supports VK_QUEUE_TRANSFER_BIT
      transferFamilyQueue = i;
      continue;
    }

    VkQueueFlags priorFlags = queueFamilies[transferFamilyQueue].queueFlags;
    if ((priorFlags & VK_QUEUE_GRAPHICS_BIT) > (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
      // This is a better queue, as it is specifically for transfers and not graphics
      transferFamilyQueue = i;
      continue;
    }
    if ((priorFlags & VK_QUEUE_COMPUTE_BIT) > (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
      // This is a better queue, as it is specifically for transfers and not graphics
      transferFamilyQueue = i;
      continue;
    }
  }

  // Second, select the compute transfer queue
  for (int i = 0; i < queueFamilies.size(); i++) {
    const VkQueueFamilyProperties& queueFamily = queueFamilies[i];
    if ((queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) == 0) {
      // This queue does not support compute.  Skip it.
      continue;
    }
    if (computeFamilyQueue == -1) {
      // If we don't already have a compute queue, take anything that supports VK_QUEUE_COMPUTE_BIT
      computeFamilyQueue = i;
      continue;
    }
    if (computeFamilyQueue == transferFamilyQueue) {
      // Avoid sharing a compute queue with the asset streaming
      computeFamilyQueue = i;
      continue;
    }
    VkQueueFlags priorFlags = queueFamilies[computeFamilyQueue].queueFlags;
    if ((priorFlags & VK_QUEUE_GRAPHICS_BIT) > (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
      // This is a better queue, as it is specifically for compute and not graphics
      computeFamilyQueue = i;
      continue;
    }
  }

  for (int i = 0; i < queueFamilies.size(); i++) {
    const VkQueueFamilyProperties& queueFamily = queueFamilies[i];
    if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) {
      // This queue does not support graphics.  Skip it.
      continue;
    }
    if (graphicsFamilyQueue == -1) {
      // If we don't already have a graphics queue, take anything that supports VK_QUEUE_GRAPHICS_BIT
      graphicsFamilyQueue = i;
      continue;
    }
    if (graphicsFamilyQueue == transferFamilyQueue) {
      // Avoid sharing a graphics queue with the asset streaming
      graphicsFamilyQueue = i;
      continue;
    }
    if (graphicsFamilyQueue == computeFamilyQueue) {
      // Avoid sharing a graphics queue with compute
      graphicsFamilyQueue = i;
      continue;
    }
  }
  if (graphicsFamilyQueue == -1) {
    // No graphics queue family, not suitable
    return false;
  }

  if (computeFamilyQueue == -1) {
    // No compute queue family, not suitable
    return false;
  }

  if (transferFamilyQueue == -1) {
    // No transfer queue family, not suitable
    return false;
  }

  m_graphicsFamilyQueueIndex = graphicsFamilyQueue;
  m_computeFamilyQueueIndex = computeFamilyQueue;
  m_transferFamilyQueueIndex = transferFamilyQueue;

  return true;
}

bool KRDevice::initDeviceAndQueues(const std::vector<const char*>& deviceExtensions)
{
  VkDeviceQueueCreateInfo queueCreateInfo[3]{};
  int queueCount = 1;
  float queuePriority = 1.0f;
  queueCreateInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueCreateInfo[0].queueFamilyIndex = m_graphicsFamilyQueueIndex;
  queueCreateInfo[0].queueCount = 1;
  queueCreateInfo[0].pQueuePriorities = &queuePriority;
  if (m_graphicsFamilyQueueIndex != m_computeFamilyQueueIndex) {
    queueCount++;
    queueCreateInfo[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo[1].queueFamilyIndex = m_computeFamilyQueueIndex;
    queueCreateInfo[1].queueCount = 1;
    queueCreateInfo[1].pQueuePriorities = &queuePriority;
  }
  if (m_transferFamilyQueueIndex != m_graphicsFamilyQueueIndex && m_transferFamilyQueueIndex != m_computeFamilyQueueIndex) {
    queueCount++;
    queueCreateInfo[2].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo[2].queueFamilyIndex = m_transferFamilyQueueIndex;
    queueCreateInfo[2].queueCount = 1;
    queueCreateInfo[2].pQueuePriorities = &queuePriority;
  }

  VkDeviceCreateInfo deviceCreateInfo{};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.pQueueCreateInfos = queueCreateInfo;
  deviceCreateInfo.queueCreateInfoCount = queueCount;
  VkPhysicalDeviceFeatures deviceFeatures{};
  deviceFeatures.samplerAnisotropy = VK_TRUE;
  deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
  deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
  deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
  if (vkCreateDevice(m_device, &deviceCreateInfo, nullptr, &m_logicalDevice) != VK_SUCCESS) {
    // TODO - Log a warning...
    return false;
  }
  vkGetDeviceQueue(m_logicalDevice, m_graphicsFamilyQueueIndex, 0, &m_graphicsQueue);
  vkGetDeviceQueue(m_logicalDevice, m_computeFamilyQueueIndex, 0, &m_computeQueue);
  vkGetDeviceQueue(m_logicalDevice, m_transferFamilyQueueIndex, 0, &m_transferQueue);
#if KRENGINE_DEBUG_GPU_LABELS
  setDebugLabel(m_graphicsQueue, "Graphics");
  setDebugLabel(m_computeQueue, "Compute");
  setDebugLabel(m_transferQueue, "Transfer");
#endif // KRENGINE_DEBUG_GPU_LABELS

  return true;
}

bool KRDevice::initCommandPools()
{
  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.queueFamilyIndex = m_graphicsFamilyQueueIndex;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  if (vkCreateCommandPool(m_logicalDevice, &poolInfo, nullptr, &m_graphicsCommandPool) != VK_SUCCESS) {
    return false;
  }

  poolInfo.queueFamilyIndex = m_computeFamilyQueueIndex;
  if (vkCreateCommandPool(m_logicalDevice, &poolInfo, nullptr, &m_computeCommandPool) != VK_SUCCESS) {
    return false;
  }

  poolInfo.queueFamilyIndex = m_transferFamilyQueueIndex;
  if (vkCreateCommandPool(m_logicalDevice, &poolInfo, nullptr, &m_transferCommandPool) != VK_SUCCESS) {
    return false;
  }
  return true;
}

bool KRDevice::initCommandBuffers()
{
  const int kMaxGraphicsCommandBuffers = KRENGINE_MAX_FRAMES_IN_FLIGHT;
  m_graphicsCommandBuffers.resize(kMaxGraphicsCommandBuffers);

  const int kMaxComputeCommandBuffers = 4; // TODO - This needs to be dynamic?
  m_computeCommandBuffers.resize(kMaxComputeCommandBuffers);

  const int kMaxTransferCommandBuffers = 4; // TODO - This needs to be dynamic?
  m_transferCommandBuffers.resize(kMaxTransferCommandBuffers);

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = m_graphicsCommandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = (uint32_t)m_graphicsCommandBuffers.size();

  if (vkAllocateCommandBuffers(m_logicalDevice, &allocInfo, m_graphicsCommandBuffers.data()) != VK_SUCCESS) {
    return false;
  }

  allocInfo.commandPool = m_computeCommandPool;
  allocInfo.commandBufferCount = (uint32_t)m_computeCommandBuffers.size();
  if (vkAllocateCommandBuffers(m_logicalDevice, &allocInfo, m_computeCommandBuffers.data()) != VK_SUCCESS) {
    return false;
  }

  allocInfo.commandPool = m_transferCommandPool;
  allocInfo.commandBufferCount = (uint32_t)m_transferCommandBuffers.size();
  if (vkAllocateCommandBuffers(m_logicalDevice, &allocInfo, m_transferCommandBuffers.data()) != VK_SUCCESS) {
    return false;
  }

#if KRENGINE_DEBUG_GPU_LABELS
  const size_t kMaxLabelSize = 64;
  char debug_label[kMaxLabelSize];
  for (int i = 0; i < m_transferCommandBuffers.size(); i++) {
    snprintf(debug_label, kMaxLabelSize, "Transfer %i", i);
    setDebugLabel(m_transferCommandBuffers[i], debug_label);
  }
  for (int i = 0; i < m_graphicsCommandBuffers.size(); i++) {
    snprintf(debug_label, kMaxLabelSize, "Presentation %i", i);
    setDebugLabel(m_graphicsCommandBuffers[i], debug_label);
  }
  for (int i = 0; i < m_computeCommandBuffers.size(); i++) {
    snprintf(debug_label, kMaxLabelSize, "Compute %i", i);
    setDebugLabel(m_computeCommandBuffers[i], debug_label);
  }
#endif

  return true;
}

bool KRDevice::initAllocator()
{
  // Create Vulkan Memory Allocator instance for this device

  // We are dynamically linking Vulkan, so we need to give VMA some hints
  // on finding the function pointers
  VmaVulkanFunctions vmaVulkanFunctions{};
  vmaVulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
  vmaVulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

  VmaAllocatorCreateInfo vmaCreateInfo{};
  vmaCreateInfo.flags = VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT;
  // TODO - Hook vmaCreateInfo.pAllocationCallbacks;

  vmaCreateInfo.physicalDevice = m_device;
  vmaCreateInfo.device = m_logicalDevice;
  vmaCreateInfo.instance = m_pContext->getDeviceManager()->getVulkanInstance();
  vmaCreateInfo.vulkanApiVersion = VK_API_VERSION_1_2;
  vmaCreateInfo.pVulkanFunctions = &vmaVulkanFunctions;
  if (vmaCreateAllocator(&vmaCreateInfo, &m_allocator) != VK_SUCCESS) {
    return false;
  }
  return true;
}

bool KRDevice::initStagingBuffers()
{
  // Create Staging Buffer for the transfer queue.
  // This will be used for asynchronous asset streaming in the streamer thread.
  // Start with a 256MB staging buffer.
  // TODO - Dynamically size staging buffer using heuristics
  size_t size = size_t(256) * 1024 * 1024;
  if (!initStagingBuffer(size, &m_streamingStagingBuffer
#if KRENGINE_DEBUG_GPU_LABELS
    , "Streaming Staging Buffer"
#endif // KRENGINE_DEBUG_GPU_LABELS
    )) {
    return false;
  }

  // Create Staging Buffer for the graphics queue.
  // This will be used for uploading assets procedurally generated while recording the graphics command buffer.
  // Start with a 256MB staging buffer.
  // TODO - Dynamically size staging buffer using heuristics
  size = size_t(256) * 1024 * 1024;
  if (!initStagingBuffer(size,
    &m_graphicsStagingBuffer
#if KRENGINE_DEBUG_GPU_LABELS
    , "Graphics Staging Buffer"
#endif // KRENGINE_DEBUG_GPU_LABELS
    )) {
    return false;
  }
  return true;
}

bool KRDevice::initStagingBuffer(VkDeviceSize size, StagingBufferInfo* info
#if KRENGINE_DEBUG_GPU_LABELS
  , const char* debug_label
#endif // KRENGINE_DEBUG_GPU_LABELS
)
{
  if (!createBuffer(
    size,
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
    &info->buffer,
    &info->allocation
#if KRENGINE_DEBUG_GPU_LABELS
    , debug_label
#endif // KRENGINE_DEBUG_GPU_LABELS
    )) {
    return false;
  }
  if (vmaMapMemory(m_allocator, info->allocation, &info->data) != VK_SUCCESS) {
    return false;
  }
  info->size = size;
  return true;
}

bool KRDevice::initDescriptorPool()
{
  // TODO - Vulkan Refactoring - These values need to be dynamic
  // TODO - Perhaps we should dynamically creaate new pools as needed
  const size_t kMaxDescriptorSets = 64;
  const size_t kMaxUniformBufferDescriptors = 1024;
  const size_t kMaxImageSamplerDescriptors = 1024;

  VkDescriptorPoolSize poolSizes[2] = {};
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[0].descriptorCount = static_cast<uint32_t>(kMaxUniformBufferDescriptors);

  poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSizes[1].descriptorCount = static_cast<uint32_t>(kMaxImageSamplerDescriptors);

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = 2;
  poolInfo.pPoolSizes = poolSizes;
  poolInfo.maxSets = static_cast<uint32_t>(kMaxDescriptorSets);

  if (vkCreateDescriptorPool(m_logicalDevice, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
    return false;
  }
  return true;
}

void KRDevice::createDescriptorSets(const std::vector<VkDescriptorSetLayout>& layouts, std::vector<VkDescriptorSet>& descriptorSets)
{
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = m_descriptorPool;
  allocInfo.descriptorSetCount = descriptorSets.size();
  allocInfo.pSetLayouts = layouts.data();
  if (vkAllocateDescriptorSets(m_logicalDevice, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
    // TODO - Vulkan Refactoring - Error Handling
    // In event of failure, should allocate an additional descriptor pool and try again
    assert(false);
  }
}

bool KRDevice::initialize(const std::vector<const char*>& deviceExtensions)
{
  // TODO - Return discrete failure codes
  if (!getAndCheckDeviceCapabilities(deviceExtensions)) {
    return false;
  }

  if (!selectQueueFamilies()) {
    return false;
  }

  if (!initDeviceAndQueues(deviceExtensions)) {
    return false;
  }

  if (!initCommandPools()) {
    destroy();
    return false;
  }

  if (!initCommandBuffers()) {
    destroy();
    return false;
  }

  if (!initAllocator()) {
    destroy();
    return false;
  }

  if (!initStagingBuffers()) {
    destroy();
    return false;
  }

  if (!initDescriptorPool()) {
    destroy();
    return false;
  }

  return true;
}

VmaAllocator KRDevice::getAllocator()
{
  assert(m_allocator != VK_NULL_HANDLE);
  return m_allocator;
}

void KRDevice::getQueueFamiliesForSharing(uint32_t* queueFamilyIndices, uint32_t* familyCount, VkSharingMode* sharingMode)
{
  *familyCount = 1;
  queueFamilyIndices[0] = m_graphicsFamilyQueueIndex;
  if (m_graphicsFamilyQueueIndex != m_transferFamilyQueueIndex) {
    queueFamilyIndices[1] = m_transferFamilyQueueIndex;
    (*familyCount)++;
  }
  if (*familyCount > 1) {
    *sharingMode = VK_SHARING_MODE_CONCURRENT;
  } else {
    *sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }
}

#if KRENGINE_DEBUG_GPU_LABELS
void KRDevice::setDebugLabel(uint64_t objectHandle, VkObjectType objectType, const char* debugLabel)
{
  VkDebugUtilsObjectNameInfoEXT debugInfo{};
  debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
  debugInfo.objectHandle = objectHandle;
  debugInfo.objectType = objectType;
  debugInfo.pObjectName = debugLabel;
  VkResult res = vkSetDebugUtilsObjectNameEXT(m_logicalDevice, &debugInfo);
  assert(res == VK_SUCCESS);
}

void KRDevice::setDebugLabel(const VkImage& image, const char* debugLabel)
{
  setDebugLabel((uint64_t)image, VK_OBJECT_TYPE_IMAGE, debugLabel);
}
void KRDevice::setDebugLabel(const VkBuffer& buffer, const char* debugLabel)
{
  setDebugLabel((uint64_t)buffer, VK_OBJECT_TYPE_BUFFER, debugLabel);
}

void KRDevice::setDebugLabel(const VkQueue& queue, const char* debugLabel)
{
  setDebugLabel((uint64_t)queue, VK_OBJECT_TYPE_QUEUE, debugLabel);
}

void KRDevice::setDebugLabel(const VkCommandBuffer& commandBuffer, const char* debugLabel)
{
  setDebugLabel((uint64_t)commandBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER, debugLabel);
}

void KRDevice::setDebugLabel(const VkRenderPass& renderPass, const char* debugLabel)
{
  setDebugLabel((uint64_t)renderPass, VK_OBJECT_TYPE_RENDER_PASS, debugLabel);
}

void KRDevice::setDebugLabel(const VkDevice& device, const char* debugLabel)
{
  setDebugLabel((uint64_t)device, VK_OBJECT_TYPE_DEVICE, debugLabel);
}

#endif // KRENGINE_DEBUG_GPU_LABELS

bool KRDevice::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VmaAllocation* allocation
#if KRENGINE_DEBUG_GPU_LABELS  
  , const char* debug_label
#endif
)
{
  VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
  bufferInfo.size = size;
  bufferInfo.usage = usage;

  uint32_t queueFamilyIndices[2] = {};
  bufferInfo.pQueueFamilyIndices = queueFamilyIndices;
  bufferInfo.queueFamilyIndexCount = 0;
  getQueueFamiliesForSharing(queueFamilyIndices, &bufferInfo.queueFamilyIndexCount, &bufferInfo.sharingMode);

  VmaAllocationCreateInfo allocInfo = {};
  allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
  allocInfo.requiredFlags = properties;

  VkResult res = vmaCreateBuffer(m_allocator, &bufferInfo, &allocInfo, buffer, allocation, nullptr);
  if (res != VK_SUCCESS) {
    return false;
  }

#if KRENGINE_DEBUG_GPU_LABELS
  setDebugLabel(*buffer, debug_label);
#endif

  return true;
}

KrResult KRDevice::selectSurfaceFormat(VkSurfaceKHR& surface, VkSurfaceFormatKHR& selectedFormat) const
{

  std::vector<VkSurfaceFormatKHR> surfaceFormats;
  uint32_t formatCount = 0;
  if (vkGetPhysicalDeviceSurfaceFormatsKHR(m_device, surface, &formatCount, nullptr) != VK_SUCCESS) {
    return KR_ERROR_VULKAN_SWAP_CHAIN;
  }

  if (formatCount != 0) {
    surfaceFormats.resize(formatCount);
    if (vkGetPhysicalDeviceSurfaceFormatsKHR(m_device, surface, &formatCount, surfaceFormats.data()) != VK_SUCCESS) {
      return KR_ERROR_VULKAN_SWAP_CHAIN;
    }
  }

  selectedFormat = surfaceFormats[0];
  for (const auto& availableFormat : surfaceFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      selectedFormat = availableFormat;
      break;
    }
  }
  return KR_SUCCESS;
}

KrResult KRDevice::selectDepthFormat(VkFormat& selectedDepthFormat) const
{
  selectedDepthFormat = VK_FORMAT_UNDEFINED;
  VkFormatFeatureFlags requiredFeatures = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
  std::vector<VkFormat> candidateFormats;
  candidateFormats.push_back(VK_FORMAT_D32_SFLOAT_S8_UINT);
  candidateFormats.push_back(VK_FORMAT_D24_UNORM_S8_UINT);
  for (VkFormat format : candidateFormats) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(m_device, format, &props);

    if ((props.optimalTilingFeatures & requiredFeatures) == requiredFeatures) {
      selectedDepthFormat = format;
      break;
    }
  }

  if (selectedDepthFormat == VK_FORMAT_UNDEFINED) {
    return KR_ERROR_VULKAN_DEPTHBUFFER;
  }

  return KR_SUCCESS;
}

KrResult KRDevice::selectPresentMode(VkSurfaceKHR& surface, VkPresentModeKHR& selectedPresentMode) const
{
  // VK_PRESENT_MODE_FIFO_KHR is always available
  selectedPresentMode = VK_PRESENT_MODE_FIFO_KHR;

  std::vector<VkPresentModeKHR> surfacePresentModes;

  uint32_t presentModeCount = 0;
  if (vkGetPhysicalDeviceSurfacePresentModesKHR(m_device, surface, &presentModeCount, nullptr) != VK_SUCCESS) {
    return KR_ERROR_VULKAN_SWAP_CHAIN;
  }

  if (presentModeCount != 0) {
    surfacePresentModes.resize(presentModeCount);
    if (vkGetPhysicalDeviceSurfacePresentModesKHR(m_device, surface, &presentModeCount, surfacePresentModes.data()) != VK_SUCCESS) {
      return KR_ERROR_VULKAN_SWAP_CHAIN;
    }
  }

  // Try to find a better mode
  for (const auto& availablePresentMode : surfacePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      selectedPresentMode = availablePresentMode;
    }
  }
  return KR_SUCCESS;
}

void KRDevice::streamStart()
{
  if (!m_streamingStagingBuffer.started) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(m_transferCommandBuffers[0], &beginInfo);

    m_streamingStagingBuffer.started = true;
  }
}

void KRDevice::streamUpload(Block& data, VkBuffer destination)
{
  data.lock();
  streamUpload(data.getStart(), data.getSize(), destination);
  data.unlock();
}

void KRDevice::graphicsUpload(VkCommandBuffer& commandBuffer, Block& data, VkBuffer destination)
{
  data.lock();
  graphicsUpload(commandBuffer, data.getStart(), data.getSize(), destination);
  data.unlock();
}

void KRDevice::checkFlushStreamBuffer(size_t size)
{
  // Flush the buffers if we would run out of space
  if (m_streamingStagingBuffer.usage + size > m_streamingStagingBuffer.size) {
    // If we hit this often, then we need a larger staging buffer.
    // TODO - Dynamically allocate more/larger staging buffers.
    assert(size < m_streamingStagingBuffer.size);
    streamEnd();
    streamStart();
  }
}

void KRDevice::streamUpload(void* data, size_t size, VkBuffer destination)
{
  checkFlushStreamBuffer(size);
  memcpy((uint8_t*)m_streamingStagingBuffer.data + m_streamingStagingBuffer.usage, data, size);

  // TODO - Beneficial to batch many regions in a single call?
  VkBufferCopy copyRegion{};
  copyRegion.srcOffset = m_streamingStagingBuffer.usage;
  copyRegion.dstOffset = 0; // Optional
  copyRegion.size = size;
  vkCmdCopyBuffer(m_transferCommandBuffers[0], m_streamingStagingBuffer.buffer, destination, 1, &copyRegion);

  // TODO - Assert on any needed alignment?
  m_streamingStagingBuffer.usage += size;
}

void KRDevice::graphicsUpload(VkCommandBuffer& commandBuffer, void* data, size_t size, VkBuffer destination)
{
  memcpy((uint8_t*)m_graphicsStagingBuffer.data + m_graphicsStagingBuffer.usage, data, size);

  // TODO - Beneficial to batch many regions in a single call?
  VkBufferCopy copyRegion{};
  copyRegion.srcOffset = m_graphicsStagingBuffer.usage;
  copyRegion.dstOffset = 0; // Optional
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, m_graphicsStagingBuffer.buffer, destination, 1, &copyRegion);

  // TODO - Assert on any needed alignment?
  m_graphicsStagingBuffer.usage += size;
}

void KRDevice::streamUpload(void* data, size_t size, Vector3i dimensions, VkImage destination)
{
  checkFlushStreamBuffer(size);

  memcpy((uint8_t*)m_streamingStagingBuffer.data + m_streamingStagingBuffer.usage, data, size);
  
  streamUploadImpl(size, dimensions, destination, 0, 1);
}

void KRDevice::streamUpload(Block& data, VkImage destination, size_t offset, size_t size, Vector3i dimensions, uint32_t baseMipLevel, uint32_t levelCount)
{
  checkFlushStreamBuffer(size);

  data.copy((uint8_t*)m_streamingStagingBuffer.data + m_streamingStagingBuffer.usage, offset, size);

  streamUploadImpl(size, dimensions, destination, 0, 1);
}

void KRDevice::streamUploadImpl(size_t size, Vector3i dimensions, VkImage destination, uint32_t baseMipLevel, uint32_t levelCount)
{
  // TODO - Refactor memory barriers into helper functions
  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = destination;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  // For VK_IMAGE_LAYOUT_UNDEFINED -> VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
  barrier.srcAccessMask = 0;
  barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

  vkCmdPipelineBarrier(
    m_transferCommandBuffers[0],
    sourceStage, destinationStage,
    0,
    0, nullptr,
    0, nullptr,
    1, &barrier
  );

  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;

  region.imageOffset = { 0, 0, 0 };
  region.imageExtent = {
      (unsigned int)dimensions.x,
      (unsigned int)dimensions.y,
      1
  };

  vkCmdCopyBufferToImage(
    m_transferCommandBuffers[0],
    m_streamingStagingBuffer.buffer,
    destination,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    1,
    &region
  );

  barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  // For VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL -> VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
  barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

  vkCmdPipelineBarrier(
    m_transferCommandBuffers[0],
    sourceStage, destinationStage,
    0,
    0, nullptr,
    0, nullptr,
    1, &barrier
  );

  // TODO - Assert on any needed alignment?
  m_streamingStagingBuffer.usage += size;
}

void KRDevice::streamEnd()
{
  if (m_streamingStagingBuffer.usage == 0) {
    return;
  }
  vkEndCommandBuffer(m_transferCommandBuffers[0]);

  if (m_streamingStagingBuffer.usage > 0) {
    VkResult res = vmaFlushAllocation(m_allocator, m_streamingStagingBuffer.allocation, 0, m_streamingStagingBuffer.usage);
    assert(res == VK_SUCCESS);
    m_streamingStagingBuffer.usage = 0;
  }

  // TODO - Should double buffer and use a fence rather than block the thread
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &m_transferCommandBuffers[0];

  vkQueueSubmit(m_transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(m_transferQueue);

  m_streamingStagingBuffer.started = false;
}
