//
//  KRSurface.h
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

#pragma once

#include "KREngine-common.h"
#include "KRContext.h"

class KRDevice;
class KRRenderPass;
class KRSwapchain;
class KRRenderGraph;
class KRRenderGraphBlackFrame;
class KRRenderGraphDeferred;
class KRRenderGraphForward;
enum RenderPassType : uint8_t;

class KRSurface : public KRContextObject
{
public:
  KRSurface(KRContext& context, KrSurfaceHandle handle, void* platformHandle);
  ~KRSurface();
  void destroy();
  uint32_t getWidth() const;
  uint32_t getHeight() const;
  hydra::Vector2i getDimensions() const;
  VkFormat getDepthFormat() const;
  VkFormat getSurfaceFormat() const;

  KRSurface(const KRSurface&) = delete;
  KRSurface& operator=(const KRSurface&) = delete;
  std::unique_ptr<KRDevice>& getDevice();

  KrResult initialize();
  KrResult recreateSwapChain();

  void endFrame();
  KrSurfaceHandle m_handle;

  void* m_platformHandle;
  KrDeviceHandle m_deviceHandle;
  VkSurfaceKHR m_surface;

  VkSemaphore m_imageAvailableSemaphores[KRENGINE_MAX_FRAMES_IN_FLIGHT];
  VkSemaphore m_renderFinishedSemaphores[KRENGINE_MAX_FRAMES_IN_FLIGHT];
  VkFence m_inFlightFences[KRENGINE_MAX_FRAMES_IN_FLIGHT];

  std::unique_ptr<KRSwapchain> m_swapChain;

  // TODO - This needs to be advanced per swap chain
  uint64_t m_frameIndex;

private:
  void destroySwapChain();
  KrResult createSwapChain();

public:
  // TODO - These need to be relocated...
  std::unique_ptr<KRRenderGraphForward> m_renderGraphForward;
  std::unique_ptr<KRRenderGraphDeferred> m_renderGraphDeferred;
  std::unique_ptr<KRRenderGraphBlackFrame> m_renderGraphBlackFrame;
  
  VkSurfaceFormatKHR m_surfaceFormat;
};
