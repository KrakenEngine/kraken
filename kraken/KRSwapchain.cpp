//
//  KRSwapchain.cpp
//  Kraken Engine
//
//  Copyright 2025 Kearwood Gilbert. All rights reserved.
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

#include "KRSwapchain.h"
#include "KRRenderPass.h"

KRSwapchain::KRSwapchain(KRContext& context)
  : KRContextObject(context)
  , m_swapChain(VK_NULL_HANDLE)
  , m_extent({ 0, 0 })
  , m_imageFormat(VK_FORMAT_UNDEFINED)
  , m_depthFormat(VK_FORMAT_UNDEFINED)
  , m_depthImage(VK_NULL_HANDLE)
  , m_depthImageAllocation(VK_NULL_HANDLE)
  , m_depthImageView(VK_NULL_HANDLE)
{

}

KRSwapchain::~KRSwapchain()
{
  assert(m_swapChain == VK_NULL_HANDLE);
}

KrResult KRSwapchain::create(KRDevice& device, VkSurfaceKHR& surface, VkSurfaceFormatKHR& surfaceFormat, VkFormat depthFormat, VkExtent2D& extent, uint32_t imageCount, const KRRenderPass& renderPass)
{
  KrResult res = KR_SUCCESS;

  m_extent = extent;
  VkSurfaceCapabilitiesKHR surfaceCapabilities{};
  if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.m_device, surface, &surfaceCapabilities) != VK_SUCCESS) {
    return KR_ERROR_VULKAN_SWAP_CHAIN;
  }

  VkPresentModeKHR selectedPresentMode;
  res = device.selectPresentMode(surface, selectedPresentMode);
  if (res != KR_SUCCESS) return res;

  m_imageFormat = surfaceFormat.format;
  m_depthFormat = depthFormat;

  VkSwapchainCreateInfoKHR swapChainCreateInfo{};
  swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapChainCreateInfo.surface = surface;
  swapChainCreateInfo.minImageCount = imageCount;
  swapChainCreateInfo.imageFormat = surfaceFormat.format;
  swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
  swapChainCreateInfo.imageExtent = extent;
  swapChainCreateInfo.imageArrayLayers = 1;
  swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  uint32_t queueFamilyIndices[] = {
    device.m_graphicsFamilyQueueIndex,
    device.m_computeFamilyQueueIndex
  };
  if (device.m_graphicsFamilyQueueIndex == device.m_computeFamilyQueueIndex) {
    swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapChainCreateInfo.queueFamilyIndexCount = 0;
    swapChainCreateInfo.pQueueFamilyIndices = nullptr;
  } else {
    swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapChainCreateInfo.queueFamilyIndexCount = 2;
    swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
  }

  swapChainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
  swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapChainCreateInfo.presentMode = selectedPresentMode;
  swapChainCreateInfo.clipped = VK_TRUE;
  swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

  if (vkCreateSwapchainKHR(device.m_logicalDevice, &swapChainCreateInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
    return KR_ERROR_VULKAN_SWAP_CHAIN;
  }

  vkGetSwapchainImagesKHR(device.m_logicalDevice, m_swapChain, &imageCount, nullptr);
  m_images.resize(imageCount);
  vkGetSwapchainImagesKHR(device.m_logicalDevice, m_swapChain, &imageCount, m_images.data());

  m_imageViews.resize(m_images.size());
  for (size_t i = 0; i < m_images.size(); i++) {
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = m_images[i];
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = m_imageFormat;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;
    if (vkCreateImageView(device.m_logicalDevice, &createInfo, nullptr, &m_imageViews[i]) != VK_SUCCESS) {
      return KR_ERROR_VULKAN_SWAP_CHAIN;
    }
  }


  {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = m_extent.width;
    imageInfo.extent.height = m_extent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = depthFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    VmaAllocator allocator = device.getAllocator();
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
    viewInfo.format = depthFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device.m_logicalDevice, &viewInfo, nullptr, &m_depthImageView) != VK_SUCCESS) {
      return KR_ERROR_VULKAN_DEPTHBUFFER;
    }

    /*
    TODO - Track memory usage

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device->m_logicalDevice, m_depthImage, &memRequirements);
    */
  }

  m_framebuffers.resize(m_imageViews.size());

  for (size_t i = 0; i < m_imageViews.size(); i++) {
    std::array<VkImageView, 2> attachments = {
      m_depthImageView,
      m_imageViews[i]
    };

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass.m_renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = m_extent.width;
    framebufferInfo.height = m_extent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(device.m_logicalDevice, &framebufferInfo, nullptr, &m_framebuffers[i]) != VK_SUCCESS) {
      return KR_ERROR_VULKAN_FRAMEBUFFER;
    }
  }
  return KR_SUCCESS;
}

void KRSwapchain::destroy(KRDevice& device)
{
  for (auto framebuffer : m_framebuffers) {
    vkDestroyFramebuffer(device.m_logicalDevice, framebuffer, nullptr);
  }

  for (auto imageView : m_imageViews) {
    vkDestroyImageView(device.m_logicalDevice, imageView, nullptr);
  }

  if (m_swapChain != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(device.m_logicalDevice, m_swapChain, nullptr);
    m_swapChain = VK_NULL_HANDLE;
  }

  if (m_depthImageView) {
    vkDestroyImageView(device.m_logicalDevice, m_depthImageView, nullptr);
    m_depthImageView = VK_NULL_HANDLE;
  }

  if (m_depthImage) {
    vmaDestroyImage(device.getAllocator(), m_depthImage, m_depthImageAllocation);
    m_depthImage = VK_NULL_HANDLE;
    m_depthImageAllocation = VK_NULL_HANDLE;
  }

  m_framebuffers.clear();
  m_imageViews.clear();
}
