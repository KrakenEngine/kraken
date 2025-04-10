//
//  KRPresentationThread.cpp
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

#include "KRPresentationThread.h"
#include "KRRenderPass.h"
#include "KRSwapchain.h"
#include "KRRenderGraph.h"
#include "KRRenderGraphDeferred.h"
#include "KRRenderGraphForward.h"
#include "KRRenderGraphBlackFrame.h"

KRPresentationThread::KRPresentationThread(KRContext& context)
  : KRContextObject(context)
  , m_requestedState(PresentThreadRequest::stop)
  , m_activeState(PresentThreadState::stop)
  , m_currentFrame(0)
{

}

KRPresentationThread::~KRPresentationThread()
{

}

void KRPresentationThread::start()
{
  m_requestedState = PresentThreadRequest::run;
  m_thread = std::thread(&KRPresentationThread::run, this);
}

void KRPresentationThread::stop()
{
  m_requestedState = PresentThreadRequest::stop;
  m_thread.join();
}

void KRPresentationThread::run()
{
#if defined(ANDROID)
  // TODO - Set thread names on Android
#elif defined(_WIN32) || defined(_WIN64)
  // TODO - Set thread names on windows
#else
  pthread_setname_np("Kraken - Presentation");
#endif

  std::chrono::microseconds sleep_duration(15000);

  m_activeState = PresentThreadState::run;
  while (m_requestedState != PresentThreadRequest::stop) {
    switch (m_activeState) {
    case PresentThreadState::pause:
    case PresentThreadState::stop:
      if (m_requestedState == PresentThreadRequest::run) {
        m_activeState = PresentThreadState::run;
      }
      break;
    case PresentThreadState::run:
      if (m_requestedState == PresentThreadRequest::pause) {
        m_activeState = PresentThreadState::pause;
      } else {
        renderFrame();
      }
      break;
    case PresentThreadState::error:
      break;
    }
    std::this_thread::sleep_for(sleep_duration);
  }
  m_activeState = PresentThreadState::stop;
}

void KRPresentationThread::renderFrame()
{
  // TODO - We should use fences to eliminate this mutex
  const std::lock_guard<std::mutex> surfaceLock(KRContext::g_SurfaceInfoMutex);

  unordered_map<KrSurfaceHandle, std::unique_ptr<KRSurface>>& surfaces = m_pContext->getSurfaceManager()->getSurfaces();

  KRSceneManager* sceneManager = m_pContext->getSceneManager();
  KRScene* scene = sceneManager->getFirstScene();

  for (auto surfaceItr = surfaces.begin(); surfaceItr != surfaces.end(); surfaceItr++) {
    KRSurface& surface = *(*surfaceItr).second;
    KRDevice& device = *m_pContext->getDeviceManager()->getDevice(surface.m_deviceHandle);
    // TODO - Handle device removal

    VkSurfaceCapabilitiesKHR surfaceCapabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.m_device, surface.m_surface, &surfaceCapabilities);
    if (surfaceCapabilities.currentExtent.width == 0 || surfaceCapabilities.currentExtent.height == 0) {
      // The window may be minimized...  Pause rendering until restored.
      break;
    }
    bool resized = false;
    if (surface.m_swapChain->m_extent.width != surfaceCapabilities.currentExtent.width ||
      surface.m_swapChain->m_extent.height != surfaceCapabilities.currentExtent.height) {
      // We can't rely on VK_ERROR_OUT_OF_DATE_KHR to always signal when a resize has happend.
      // This must also be checked for explicitly.
      resized = true;
    }

    vkWaitForFences(device.m_logicalDevice, 1, &surface.m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex = 0;
    VkResult result = vkAcquireNextImageKHR(device.m_logicalDevice, surface.m_swapChain->m_swapChain, UINT64_MAX, surface.m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || resized) {
      // TODO - Must explicitly detect resize and trigger swapchain re-creation as well
      vkDeviceWaitIdle(device.m_logicalDevice);
      if (surface.recreateSwapChain() != KR_SUCCESS) {
        m_activeState = PresentThreadState::error;
      }
      break;
    } else if (result != VK_SUCCESS) {
      m_activeState = PresentThreadState::error;
      break;
    }

    // Only reset the fence once we know we'll submit work,
    // avoiding a deadlock on swapchain recreation.
    vkResetFences(device.m_logicalDevice, 1, &surface.m_inFlightFences[m_currentFrame]);

    // TODO - this will break with more than one surface...  Expect to refactor this out
    VkCommandBuffer commandBuffer = device.m_graphicsCommandBuffers[m_currentFrame];

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
      m_activeState = PresentThreadState::error;
      // TODO - Add error handling...
    }

    // TODO - This needs to be moved to the Render thread...
    float deltaTime = 0.005; // TODO - Replace dummy value
    if (scene) {
      KRRenderGraphForward* renderGraph = surface.m_renderGraphForward.get();
      scene->renderFrame(commandBuffer, surface, *renderGraph, deltaTime);
    } else {
      surface.m_renderGraphBlackFrame->render(commandBuffer, surface, nullptr);
    }

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
      m_activeState = PresentThreadState::error;
      // TODO - Add error handling...
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { surface.m_imageAvailableSemaphores[m_currentFrame]};
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signalSemaphores[] = { surface.m_renderFinishedSemaphores[m_currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(device.m_graphicsQueue, 1, &submitInfo, surface.m_inFlightFences[m_currentFrame]) != VK_SUCCESS) {
      m_activeState = PresentThreadState::error;
      // TODO - Add error handling...
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &surface.m_swapChain->m_swapChain;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;
    vkQueuePresentKHR(device.m_graphicsQueue, &presentInfo);

    surface.endFrame();

    m_currentFrame = (m_currentFrame + 1) % KRENGINE_MAX_FRAMES_IN_FLIGHT;
  }
}
