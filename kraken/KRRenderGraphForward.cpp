//
//  KRRenderGraphForward.cpp
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

#include "KRREnderGraphForward.h"
#include "KRRenderPass.h"
#include "KRSurface.h"
#include "KRDevice.h"

KRRenderGraphForward::KRRenderGraphForward(KRContext& context)
  : KRRenderGraph(context)
{
  
}

KRRenderGraphForward::~KRRenderGraphForward()
{
}

KrResult KRRenderGraphForward::initialize(KRSurface &surface)
{
  VkFormat depthImageFormat = VK_FORMAT_UNDEFINED;
  KrResult res = KR_SUCCESS;
  res = surface.getDevice()->selectDepthFormat(depthImageFormat);
  if (res != KR_SUCCESS) {
    return res;
  }

  // ----- Configuration -----
  int shadow_buffer_count = 0;
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
#if KRENGINE_DEBUG_GPU_LABELS
  strncpy(info.debugLabel, "PreStream", KRENGINE_DEBUG_GPU_LABEL_MAX_LEN);
#endif
  addRenderPass(*surface.getDevice(), info);

  for (int shadow_index = 0; shadow_index < shadow_buffer_count; shadow_index++) {
    info.depthAttachment.id = attachment_shadow_cascades[shadow_index];
    info.depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    info.depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    info.depthAttachment.clearVaue.depthStencil.depth = 1.0f;
    info.depthAttachment.clearVaue.depthStencil.stencil = 0;
    info.type = RenderPassType::RENDER_PASS_SHADOWMAP;
#if KRENGINE_DEBUG_GPU_LABELS
    snprintf(info.debugLabel, KRENGINE_DEBUG_GPU_LABEL_MAX_LEN, "Shadow Map %i", shadow_index);
#endif
    addRenderPass(*surface.getDevice(), info);
  }

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
#if KRENGINE_DEBUG_GPU_LABELS
  strncpy(info.debugLabel, "Forward Opaque", KRENGINE_DEBUG_GPU_LABEL_MAX_LEN);
#endif
  addRenderPass(*surface.getDevice(), info);

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
#if KRENGINE_DEBUG_GPU_LABELS
  strncpy(info.debugLabel, "Forward Transparent", KRENGINE_DEBUG_GPU_LABEL_MAX_LEN);
#endif
  addRenderPass(*surface.getDevice(), info);

  info.type = RenderPassType::RENDER_PASS_PARTICLE_OCCLUSION;
#if KRENGINE_DEBUG_GPU_LABELS
  strncpy(info.debugLabel, "Particle Occlusion", KRENGINE_DEBUG_GPU_LABEL_MAX_LEN);
#endif
  addRenderPass(*surface.getDevice(), info);

  info.type = RenderPassType::RENDER_PASS_ADDITIVE_PARTICLES;
#if KRENGINE_DEBUG_GPU_LABELS
  strncpy(info.debugLabel, "Additive Particles", KRENGINE_DEBUG_GPU_LABEL_MAX_LEN);
#endif
  addRenderPass(*surface.getDevice(), info);

  info.type = RenderPassType::RENDER_PASS_VOLUMETRIC_EFFECTS_ADDITIVE;
#if KRENGINE_DEBUG_GPU_LABELS
  strncpy(info.debugLabel, "Additive Volumetric Effects", KRENGINE_DEBUG_GPU_LABEL_MAX_LEN);
#endif
  addRenderPass(*surface.getDevice(), info);

  info.type = RenderPassType::RENDER_PASS_DEBUG_OVERLAYS;
#if KRENGINE_DEBUG_GPU_LABELS
  strncpy(info.debugLabel, "Debug Overlays", KRENGINE_DEBUG_GPU_LABEL_MAX_LEN);
#endif
  addRenderPass(*surface.getDevice(), info);

  info.finalPass = true;
  info.type = RenderPassType::RENDER_PASS_POST_COMPOSITE;
#if KRENGINE_DEBUG_GPU_LABELS
  strncpy(info.debugLabel, "Post Composite", KRENGINE_DEBUG_GPU_LABEL_MAX_LEN);
#endif
  addRenderPass(*surface.getDevice(), info);

  return KR_SUCCESS;
}
