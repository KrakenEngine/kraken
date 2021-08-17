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
  , m_imageAvailableSemaphore(VK_NULL_HANDLE)
  , m_renderFinishedSemaphore(VK_NULL_HANDLE)
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

  KRDevice* deviceInfo = &m_pContext->getDeviceManager()->getDeviceInfo(m_deviceHandle);

  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  if (vkCreateSemaphore(deviceInfo->m_logicalDevice, &semaphoreInfo, nullptr, &m_imageAvailableSemaphore) != VK_SUCCESS) {
    return KR_ERROR_VULKAN;
  }
  if (vkCreateSemaphore(deviceInfo->m_logicalDevice, &semaphoreInfo, nullptr, &m_renderFinishedSemaphore) != VK_SUCCESS) {
    return KR_ERROR_VULKAN;
  }

  VkSurfaceCapabilitiesKHR surfaceCapabilities{};
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(deviceInfo->m_device, m_surface, &surfaceCapabilities);

  std::vector<VkSurfaceFormatKHR> surfaceFormats;
  uint32_t formatCount = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(deviceInfo->m_device, m_surface, &formatCount, nullptr);


  if (formatCount != 0) {
    surfaceFormats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(deviceInfo->m_device, m_surface, &formatCount, surfaceFormats.data());
  }

  std::vector<VkPresentModeKHR> surfacePresentModes;

  uint32_t presentModeCount = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(deviceInfo->m_device, m_surface, &presentModeCount, nullptr);

  if (presentModeCount != 0) {
    surfacePresentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(deviceInfo->m_device, m_surface, &presentModeCount, surfacePresentModes.data());
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
    deviceInfo->m_graphicsFamilyQueueIndex,
    deviceInfo->m_computeFamilyQueueIndex
  };
  if (deviceInfo->m_graphicsFamilyQueueIndex == deviceInfo->m_computeFamilyQueueIndex) {
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

  if (vkCreateSwapchainKHR(deviceInfo->m_logicalDevice, &swapChainCreateInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
    return KR_ERROR_VULKAN_SWAP_CHAIN;
  }

  vkGetSwapchainImagesKHR(deviceInfo->m_logicalDevice, m_swapChain, &imageCount, nullptr);
  m_swapChainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(deviceInfo->m_logicalDevice, m_swapChain, &imageCount, m_swapChainImages.data());

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
    if (vkCreateImageView(deviceInfo->m_logicalDevice, &createInfo, nullptr, &m_swapChainImageViews[i]) != VK_SUCCESS) {
      return KR_ERROR_VULKAN_SWAP_CHAIN;
    }
  }

  KRPipelineManager* pipelineManager = m_pContext->getPipelineManager();
  pipelineManager->createPipelines(*this);

  KRPipeline* testPipeline = pipelineManager->get("vulkan_test");
  m_swapChainFramebuffers.resize(m_swapChainImageViews.size());

  for (size_t i = 0; i < m_swapChainImageViews.size(); i++) {
    VkImageView attachments[] = { m_swapChainImageViews[i] };

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = testPipeline->getRenderPass();
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = m_swapChainExtent.width;
    framebufferInfo.height = m_swapChainExtent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(deviceInfo->m_logicalDevice, &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]) != VK_SUCCESS) {
      return KR_ERROR_VULKAN_FRAMEBUFFER;
    }
  }

  return KR_SUCCESS;
}

void KRSurface::destroy()
{
  KRDevice& deviceInfo = m_pContext->getDeviceManager()->getDeviceInfo(m_deviceHandle);

  for (auto framebuffer : m_swapChainFramebuffers) {
    vkDestroyFramebuffer(deviceInfo.m_logicalDevice, framebuffer, nullptr);
  }
  m_swapChainFramebuffers.clear();

  for (auto imageView : m_swapChainImageViews) {
    vkDestroyImageView(deviceInfo.m_logicalDevice, imageView, nullptr);
  }
  m_swapChainImageViews.clear();

  if (m_swapChain != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(deviceInfo.m_logicalDevice, m_swapChain, nullptr);
    m_swapChain = VK_NULL_HANDLE;
  }
  
  if (m_renderFinishedSemaphore != VK_NULL_HANDLE) {
    vkDestroySemaphore(deviceInfo.m_logicalDevice, m_renderFinishedSemaphore, nullptr);
    m_renderFinishedSemaphore = VK_NULL_HANDLE;
  }
  
  if (m_imageAvailableSemaphore != VK_NULL_HANDLE) {
    vkDestroySemaphore(deviceInfo.m_logicalDevice, m_imageAvailableSemaphore, nullptr);
    m_imageAvailableSemaphore = VK_NULL_HANDLE;
  }
  
  if (m_surface != VK_NULL_HANDLE) {
    vkDestroySurfaceKHR(m_pContext->getDeviceManager()->getVulkanInstance(), m_surface, nullptr);
    m_surface = VK_NULL_HANDLE;
  }

}
