//
//  KRSurface.cpp
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

#include "KRSurface.h"

#ifdef WIN32
KRSurface::KRSurface(KRContext& context, HWND hWnd)
#else
KRSurface::KRSurface(KRContext& context)
#endif
  : KRContextObject(context)
#ifdef WIN32
  , m_hWnd(hWnd)
#endif
  , m_deviceHandle(VK_NULL_HANDLE)
  , m_surface(VK_NULL_HANDLE)
  , m_swapChain(VK_NULL_HANDLE)
  , m_swapChainImageFormat(VK_FORMAT_UNDEFINED)
  , m_swapChainExtent({ 0, 0  })
  , m_depthImageFormat(VK_FORMAT_UNDEFINED)
  , m_depthImage(VK_NULL_HANDLE)
  , m_depthImageAllocation(VK_NULL_HANDLE)
  , m_depthImageView(VK_NULL_HANDLE)
  , m_imageAvailableSemaphore(VK_NULL_HANDLE)
  , m_renderFinishedSemaphore(VK_NULL_HANDLE)
  , m_renderPass(VK_NULL_HANDLE)
{

}

KRSurface::~KRSurface()
{
  destroy();
}

KrResult KRSurface::initialize()
{
  VkWin32SurfaceCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  createInfo.hinstance = GetModuleHandle(nullptr);
  createInfo.hwnd = m_hWnd;
  if (vkCreateWin32SurfaceKHR(m_pContext->getDeviceManager()->getVulkanInstance(), &createInfo, nullptr, &m_surface) != VK_SUCCESS) {
    return KR_ERROR_VULKAN;
  }

  m_deviceHandle = m_pContext->getDeviceManager()->getBestDeviceForSurface(m_surface);
  if (m_deviceHandle == 0) {
    return KR_ERROR_NO_DEVICE;
  }

  std::unique_ptr<KRDevice>& device = m_pContext->getDeviceManager()->getDevice(m_deviceHandle);

  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  if (vkCreateSemaphore(device->m_logicalDevice, &semaphoreInfo, nullptr, &m_imageAvailableSemaphore) != VK_SUCCESS) {
    return KR_ERROR_VULKAN;
  }
  if (vkCreateSemaphore(device->m_logicalDevice, &semaphoreInfo, nullptr, &m_renderFinishedSemaphore) != VK_SUCCESS) {
    return KR_ERROR_VULKAN;
  }

  return createSwapChain();
}

void KRSurface::destroy()
{
  destroySwapChain();

  std::unique_ptr<KRDevice>& device = m_pContext->getDeviceManager()->getDevice(m_deviceHandle);
 
  if (m_renderPass) {
    vkDestroyRenderPass(device->m_logicalDevice, m_renderPass, nullptr);
    m_renderPass = VK_NULL_HANDLE;
  }

  if (device && m_renderFinishedSemaphore != VK_NULL_HANDLE) {
    vkDestroySemaphore(device->m_logicalDevice, m_renderFinishedSemaphore, nullptr);
    m_renderFinishedSemaphore = VK_NULL_HANDLE;
  }
  
  if (device && m_imageAvailableSemaphore != VK_NULL_HANDLE) {
    vkDestroySemaphore(device->m_logicalDevice, m_imageAvailableSemaphore, nullptr);
    m_imageAvailableSemaphore = VK_NULL_HANDLE;
  }
  
  if (m_surface != VK_NULL_HANDLE) {
    vkDestroySurfaceKHR(m_pContext->getDeviceManager()->getVulkanInstance(), m_surface, nullptr);
    m_surface = VK_NULL_HANDLE;
  }
}

KrResult KRSurface::createSwapChain()
{
  std::unique_ptr<KRDevice>& device = m_pContext->getDeviceManager()->getDevice(m_deviceHandle);

  VkSurfaceCapabilitiesKHR surfaceCapabilities{};
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->m_device, m_surface, &surfaceCapabilities);

  std::vector<VkSurfaceFormatKHR> surfaceFormats;
  uint32_t formatCount = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device->m_device, m_surface, &formatCount, nullptr);

  if (formatCount != 0) {
    surfaceFormats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device->m_device, m_surface, &formatCount, surfaceFormats.data());
  }

  std::vector<VkPresentModeKHR> surfacePresentModes;

  uint32_t presentModeCount = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device->m_device, m_surface, &presentModeCount, nullptr);

  if (presentModeCount != 0) {
    surfacePresentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device->m_device, m_surface, &presentModeCount, surfacePresentModes.data());
  }

  VkSurfaceFormatKHR selectedSurfaceFormat = surfaceFormats[0];
  for (const auto& availableFormat : surfaceFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      selectedSurfaceFormat = availableFormat;
      break;
    }
  }

  // VK_PRESENT_MODE_FIFO_KHR is always available
  VkPresentModeKHR selectedPresentMode = VK_PRESENT_MODE_FIFO_KHR;

  // Try to find a better mode
  for (const auto& availablePresentMode : surfacePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      selectedPresentMode = availablePresentMode;
    }
  }

  VkExtent2D swapExtent;
  if (surfaceCapabilities.currentExtent.width != UINT32_MAX) {
    swapExtent = surfaceCapabilities.currentExtent;
  }
  else {
    const uint32_t MAX_WIDTH = 8192;
    const uint32_t MAX_HEIGHT = 8192;
    swapExtent.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, MAX_WIDTH));
    swapExtent.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, MAX_HEIGHT));
  }
  m_swapChainExtent = swapExtent;

  uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
  if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount) {
    imageCount = surfaceCapabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR swapChainCreateInfo{};
  swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapChainCreateInfo.surface = m_surface;
  swapChainCreateInfo.minImageCount = imageCount;
  swapChainCreateInfo.imageFormat = selectedSurfaceFormat.format;
  swapChainCreateInfo.imageColorSpace = selectedSurfaceFormat.colorSpace;
  swapChainCreateInfo.imageExtent = swapExtent;
  swapChainCreateInfo.imageArrayLayers = 1;
  swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  uint32_t queueFamilyIndices[] = {
    device->m_graphicsFamilyQueueIndex,
    device->m_computeFamilyQueueIndex
  };
  if (device->m_graphicsFamilyQueueIndex == device->m_computeFamilyQueueIndex) {
    swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapChainCreateInfo.queueFamilyIndexCount = 0;
    swapChainCreateInfo.pQueueFamilyIndices = nullptr;
  }
  else {
    swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapChainCreateInfo.queueFamilyIndexCount = 2;
    swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
  }

  swapChainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
  swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapChainCreateInfo.presentMode = selectedPresentMode;
  swapChainCreateInfo.clipped = VK_TRUE;
  swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

  if (vkCreateSwapchainKHR(device->m_logicalDevice, &swapChainCreateInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
    return KR_ERROR_VULKAN_SWAP_CHAIN;
  }

  vkGetSwapchainImagesKHR(device->m_logicalDevice, m_swapChain, &imageCount, nullptr);
  m_swapChainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(device->m_logicalDevice, m_swapChain, &imageCount, m_swapChainImages.data());

  m_swapChainImageFormat = selectedSurfaceFormat.format;

  m_swapChainImageViews.resize(m_swapChainImages.size());
  for (size_t i = 0; i < m_swapChainImages.size(); i++) {
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = m_swapChainImages[i];
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = m_swapChainImageFormat;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;
    if (vkCreateImageView(device->m_logicalDevice, &createInfo, nullptr, &m_swapChainImageViews[i]) != VK_SUCCESS) {
      return KR_ERROR_VULKAN_SWAP_CHAIN;
    }
  }

  m_depthImageFormat = VK_FORMAT_UNDEFINED;
  VkFormatFeatureFlags requiredFeatures = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
  std::vector<VkFormat> candidateFormats;
  candidateFormats.push_back(VK_FORMAT_D32_SFLOAT_S8_UINT);
  candidateFormats.push_back(VK_FORMAT_D24_UNORM_S8_UINT);
  for (VkFormat format : candidateFormats) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(device->m_device, format, &props);

    if ((props.optimalTilingFeatures & requiredFeatures) == requiredFeatures) {
      m_depthImageFormat = format;
      break;
    }
  }

  if (m_depthImageFormat == VK_FORMAT_UNDEFINED) {
    return KR_ERROR_VULKAN_DEPTHBUFFER;
  }

  /*
  createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
  depthImageView = createImageView(depthImage, depthFormat);
  */
  {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = m_swapChainExtent.width;
    imageInfo.extent.height = m_swapChainExtent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = m_depthImageFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    VmaAllocator allocator = device->getAllocator();
    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocationCreateInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vmaCreateImage(allocator, &imageInfo, &allocationCreateInfo, &m_depthImage, &m_depthImageAllocation, nullptr) != VK_SUCCESS) {
      return KR_ERROR_VULKAN_DEPTHBUFFER;
    }

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_depthImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = m_depthImageFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device->m_logicalDevice, &viewInfo, nullptr, &m_depthImageView) != VK_SUCCESS) {
      return KR_ERROR_VULKAN_DEPTHBUFFER;
    }

    /*
    TODO - Track memory usage
    
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device->m_logicalDevice, m_depthImage, &memRequirements);
    */
  }

  createRenderPasses();

  m_swapChainFramebuffers.resize(m_swapChainImageViews.size());

  for (size_t i = 0; i < m_swapChainImageViews.size(); i++) {
    VkImageView attachments[] = { m_swapChainImageViews[i] };

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = getRenderPass();
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = m_swapChainExtent.width;
    framebufferInfo.height = m_swapChainExtent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(device->m_logicalDevice, &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]) != VK_SUCCESS) {
      return KR_ERROR_VULKAN_FRAMEBUFFER;
    }
  }

  return KR_SUCCESS;
}

void KRSurface::destroySwapChain()
{
  KRPipelineManager* pipelineManager = m_pContext->getPipelineManager();
  // TODO - Destroy the dependent pipeline..

  std::unique_ptr<KRDevice>& device = m_pContext->getDeviceManager()->getDevice(m_deviceHandle);
  // TODO - Handle device removal
  if (device) {
    for (auto framebuffer : m_swapChainFramebuffers) {
      vkDestroyFramebuffer(device->m_logicalDevice, framebuffer, nullptr);
    }
    
    for (auto imageView : m_swapChainImageViews) {
      vkDestroyImageView(device->m_logicalDevice, imageView, nullptr);
    }
    
    if (m_swapChain != VK_NULL_HANDLE) {
      vkDestroySwapchainKHR(device->m_logicalDevice, m_swapChain, nullptr);
      m_swapChain = VK_NULL_HANDLE;
    }
  }

  if (m_depthImageView) {
    vkDestroyImageView(device->m_logicalDevice, m_depthImageView, nullptr);
    m_depthImageView = VK_NULL_HANDLE;
  }
  
  if (m_depthImage) {
    vmaDestroyImage(device->getAllocator(), m_depthImage, m_depthImageAllocation);
    m_depthImage = VK_NULL_HANDLE;
    m_depthImageAllocation = VK_NULL_HANDLE;
  }
  
  m_swapChainFramebuffers.clear();
  m_swapChainImageViews.clear();
}

KrResult KRSurface::recreateSwapChain()
{
  destroySwapChain();
  KrResult result = createSwapChain();
  if (result != KR_SUCCESS) {
    destroySwapChain();
  }
  return result;
}

void KRSurface::createRenderPasses()
{
  if (m_renderPass) {
    return;
  }

  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = m_swapChainImageFormat;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentRef{};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;

  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &colorAttachment;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  std::unique_ptr<KRDevice>& device = m_pContext->getDeviceManager()->getDevice(m_deviceHandle);

  if (vkCreateRenderPass(device->m_logicalDevice, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
    // failed! TODO - Error handling
  }
}

VkRenderPass& KRSurface::getRenderPass()
{
  return m_renderPass;
}

std::unique_ptr<KRDevice>& KRSurface::getDevice()
{
  return m_pContext->getDeviceManager()->getDevice(m_deviceHandle);
}

uint32_t KRSurface::getWidth() const
{
  return m_swapChainExtent.width;
}

uint32_t KRSurface::getHeight() const
{
  return m_swapChainExtent.height;
}
