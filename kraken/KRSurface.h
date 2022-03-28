//
//  KRSurface.h
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

#include "KREngine-common.h"
#include "KRContext.h"

#ifndef KRSURFACE_H
#define KRSURFACE_H

class KRDevice;
class KRRenderPass;

class KRSurface : public KRContextObject
{
public:
#ifdef WIN32
  KRSurface(KRContext& context, HWND hWnd);
#else
  KRSurface(KRContext& context);
#endif
  ~KRSurface();
  void destroy();
  uint32_t getWidth() const;
  uint32_t getHeight() const;
  VkFormat getDepthFormat() const;

  KRSurface(const KRSurface&) = delete;
  KRSurface& operator=(const KRSurface&) = delete;
  std::unique_ptr<KRDevice>& getDevice();

  KrResult initialize();
  KrResult recreateSwapChain();
  void createRenderPasses();
  KRRenderPass& getForwardOpaquePass();

#ifdef WIN32
  HWND m_hWnd;
#endif
  KrDeviceHandle m_deviceHandle;
  VkSurfaceKHR m_surface;
  VkSwapchainKHR m_swapChain;
  std::vector<VkImage> m_swapChainImages; 
  VkFormat m_swapChainImageFormat;
  VkExtent2D m_swapChainExtent;
  std::vector<VkImageView> m_swapChainImageViews;
  std::vector<VkFramebuffer> m_swapChainFramebuffers;
  VkFormat m_depthImageFormat;
  VkImage m_depthImage;
  VmaAllocation m_depthImageAllocation;
  VkImageView m_depthImageView;
  VkSemaphore m_imageAvailableSemaphore;
  VkSemaphore m_renderFinishedSemaphore;

  std::unique_ptr<KRRenderPass> m_forwardOpaquePass;

private:
  void destroySwapChain();
  KrResult createSwapChain();
};

#endif // KRSURFACE_H
