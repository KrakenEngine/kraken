//
//  KRSurface.cpp
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

#include "KRSurface.h"
#include "KRSwapchain.h"
#include "KRRenderPass.h"

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
  , m_imageAvailableSemaphores{VK_NULL_HANDLE}
  , m_renderFinishedSemaphores{VK_NULL_HANDLE}
  , m_frameIndex(0)
{
  m_forwardOpaquePass = std::make_unique<KRRenderPass>(context);
  m_deferredGBufferPass = std::make_unique<KRRenderPass>(context);
  m_deferredOpaquePass = std::make_unique<KRRenderPass>(context);
  m_swapChain = std::make_unique<KRSwapchain>(context);
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
  for(int i = 0; i < KRENGINE_MAX_FRAMES_IN_FLIGHT; i++) {
    if (vkCreateSemaphore(device->m_logicalDevice, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS) {
      return KR_ERROR_VULKAN;
    }
    if (vkCreateSemaphore(device->m_logicalDevice, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS) {
      return KR_ERROR_VULKAN;
    }
  }

  return createSwapChain();
}

void KRSurface::destroy()
{
  destroySwapChain();

  std::unique_ptr<KRDevice>& device = m_pContext->getDeviceManager()->getDevice(m_deviceHandle);

  if (m_forwardOpaquePass) {
    m_forwardOpaquePass->destroy(*device);
  }

  if (m_deferredGBufferPass) {
    m_deferredGBufferPass->destroy(*device);
  }

  if (m_deferredOpaquePass) {
    m_deferredOpaquePass->destroy(*device);
  }

  for (int i=0; i < KRENGINE_MAX_FRAMES_IN_FLIGHT; i++) {
    if (device && m_renderFinishedSemaphores != VK_NULL_HANDLE) {
      vkDestroySemaphore(device->m_logicalDevice, m_renderFinishedSemaphores[i], nullptr);
      m_renderFinishedSemaphores[i] = VK_NULL_HANDLE;
    }

    if (device && m_imageAvailableSemaphores != VK_NULL_HANDLE) {
      vkDestroySemaphore(device->m_logicalDevice, m_imageAvailableSemaphores[i], nullptr);
      m_imageAvailableSemaphores[i] = VK_NULL_HANDLE;
    }
  }

  if (m_surface != VK_NULL_HANDLE) {
    vkDestroySurfaceKHR(m_pContext->getDeviceManager()->getVulkanInstance(), m_surface, nullptr);
    m_surface = VK_NULL_HANDLE;
  }
}

KrResult KRSurface::createSwapChain()
{

  std::unique_ptr<KRDevice>& device = m_pContext->getDeviceManager()->getDevice(m_deviceHandle);

  KrResult res = KR_SUCCESS;
  VkSurfaceFormatKHR selectedSurfaceFormat{};
  res = device->selectSurfaceFormat(m_surface, selectedSurfaceFormat);
  if (res != KR_SUCCESS) return res;

  VkFormat depthImageFormat = VK_FORMAT_UNDEFINED;
  res = device->selectDepthFormat(depthImageFormat);
  if (res != KR_SUCCESS) {
    return res;
  }

  VkSurfaceCapabilitiesKHR surfaceCapabilities{};
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->m_device, m_surface, &surfaceCapabilities);

  VkExtent2D swapExtent;
  if (surfaceCapabilities.currentExtent.width != UINT32_MAX) {
    swapExtent = surfaceCapabilities.currentExtent;
  } else {
    const uint32_t MAX_WIDTH = 8192;
    const uint32_t MAX_HEIGHT = 8192;
    swapExtent.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, MAX_WIDTH));
    swapExtent.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, MAX_HEIGHT));
  }

  uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
  if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount) {
    imageCount = surfaceCapabilities.maxImageCount;
  }


  KRRenderPass::RenderPassInfo info{};
  info.clearDepth = true;
  info.keepDepth = false;
  m_forwardOpaquePass->create(*device, selectedSurfaceFormat.format, depthImageFormat, info);

  info.clearDepth = true;
  info.keepDepth = true;
  m_deferredGBufferPass->create(*device, selectedSurfaceFormat.format, depthImageFormat, info);

  info.clearDepth = false;
  info.keepDepth = false;
  m_deferredOpaquePass->create(*device, selectedSurfaceFormat.format, depthImageFormat, info);

  m_swapChain->create(*device, m_surface, selectedSurfaceFormat, depthImageFormat, swapExtent, imageCount, *m_forwardOpaquePass);

  return KR_SUCCESS;
}

void KRSurface::destroySwapChain()
{
  KRPipelineManager* pipelineManager = m_pContext->getPipelineManager();
  // TODO - Destroy the dependent pipeline..

  std::unique_ptr<KRDevice>& device = m_pContext->getDeviceManager()->getDevice(m_deviceHandle);
  // TODO - Handle device removal
  if (device) {
    m_swapChain->destroy(*device);
  }

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

std::unique_ptr<KRDevice>& KRSurface::getDevice()
{
  return m_pContext->getDeviceManager()->getDevice(m_deviceHandle);
}

uint32_t KRSurface::getWidth() const
{
  return m_swapChain->m_extent.width;
}

uint32_t KRSurface::getHeight() const
{
  return m_swapChain->m_extent.height;
}

VkFormat KRSurface::getDepthFormat() const
{
  return m_swapChain->m_depthFormat;
}

KRRenderPass& KRSurface::getForwardOpaquePass()
{
  return *m_forwardOpaquePass;
}

KRRenderPass& KRSurface::getDeferredGBufferPass()
{
  return *m_deferredGBufferPass;
}

KRRenderPass& KRSurface::getDeferredOpaquePass()
{
  return *m_deferredOpaquePass;
}

void KRSurface::endFrame()
{
  m_frameIndex++;;
}
