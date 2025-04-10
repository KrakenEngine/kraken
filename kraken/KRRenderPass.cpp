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

#include "KRRenderPass.h"
#include "KRSurface.h"
#include "KRSwapchain.h"

using namespace hydra;

KRRenderPass::KRRenderPass(KRContext& context)
  : KRContextObject(context)
  , m_renderPass(VK_NULL_HANDLE)
  , m_info{}
{

}

KRRenderPass::~KRRenderPass()
{
  assert(m_renderPass == VK_NULL_HANDLE);
}

void KRRenderPass::create(KRDevice& device, const RenderPassInfo& info, const VkRenderPassCreateInfo& createInfo)
{
  assert(m_renderPass == VK_NULL_HANDLE);
  m_info = info;

  if (vkCreateRenderPass(device.m_logicalDevice, &createInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
    // failed! TODO - Error handling
  }

#if KRENGINE_DEBUG_GPU_LABELS
  device.setDebugLabel(m_renderPass, info.debugLabel);
#endif
}

void KRRenderPass::destroy(KRDevice& device)
{
  if (m_renderPass) {
    vkDestroyRenderPass(device.m_logicalDevice, m_renderPass, nullptr);
    m_renderPass = VK_NULL_HANDLE;
  }
}

void KRRenderPass::begin(VkCommandBuffer& commandBuffer, KRSurface& surface)
{
  int attachmentCount = 0;
  std::array<VkClearValue, RENDER_PASS_ATTACHMENT_MAX_COUNT> clearValues{};
  if (m_info.depthAttachment.id != 0) {
    clearValues[attachmentCount] = m_info.depthAttachment.clearVaue;
    attachmentCount++;
  }
  for(int i=0; i < RENDER_PASS_ATTACHMENT_MAX_COUNT; i++) {
    if(m_info.colorAttachments[i].id != 0) {
      clearValues[attachmentCount] = m_info.colorAttachments[i].clearVaue;
      attachmentCount++;
    }
  }

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = m_renderPass;
  renderPassInfo.framebuffer = surface.m_swapChain->m_framebuffers[surface.m_frameIndex % surface.m_swapChain->m_framebuffers.size()];
  renderPassInfo.renderArea.offset = { 0, 0 };
  renderPassInfo.renderArea.extent = surface.m_swapChain->m_extent;
  renderPassInfo.clearValueCount = attachmentCount;
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void KRRenderPass::end(VkCommandBuffer& commandBuffer)
{
  vkCmdEndRenderPass(commandBuffer);
}


RenderPassType KRRenderPass::getType() const
{
  return m_info.type;
}

bool KRRenderPass::isFinal() const
{
  return m_info.finalPass;
}
