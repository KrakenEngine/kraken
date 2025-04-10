//
//  KRRenderGraph.h
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

#include "KRContextObject.h"
#include "KRRenderPass.h"

class KRRenderPass;
class KRSurface;
class KRDevice;
struct RenderPassInfo;
enum RenderPassType : uint8_t;

#define RENDER_PASS_ATTACHMENT_NAME_LENGTH 64

class KRRenderGraph : public KRContextObject
{
public:
  KRRenderGraph(KRContext& context);
  ~KRRenderGraph();
  
  int addAttachment(const char* name, VkFormat format);
  void addRenderPass(KRDevice& device, const RenderPassInfo& info);
  KRRenderPass* getRenderPass(RenderPassType type);
  KRRenderPass* getFinalRenderPass();
  void render(VkCommandBuffer &commandBuffer, KRSurface& surface, KRCamera* camera);
  void destroy(KRDevice& device);
    
private:
  
  struct AttachmentInfo
  {
    char name[RENDER_PASS_ATTACHMENT_NAME_LENGTH];
    VkFormat format;
  };
  
  std::vector<AttachmentInfo> m_attachments;
  std::vector<KRRenderPass*> m_renderPasses;

};
