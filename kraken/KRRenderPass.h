//
//  KRScene.h
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

class KRSurface;

enum RenderPassType : uint8_t
{
  RENDER_PASS_FORWARD_OPAQUE,
  RENDER_PASS_DEFERRED_GBUFFER,
  RENDER_PASS_DEFERRED_LIGHTS,
  RENDER_PASS_DEFERRED_OPAQUE,
  RENDER_PASS_FORWARD_TRANSPARENT,
  RENDER_PASS_PARTICLE_OCCLUSION,
  RENDER_PASS_ADDITIVE_PARTICLES,
  RENDER_PASS_VOLUMETRIC_EFFECTS_ADDITIVE,
  RENDER_PASS_SHADOWMAP,
  RENDER_PASS_PRESTREAM,
  RENDER_PASS_POST_COMPOSITE,
  RENDER_PASS_DEBUG_OVERLAYS,
  RENDER_PASS_BLACK_FRAME,
};

#define RENDER_PASS_ATTACHMENT_MAX_COUNT 16

struct RenderPassAttachmentInfo
{
  int id;
  VkAttachmentLoadOp loadOp;
  VkAttachmentLoadOp stencilLoadOp;
  VkClearValue clearVaue;
};

struct RenderPassInfo
{
  RenderPassType type;
  RenderPassAttachmentInfo colorAttachments[RENDER_PASS_ATTACHMENT_MAX_COUNT];
  RenderPassAttachmentInfo depthAttachment;
#if KRENGINE_DEBUG_GPU_LABELS
  char debugLabel[KRENGINE_DEBUG_GPU_LABEL_MAX_LEN];
#endif
  
  bool finalPass;
};

class KRRenderPass : public KRContextObject
{
public:
  
  KRRenderPass(KRContext& context);
  ~KRRenderPass();

  void create(KRDevice& device, const RenderPassInfo& m_info, const VkRenderPassCreateInfo& info);
  void destroy(KRDevice& device);

  void begin(VkCommandBuffer& commandBuffer, KRSurface& surface);
  void end(VkCommandBuffer& commandBuffer);
  
  RenderPassType getType() const;
  bool isFinal() const;

  // private:
  VkRenderPass m_renderPass;
  RenderPassInfo m_info;
};
