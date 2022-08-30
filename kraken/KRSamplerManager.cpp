//
//  KRSamplerManager.cpp
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

#include "KREngine-common.h"

#include "KRSamplerManager.h"

// TODO - Purge samplers that are not used for several frames

bool SamplerInfo::operator==(const SamplerInfo& rhs) const
{
  assert(rhs.createInfo.pNext == nullptr);
  assert(createInfo.pNext == nullptr);
  // Compare the contents of the SamplerInfo struct, ignoring the first two members (sType, pNext)
  return memcmp(&rhs.createInfo.flags, &createInfo.flags, sizeof(createInfo) - size_t(&createInfo.flags) - size_t(&createInfo));
}

KRSamplerManager::KRSamplerManager(KRContext& context)
  : KRContextObject(context)
  , DEFAULT_CLAMPED_SAMPLER(nullptr)
  , DEFAULT_WRAPPING_SAMPLER(nullptr)
{
}

KRSamplerManager::~KRSamplerManager()
{
  
}

void KRSamplerManager::destroy()
{
  for (SamplerMap::iterator itr = m_samplers.begin(); itr != m_samplers.end(); ++itr) {
    delete (*itr).second;
  }
  m_samplers.clear();
}

void KRSamplerManager::init()
{
  SamplerInfo info{};
  info.createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  info.createInfo.magFilter = VK_FILTER_LINEAR;
  info.createInfo.minFilter = VK_FILTER_LINEAR;
  info.createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  info.createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  info.createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  info.createInfo.anisotropyEnable = VK_TRUE;
  info.createInfo.maxAnisotropy = 16; // TODO - This should be dynamic
  info.createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  info.createInfo.unnormalizedCoordinates = VK_FALSE;
  info.createInfo.compareEnable = VK_FALSE;
  info.createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  info.createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  info.createInfo.mipLodBias = 0.0f;
  info.createInfo.minLod = 0.0f;
  info.createInfo.maxLod = 0.0f;

  DEFAULT_CLAMPED_SAMPLER = getSampler(info);

  info.createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  info.createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  info.createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

  DEFAULT_WRAPPING_SAMPLER = getSampler(info);
}

KRSampler* KRSamplerManager::getSampler(const SamplerInfo& info)
{
  SamplerMap::iterator itr = m_samplers.find(info);
  if (itr != m_samplers.end()) {
    return itr->second;
  }
  KRSampler* sampler = new KRSampler(getContext());
  sampler->createSamplers(info);
  m_samplers[info] = sampler;
  return sampler;
}
