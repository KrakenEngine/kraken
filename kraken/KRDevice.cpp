//
//  KRDevice.cpp
//  Kraken Engine
//
//  Copyright 2021 Kearwood Gilbert. All rights reserved.
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
  , m_graphicsCommandPool(VK_NULL_HANDLE)
  , m_computeCommandPool(VK_NULL_HANDLE)
  , m_allocator(VK_NULL_HANDLE)
{

}

KRDevice::~KRDevice()
{
  destroy();
}

void KRDevice::destroy()
{
  if (m_logicalDevice != VK_NULL_HANDLE) {
    vkDestroyCommandPool(m_logicalDevice, m_graphicsCommandPool, nullptr);
    m_graphicsCommandPool = VK_NULL_HANDLE;
  }
  
  if (m_computeCommandPool != VK_NULL_HANDLE) {
    vkDestroyCommandPool(m_logicalDevice, m_computeCommandPool, nullptr);
    m_computeCommandPool = VK_NULL_HANDLE;
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
  uint32_t i = 0;
  for (const auto& queueFamily : queueFamilies) {
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      graphicsFamilyQueue = i;
    }
    if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
      computeFamilyQueue = i;
    }
    i++;
  }
  if (graphicsFamilyQueue == -1) {
    // No graphics queue family, not suitable
    return false;
  }

  if (computeFamilyQueue == -1) {
    // No compute queue family, not suitable
    return false;
  }

  m_graphicsFamilyQueueIndex = graphicsFamilyQueue;
  m_computeFamilyQueueIndex = computeFamilyQueue;

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

  VkDeviceQueueCreateInfo queueCreateInfo[2]{};
  float queuePriority = 1.0f;
  queueCreateInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueCreateInfo[0].queueFamilyIndex = m_graphicsFamilyQueueIndex;
  queueCreateInfo[0].queueCount = 1;
  queueCreateInfo[0].pQueuePriorities = &queuePriority;
  if (m_graphicsFamilyQueueIndex != m_computeFamilyQueueIndex) {
    queueCreateInfo[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo[1].queueFamilyIndex = m_computeFamilyQueueIndex;
    queueCreateInfo[1].queueCount = 1;
    queueCreateInfo[1].pQueuePriorities = &queuePriority;
  }

  VkDeviceCreateInfo deviceCreateInfo{};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.pQueueCreateInfos = queueCreateInfo;
  deviceCreateInfo.queueCreateInfoCount = m_graphicsFamilyQueueIndex == m_computeFamilyQueueIndex ? 1 : 2;
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

  const int kMaxGraphicsCommandBuffers = 10; // TODO - This needs to be dynamic?
  m_graphicsCommandBuffers.resize(kMaxGraphicsCommandBuffers);

  const int kMaxComputeCommandBuffers = 4; // TODO - This needs to be dynamic?
  m_computeCommandBuffers.resize(kMaxComputeCommandBuffers);

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

  return true;
}

VmaAllocator KRDevice::getAllocator()
{
  assert(m_allocator != VK_NULL_HANDLE);
  return m_allocator;
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
