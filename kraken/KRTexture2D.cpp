//
//  KRTexture2D.cpp
//  Kraken Engine
//
//  Copyright 2023 Kearwood Gilbert. All rights reserved.
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

#include "KREngine-common.h"
#include "KRTexture2D.h"
#include "KRTextureManager.h"

KRTexture2D::KRTexture2D(KRContext& context, KRDataBlock* data, std::string name) : KRTexture(context, name)
{
  m_pData = data;
}

KRTexture2D::~KRTexture2D()
{
  delete m_pData;
}

bool KRTexture2D::createGPUTexture(int lod_max_dim)
{
  if (m_haveNewHandles) {
    return true;
  }

  bool success = true;
  int prev_lod_max_dim = m_new_lod_max_dim;
  m_new_lod_max_dim = 0;

  KRDeviceManager* deviceManager = getContext().getDeviceManager();

  for (auto deviceItr = deviceManager->getDevices().begin(); deviceItr != deviceManager->getDevices().end(); deviceItr++) {
    KRDevice& device = *(*deviceItr).second;
    KrDeviceHandle deviceHandle = (*deviceItr).first;
    VmaAllocator allocator = device.getAllocator();
    KRTexture::TextureHandle& texture = m_newHandles.emplace_back();
    texture.device = deviceHandle;
    texture.allocation = VK_NULL_HANDLE;
    texture.image = VK_NULL_HANDLE;

    if (!device.createImage(getDimensions(), 0, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texture.image, &texture.allocation
#if KRENGINE_DEBUG_GPU_LABELS
      , getName().c_str()
#endif
      )) {
      success = false;
      break;
    }

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = texture.image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = getFormat();
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    VkResult res = vkCreateImageView(device.m_logicalDevice, &viewInfo, nullptr, &texture.fullImageView);
    if (res != VK_SUCCESS) {
      success = false;
      break;
    }

    if (!uploadTexture(device, texture.image, lod_max_dim, m_new_lod_max_dim)) {
      success = false;
      break;
    }
  }

  if (success) {
    m_new_lod_max_dim = prev_lod_max_dim;
    m_haveNewHandles = true;
  } else {
    destroyNewHandles();
  }

  return success;
}

bool KRTexture2D::save(const std::string& path)
{
  if (m_pData) {
    return m_pData->save(path);
  } else {
    return false;
  }
}

bool KRTexture2D::save(KRDataBlock& data)
{
  if (m_pData) {
    data.append(*m_pData);
    return true;
  } else {
    return false;
  }
}
