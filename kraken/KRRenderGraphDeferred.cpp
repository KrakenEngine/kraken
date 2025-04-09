//
//  KRRenderGraphDeferred.cpp
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

#include "KRREnderGraphDeferred.h"
#include "KRRenderPass.h"
#include "KRSurface.h"
#include "KRDevice.h"

KRRenderGraphDeferred::KRRenderGraphDeferred(KRContext& context)
  : KRRenderGraph(context)
{
  
}

KRRenderGraphDeferred::~KRRenderGraphDeferred()
{
}

KrResult KRRenderGraphDeferred::initialize(KRSurface &surface)
{
  VkFormat depthImageFormat = VK_FORMAT_UNDEFINED;
  KrResult res = KR_SUCCESS;
  res = surface.getDevice()->selectDepthFormat(depthImageFormat);
  if (res != KR_SUCCESS) {
    return res;
  }

  // ----- Configuration -----
  int shadow_buffer_count = 0;
  bool enable_deferred_lighting = true;
  // -------------------------

  int attachment_compositeDepth = addAttachment("Composite Depth", depthImageFormat);
  int attachment_compositeColor = addAttachment("Composite Color", surface.getSurfaceFormat());
  int attachment_lightAccumulation = addAttachment("Light Accumulation", VK_FORMAT_B8G8R8A8_UINT);
  int attachment_gbuffer = addAttachment("GBuffer", VK_FORMAT_B8G8R8A8_UINT);
  int attachment_shadow_cascades[3];
  attachment_shadow_cascades[0] = addAttachment("Shadow Cascade 0", VK_FORMAT_D32_SFLOAT);
  attachment_shadow_cascades[1] = addAttachment("Shadow Cascade 1", VK_FORMAT_D32_SFLOAT);
  attachment_shadow_cascades[2] = addAttachment("Shadow Cascade 2", VK_FORMAT_D32_SFLOAT);

  RenderPassInfo info{};
  info.finalPass = false;

  info.type = RenderPassType::RENDER_PASS_PRESTREAM;
  addRenderPass(*surface.getDevice(), info);

  for (int shadow_index = 0; shadow_index < shadow_buffer_count; shadow_index++) {
    info.depthAttachment.id = attachment_shadow_cascades[shadow_index];
    info.depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    info.depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    info.depthAttachment.clearVaue.depthStencil.depth = 1.0f;
    info.depthAttachment.clearVaue.depthStencil.stencil = 0;
    info.type = RenderPassType::RENDER_PASS_SHADOWMAP;
    addRenderPass(*surface.getDevice(), info);
  }


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
  addRenderPass(*surface.getDevice(), info);

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
  addRenderPass(*surface.getDevice(), info);

  //  ----====---- Opaque Geometry, Deferred rendering Pass 3 ----====----
  info.colorAttachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  info.colorAttachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
  info.type = RenderPassType::RENDER_PASS_DEFERRED_OPAQUE;
  addRenderPass(*surface.getDevice(), info);

  info.colorAttachments[1] = {};

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
  addRenderPass(*surface.getDevice(), info);

  info.type = RenderPassType::RENDER_PASS_DEBUG_OVERLAYS;
  addRenderPass(*surface.getDevice(), info);

  info.finalPass = true;
  info.type = RenderPassType::RENDER_PASS_POST_COMPOSITE;
  addRenderPass(*surface.getDevice(), info);

  return KR_SUCCESS;
}
