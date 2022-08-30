//
//  KRSampler.cpp
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

#include "KRSampler.h"
#include "KRSamplerManager.h"
#include "KRDeviceManager.h"

KRSampler::KRSampler(KRContext& context)
  : KRContextObject(context)
{
  // TODO - Implement stub function
}

KRSampler::~KRSampler()
{
  destroy();
}

bool KRSampler::createSamplers(const SamplerInfo& info)
{
  bool success = true;
  m_samplers.clear();
  KRDeviceManager* deviceManager = getContext().getDeviceManager();
  int iAllocation = 0;

  for (auto deviceItr = deviceManager->getDevices().begin(); deviceItr != deviceManager->getDevices().end() && iAllocation < KRENGINE_MAX_GPU_COUNT; deviceItr++, iAllocation++) {
    KRDevice& device = *(*deviceItr).second;
    VkSampler sampler = VK_NULL_HANDLE;
    if (vkCreateSampler(device.m_logicalDevice, &info.createInfo, nullptr, &sampler) != VK_SUCCESS) {
      success = false;
      break;
    }
    m_samplers.push_back(std::make_pair(deviceItr->first, sampler));
  }

  if (!success) {
    destroy();
  }

  return success;
}

VkSampler KRSampler::getSampler(KrDeviceHandle& handle)
{
  for (std::pair<KrDeviceHandle, VkSampler> sampler : m_samplers) {
    if (sampler.first == handle) {
      return sampler.second;
    }
  }
  // TODO - Handle device context loss
  assert(false);
  return VK_NULL_HANDLE;
}

void KRSampler::destroy()
{

}