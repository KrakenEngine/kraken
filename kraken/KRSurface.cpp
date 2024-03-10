//
//  KRSurface.cpp
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

#include "KRSurface.h"
#include "KRSwapchain.h"
#include "KRRenderPass.h"

using namespace hydra;

KRSurface::KRSurface(KRContext& context, KrSurfaceHandle handle, void* platformHandle)
  : KRContextObject(context)
  , m_handle(handle)
  , m_platformHandle(platformHandle)
  , m_deviceHandle(0)
  , m_surface(VK_NULL_HANDLE)
  , m_imageAvailableSemaphores{VK_NULL_HANDLE}
  , m_renderFinishedSemaphores{VK_NULL_HANDLE}
  , m_inFlightFences{VK_NULL_HANDLE}
  , m_frameIndex(0)
  , m_renderGraph(std::make_unique<KRRenderGraph>(context))
  , m_blackFrameRenderGraph(std::make_unique<KRRenderGraph>(context))
  , m_swapChain(std::make_unique<KRSwapchain>(context))
{
}

KRSurface::~KRSurface()
{
  destroy();
}

KrResult KRSurface::initialize()
{
#if defined(WIN32)
  VkWin32SurfaceCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  createInfo.hinstance = GetModuleHandle(nullptr);
  createInfo.hwnd = static_cast<HWND>(m_platformHandle);
  if (vkCreateWin32SurfaceKHR(m_pContext->getDeviceManager()->getVulkanInstance(), &createInfo, nullptr, &m_surface) != VK_SUCCESS) {
    return KR_ERROR_VULKAN;
  }
#elif defined(__APPLE__)
  VkMetalSurfaceCreateInfoEXT createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
  createInfo.pLayer = static_cast<CAMetalLayer*>(m_platformHandle);
  if (vkCreateMetalSurfaceEXT(m_pContext->getDeviceManager()->getVulkanInstance(), &createInfo, nullptr, &m_surface) != VK_SUCCESS) {
    return KR_ERROR_VULKAN;
  }
#else
#error Unsupported
#endif

  m_deviceHandle = m_pContext->getDeviceManager()->getBestDeviceForSurface(m_surface);
  if (m_deviceHandle == 0) {
    return KR_ERROR_NO_DEVICE;
  }

  std::unique_ptr<KRDevice>& device = m_pContext->getDeviceManager()->getDevice(m_deviceHandle);

  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for(int i = 0; i < KRENGINE_MAX_FRAMES_IN_FLIGHT; i++) {
    if (vkCreateSemaphore(device->m_logicalDevice, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS) {
      return KR_ERROR_VULKAN;
    }
    if (vkCreateSemaphore(device->m_logicalDevice, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS) {
      return KR_ERROR_VULKAN;
    }
    if (vkCreateFence(device->m_logicalDevice, &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS) {
      return KR_ERROR_VULKAN;
    }
  }

  return createSwapChain();
}

void KRSurface::destroy()
{
  destroySwapChain();

  std::unique_ptr<KRDevice>& device = m_pContext->getDeviceManager()->getDevice(m_deviceHandle);
  m_renderGraph->destroy(*device);
  m_blackFrameRenderGraph->destroy(*device);

  for (int i=0; i < KRENGINE_MAX_FRAMES_IN_FLIGHT; i++) {
    if (device && m_renderFinishedSemaphores[i] != VK_NULL_HANDLE) {
      vkDestroySemaphore(device->m_logicalDevice, m_renderFinishedSemaphores[i], nullptr);
      m_renderFinishedSemaphores[i] = VK_NULL_HANDLE;
    }

    if (device && m_imageAvailableSemaphores[i] != VK_NULL_HANDLE) {
      vkDestroySemaphore(device->m_logicalDevice, m_imageAvailableSemaphores[i], nullptr);
      m_imageAvailableSemaphores[i] = VK_NULL_HANDLE;
    }

    if (device && m_inFlightFences[i] != VK_NULL_HANDLE) {
      vkDestroyFence(device->m_logicalDevice, m_inFlightFences[i], nullptr);
      m_inFlightFences[i] = VK_NULL_HANDLE;
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
  
  // ----- Configuration -----
  int shadow_buffer_count = 0;
  bool enable_deferred_lighting = false;
  // -------------------------
  
  int attachment_compositeDepth = m_renderGraph->addAttachment("Composite Depth", depthImageFormat);
  int attachment_compositeColor = m_renderGraph->addAttachment("Composite Color", selectedSurfaceFormat.format);
  int attachment_lightAccumulation = m_renderGraph->addAttachment("Light Accumulation", VK_FORMAT_B8G8R8A8_UINT);
  int attachment_gbuffer = m_renderGraph->addAttachment("GBuffer", VK_FORMAT_B8G8R8A8_UINT);
  int attachment_shadow_cascades[3];
  attachment_shadow_cascades[0] = m_renderGraph->addAttachment("Shadow Cascade 0", VK_FORMAT_D32_SFLOAT);
  attachment_shadow_cascades[1] = m_renderGraph->addAttachment("Shadow Cascade 1", VK_FORMAT_D32_SFLOAT);
  attachment_shadow_cascades[2] = m_renderGraph->addAttachment("Shadow Cascade 2", VK_FORMAT_D32_SFLOAT);

  RenderPassInfo info{};
  info.finalPass = false;
  
  // info.type = RenderPassType::RENDER_PASS_PRESTREAM;
  // m_renderGraph->addRenderPass(*device, info);

  for (int shadow_index = 0; shadow_index < shadow_buffer_count; shadow_index++) {
    info.depthAttachment.id = attachment_shadow_cascades[shadow_index];
    info.depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    info.depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    info.depthAttachment.clearVaue.depthStencil.depth = 1.0f;
    info.depthAttachment.clearVaue.depthStencil.stencil = 0;
    info.type = RenderPassType::RENDER_PASS_SHADOWMAP;
    m_renderGraph->addRenderPass(*device, info);
  }
  
  if (enable_deferred_lighting) {
    //  ----====---- Opaque Geometry, Deferred rendering Pass 1 ----====----
    
    info.depthAttachment.id = attachment_compositeDepth;
    info.depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    info.depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    info.depthAttachment.clearVaue.depthStencil.depth = 1.0f;
    info.depthAttachment.clearVaue.depthStencil.stencil = 0;
    
    info.colorAttachments[0].id = attachment_compositeColor;
    info.colorAttachments[0].clearVaue.color.float32[0] = 0.0f;
    info.colorAttachments[0].clearVaue.color.float32[1] = 0.0f;
    info.colorAttachments[0].clearVaue.color.float32[2] = 0.0f;
    info.colorAttachments[0].clearVaue.color.float32[3] = 0.0f;
    info.colorAttachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    info.colorAttachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    info.type = RenderPassType::RENDER_PASS_DEFERRED_GBUFFER;
    m_renderGraph->addRenderPass(*device, info);
    
    //  ----====---- Opaque Geometry, Deferred rendering Pass 2 ----====----
    
    info.depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    info.colorAttachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    
    info.colorAttachments[1].id = attachment_lightAccumulation;
    info.colorAttachments[1].clearVaue.color.float32[0] = 0.0f;
    info.colorAttachments[1].clearVaue.color.float32[1] = 0.0f;
    info.colorAttachments[1].clearVaue.color.float32[2] = 0.0f;
    info.colorAttachments[1].clearVaue.color.float32[3] = 0.0f;
    info.colorAttachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    info.colorAttachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    info.type = RenderPassType::RENDER_PASS_DEFERRED_LIGHTS;
    m_renderGraph->addRenderPass(*device, info);
    
    //  ----====---- Opaque Geometry, Deferred rendering Pass 3 ----====----
    info.colorAttachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    info.colorAttachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    info.type = RenderPassType::RENDER_PASS_DEFERRED_OPAQUE;
    m_renderGraph->addRenderPass(*device, info);
    
    info.colorAttachments[1] = {};

  } else {
    // !enable_deferred_lighting
    
    // ----====---- Opaque Geometry, Forward Rendering ----====----
    info.depthAttachment.id = attachment_compositeDepth;
    info.depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    info.depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    info.depthAttachment.clearVaue.depthStencil.depth = 1.0f;
    info.depthAttachment.clearVaue.depthStencil.stencil = 0;
    
    info.colorAttachments[0].id = attachment_compositeColor;
    info.colorAttachments[0].clearVaue.color.float32[0] = 0.0f;
    info.colorAttachments[0].clearVaue.color.float32[1] = 0.0f;
    info.colorAttachments[0].clearVaue.color.float32[2] = 0.0f;
    info.colorAttachments[0].clearVaue.color.float32[3] = 0.0f;
    info.colorAttachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    info.colorAttachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    
    info.type = RenderPassType::RENDER_PASS_FORWARD_OPAQUE;
    m_renderGraph->addRenderPass(*device, info);
  }
  
  // ----====---- Transparent Geometry, Forward Rendering ----====----
  info.depthAttachment.id = attachment_compositeDepth;
  info.depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  info.depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
  info.depthAttachment.clearVaue.depthStencil.depth = 1.0f;
  info.depthAttachment.clearVaue.depthStencil.stencil = 0;
  
  info.colorAttachments[0].id = attachment_compositeColor;
  info.colorAttachments[0].clearVaue.color.float32[0] = 0.0f;
  info.colorAttachments[0].clearVaue.color.float32[1] = 0.0f;
  info.colorAttachments[0].clearVaue.color.float32[2] = 0.0f;
  info.colorAttachments[0].clearVaue.color.float32[3] = 0.0f;
  info.colorAttachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
  info.colorAttachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  
  info.type = RenderPassType::RENDER_PASS_FORWARD_TRANSPARENT;
  m_renderGraph->addRenderPass(*device, info);
  
  info.type = RenderPassType::RENDER_PASS_DEBUG_OVERLAYS;
  m_renderGraph->addRenderPass(*device, info);
  
  info.finalPass = true;
  info.type = RenderPassType::RENDER_PASS_POST_COMPOSITE;
  m_renderGraph->addRenderPass(*device, info);
  
  
  int attachment_blackFrameDepth = m_blackFrameRenderGraph->addAttachment("Composite Depth", depthImageFormat);
  int attachment_blackFrameColor = m_blackFrameRenderGraph->addAttachment("Composite Color", selectedSurfaceFormat.format);
  
  info.colorAttachments[0].id = attachment_blackFrameColor;
  info.colorAttachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  info.depthAttachment.id = attachment_blackFrameDepth;
  info.depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  info.finalPass = true;
  info.type = RenderPassType::RENDER_PASS_BLACK_FRAME;
  m_blackFrameRenderGraph->addRenderPass(*device, info);

  m_swapChain->create(*device, m_surface, selectedSurfaceFormat, depthImageFormat, swapExtent, imageCount, *m_renderGraph->getRenderPass(RenderPassType::RENDER_PASS_FORWARD_OPAQUE));

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

Vector2i KRSurface::getDimensions() const
{
  return Vector2i::Create(static_cast<int>(m_swapChain->m_extent.width), static_cast<int>(m_swapChain->m_extent.height));
}

VkFormat KRSurface::getDepthFormat() const
{
  return m_swapChain->m_depthFormat;
}

KRRenderPass* KRSurface::getRenderPass(RenderPassType type)
{
  return m_renderGraph->getRenderPass(type);
}

void KRSurface::endFrame()
{
  m_frameIndex++;;
}


void KRSurface::renderBlackFrame(VkCommandBuffer &commandBuffer)
{
  m_blackFrameRenderGraph->render(commandBuffer, *this, nullptr);
}
