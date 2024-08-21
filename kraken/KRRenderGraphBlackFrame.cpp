//
//  KRRenderGraphBlackFrame.cpp
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

#include "KRREnderGraphBlackFrame.h"
#include "KRRenderPass.h"
#include "KRSurface.h"
#include "KRDevice.h"

KRRenderGraphBlackFrame::KRRenderGraphBlackFrame(KRContext& context)
  : KRRenderGraph(context)
{
  
}

KRRenderGraphBlackFrame::~KRRenderGraphBlackFrame()
{
}

KrResult KRRenderGraphBlackFrame::initialize(KRSurface &surface)
{
  VkFormat depthImageFormat = VK_FORMAT_UNDEFINED;
  KrResult res = KR_SUCCESS;
  res = surface.getDevice()->selectDepthFormat(depthImageFormat);
  if (res != KR_SUCCESS) {
    return res;
  }
  
  int attachment_blackFrameDepth = addAttachment("Composite Depth", depthImageFormat);
  int attachment_blackFrameColor = addAttachment("Composite Color", surface.getSurfaceFormat());
  
  RenderPassInfo info{};
  info.colorAttachments[0].id = attachment_blackFrameColor;
  info.colorAttachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  info.depthAttachment.id = attachment_blackFrameDepth;
  info.depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  info.finalPass = true;
  info.type = RenderPassType::RENDER_PASS_BLACK_FRAME;
  addRenderPass(*surface.getDevice(), info);

  return KR_SUCCESS;
}
