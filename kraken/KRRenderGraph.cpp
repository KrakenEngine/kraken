//
//  KRRenderGraph.cpp
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

#include "KRRenderGraph.h"
#include "KRRenderPass.h"
#include "KRSurface.h"
#include "KRDevice.h"

KRRenderGraph::KRRenderGraph(KRContext& context)
  : KRContextObject(context)
{
  
}

KRRenderGraph::~KRRenderGraph()
{
}

int KRRenderGraph::addAttachment(const char* name, VkFormat format)
{
  AttachmentInfo& attachment = m_attachments.emplace_back(AttachmentInfo{});
  strncpy(attachment.name, name, RENDER_PASS_ATTACHMENT_NAME_LENGTH);
  attachment.format = format;
  
  return static_cast<int>(m_attachments.size());
}

void KRRenderGraph::addRenderPass(KRDevice& device, const RenderPassInfo& info)
{
  int attachmentCount = 0;
  std::array<VkAttachmentDescription, RENDER_PASS_ATTACHMENT_MAX_COUNT> passAttachments{};
  
  if (info.depthAttachment.id != 0) {
    VkAttachmentDescription& depthAttachment = passAttachments[attachmentCount++];
    
    depthAttachment.format = m_attachments[info.depthAttachment.id - 1].format;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = info.depthAttachment.loadOp;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = info.depthAttachment.stencilLoadOp;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.initialLayout = (info.depthAttachment.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  }
  
  for(int i=0; i<RENDER_PASS_ATTACHMENT_MAX_COUNT; i++) {
    const RenderPassAttachmentInfo& attachmentInfo = info.colorAttachments[i];
    if (attachmentInfo.id == 0) {
      continue;
    }
    
    VkAttachmentDescription& colorAttachment = passAttachments[attachmentCount++];
    colorAttachment.format = m_attachments[attachmentInfo.id - 1].format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = attachmentInfo.loadOp;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = (attachmentInfo.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    if (info.finalPass) {
      colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    } else {
      colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
  }

  VkAttachmentReference depthAttachmentRef{};
  depthAttachmentRef.attachment = 0;
  depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference colorAttachmentRef[RENDER_PASS_ATTACHMENT_MAX_COUNT]{};
  for(int i=0; i < RENDER_PASS_ATTACHMENT_MAX_COUNT; i++) {
    colorAttachmentRef[i].attachment = i;
    colorAttachmentRef[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  }

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = attachmentCount;
  subpass.pColorAttachments = colorAttachmentRef;
  if (info.depthAttachment.id != 0) {
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpass.colorAttachmentCount--;
    subpass.pColorAttachments++;
  } else {
    subpass.pDepthStencilAttachment = nullptr;
  }

  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = attachmentCount;
  renderPassInfo.pAttachments = passAttachments.data();
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;
  
  KRRenderPass *pass = new KRRenderPass(getContext());
  pass->create(device, info, renderPassInfo);
  
  m_renderPasses.push_back(pass);
}

KRRenderPass* KRRenderGraph::getRenderPass(RenderPassType type)
{
  for(KRRenderPass* pass : m_renderPasses) {
    if (pass->getType() == type) {
      return pass;
    }
  }
  return nullptr;
}

KRRenderPass* KRRenderGraph::getFinalRenderPass()
{
  for (KRRenderPass* pass : m_renderPasses) {
    if (pass->isFinal()) {
      return pass;
    }
  }
  return nullptr;
}

void KRRenderGraph::render(VkCommandBuffer &commandBuffer, KRSurface& surface, KRScene* scene)
{
  for(KRRenderPass* pass : m_renderPasses) {
    pass->begin(commandBuffer, surface);
    pass->end(commandBuffer);
  }
}

void KRRenderGraph::destroy(KRDevice& device)
{
  for(KRRenderPass* pass : m_renderPasses) {
    pass->destroy(device);
    delete pass;
  }
  m_renderPasses.clear();
  m_attachments.clear();
}
