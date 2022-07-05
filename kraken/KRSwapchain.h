//
//  KRSwapchain.h
//  Kraken Engine
//
//  Copyright 2022 Kearwood Gilbert. All rights reserved.
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



#ifndef KRSWAPCHAIN_H
#define KRSWAPCHAIN_H

#include "KREngine-common.h"
#include "KRContext.h"

class KRRenderPass;

class KRSwapchain : public KRContextObject
{
public:
  KRSwapchain(KRContext& context);
  ~KRSwapchain();

  KrResult create(KRDevice& device, VkSurfaceKHR& surface, VkSurfaceFormatKHR& surfaceFormat, VkFormat depthFormat, VkExtent2D& extent, uint32_t imageCount, const KRRenderPass& renderPass);
  void destroy(KRDevice& device);

  VkSwapchainKHR m_swapChain;
  VkExtent2D m_extent;
  VkFormat m_imageFormat;
  VkFormat m_depthFormat;

  VkImage m_depthImage;
  VmaAllocation m_depthImageAllocation;
  VkImageView m_depthImageView;

  std::vector<VkImage> m_images;
  std::vector<VkImageView> m_imageViews;
  std::vector<VkFramebuffer> m_framebuffers;

};

#endif KRSWAPCHAIN_H