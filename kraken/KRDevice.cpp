//
//  KRDevice.cpp
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

#include "KRDevice.h"
#include "KRDeviceManager.h"

KRDevice::KRDevice(KRContext& context, const VkPhysicalDevice& device)
  : KRContextObject(context)
  , m_device(device)
  , m_logicalDevice(VK_NULL_HANDLE)
  , m_deviceProperties {}
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
  , m_streamingStagingBuffer(VK_NULL_HANDLE)
  , m_streamingStagingBufferAllocation(VK_NULL_HANDLE)
  , m_streamingStagingBufferSize(0)
  , m_graphicsStagingBuffer(VK_NULL_HANDLE)
  , m_graphicsStagingBufferAllocation(VK_NULL_HANDLE)
  , m_graphicsStagingBufferSize(0)
{

}

KRDevice::~KRDevice()
{
  destroy();
}

void KRDevice::destroy()
{
  if (m_streamingStagingBuffer) {
    vmaDestroyBuffer(m_allocator, m_streamingStagingBuffer, m_streamingStagingBufferAllocation);
    m_streamingStagingBufferSize = 0;
    m_streamingStagingBuffer = VK_NULL_HANDLE;
    m_streamingStagingBufferAllocation = VK_NULL_HANDLE;
  }

  if (m_graphicsStagingBuffer) {
    vmaDestroyBuffer(m_allocator, m_graphicsStagingBuffer, m_graphicsStagingBufferAllocation);
    m_graphicsStagingBufferSize = 0;
    m_graphicsStagingBuffer = VK_NULL_HANDLE;
    m_graphicsStagingBufferAllocation = VK_NULL_HANDLE;
  }

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

bool KRDevice::initialize(const std::vector<const char*>& deviceExtensions)
{
  vkGetPhysicalDeviceProperties(m_device, &m_deviceProperties);
  vkGetPhysicalDeviceFeatures(m_device, &m_deviceFeatures);

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

  // ----

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

  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.queueFamilyIndex = m_graphicsFamilyQueueIndex;
  poolInfo.flags = 0;

  if (vkCreateCommandPool(m_logicalDevice, &poolInfo, nullptr, &m_graphicsCommandPool) != VK_SUCCESS) {
    destroy();
    // TODO - Log a warning...
    return false;
  }

  poolInfo.queueFamilyIndex = m_computeFamilyQueueIndex;
  if (vkCreateCommandPool(m_logicalDevice, &poolInfo, nullptr, &m_computeCommandPool) != VK_SUCCESS) {
    destroy();
    // TODO - Log a warning...
    return false;
  }

  poolInfo.queueFamilyIndex = m_transferFamilyQueueIndex;
  if (vkCreateCommandPool(m_logicalDevice, &poolInfo, nullptr, &m_transferCommandPool) != VK_SUCCESS) {
    destroy();
    // TODO - Log a warning...
    return false;
  }

  const int kMaxGraphicsCommandBuffers = 10; // TODO - This needs to be dynamic?
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
    destroy();
    // TODO - Log a warning
    return false;
  }

  allocInfo.commandPool = m_computeCommandPool;
  allocInfo.commandBufferCount = (uint32_t)m_computeCommandBuffers.size();
  if (vkAllocateCommandBuffers(m_logicalDevice, &allocInfo, m_computeCommandBuffers.data()) != VK_SUCCESS) {
    destroy();
    // TODO - Log a warning
    return false;
  }

  allocInfo.commandPool = m_transferCommandPool;
  allocInfo.commandBufferCount = (uint32_t)m_transferCommandBuffers.size();
  if (vkAllocateCommandBuffers(m_logicalDevice, &allocInfo, m_transferCommandBuffers.data()) != VK_SUCCESS) {
    destroy();
    // TODO - Log a warning
    return false;
  }

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
    destroy();
    // TODO - Log a warning
    return false;
  }


  // Create Staging Buffer for the transfer queue.
  // This will be used for asynchronous asset streaming in the streamer thread.
  // Start with a 256MB staging buffer.
  // TODO - Dynamically size staging buffer using heuristics
  m_streamingStagingBufferSize = size_t(256) * 1024 * 1024;

  if (!createBuffer(
    m_streamingStagingBufferSize,
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    &m_streamingStagingBuffer,
    &m_streamingStagingBufferAllocation
#if KRENGINE_DEBUG_GPU_LABELS
    , "Streaming Staging Buffer"
#endif // KRENGINE_DEBUG_GPU_LABELS
  )) {
    destroy();
    // TODO - Log a warning
    return false;
  }

  // Create Staging Buffer for the graphics queue.
  // This will be used for uploading assets procedurally generated while recording the graphics command buffer.
  // Start with a 256MB staging buffer.
  // TODO - Dynamically size staging buffer using heuristics
  m_graphicsStagingBufferSize = size_t(256) * 1024 * 1024;

  if (!createBuffer(
    m_graphicsStagingBufferSize,
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    &m_graphicsStagingBuffer,
    &m_graphicsStagingBufferAllocation
#if KRENGINE_DEBUG_GPU_LABELS
    , "Streaming Staging Buffer"
#endif // KRENGINE_DEBUG_GPU_LABELS
  )) {
    destroy();
    // TODO - Log a warning
    return false;
  }

  return true;
}

VmaAllocator KRDevice::getAllocator()
{
  assert(m_allocator != VK_NULL_HANDLE);
  return m_allocator;
}


bool KRDevice::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VmaAllocation* allocation
#if KRENGINE_DEBUG_GPU_LABELS  
  , const char* debug_label
#endif
)
{
  int familyCount = 1;
  uint32_t queueFamilyIndices[2] = {};
  queueFamilyIndices[0] = m_graphicsFamilyQueueIndex;
  if (m_graphicsFamilyQueueIndex != m_transferFamilyQueueIndex) {
    queueFamilyIndices[1] = m_transferFamilyQueueIndex;
    familyCount++;
  }

  VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
  bufferInfo.queueFamilyIndexCount = familyCount;
  bufferInfo.pQueueFamilyIndices = queueFamilyIndices;

  VmaAllocationCreateInfo allocInfo = {};
  allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
  allocInfo.requiredFlags = properties;

  VkResult res = vmaCreateBuffer(m_allocator, &bufferInfo, &allocInfo, buffer, allocation, nullptr);
  if (res != VK_SUCCESS) {
    return false;
  }

#if KRENGINE_DEBUG_GPU_LABELS
  VkDebugUtilsObjectNameInfoEXT debugInfo{};
  debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
  debugInfo.objectHandle = (uint64_t)*buffer;
  debugInfo.objectType = VK_OBJECT_TYPE_BUFFER;
  debugInfo.pObjectName = debug_label;
  res = vkSetDebugUtilsObjectNameEXT(m_logicalDevice, &debugInfo);
#endif // KRENGINE_DEBUG_GPU_LABELS

  return true;
}

KrResult KRDevice::selectSurfaceFormat(VkSurfaceKHR& surface, VkSurfaceFormatKHR& selectedFormat)
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

KrResult KRDevice::selectDepthFormat(VkFormat& selectedDepthFormat)
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

KrResult KRDevice::selectPresentMode(VkSurfaceKHR& surface, VkPresentModeKHR& selectedPresentMode)
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
