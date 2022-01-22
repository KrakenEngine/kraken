//
//  KRDevice.h
//  Kraken Engine
//
//  Copyright 2021 Kearwood Gilbert. All rights reserved.
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
#include "KRContextObject.h"

#ifndef KRDEVICE_H
#define KRDEVICE_H

class KRDevice : public KRContextObject
{
public:
  KRDevice(KRContext& context, const VkPhysicalDevice& device);
  virtual ~KRDevice();

  KRDevice(const KRDevice&) = delete;
  KRDevice& operator=(const KRDevice&) = delete;

  void destroy();
  bool initialize(const std::vector<const char*>& deviceExtensions);

  VmaAllocator getAllocator();

  VkPhysicalDevice m_device;
  VkDevice m_logicalDevice;
  VkPhysicalDeviceProperties m_deviceProperties;
  VkPhysicalDeviceFeatures m_deviceFeatures;
  uint32_t m_graphicsFamilyQueueIndex;
  VkQueue m_graphicsQueue;
  uint32_t m_computeFamilyQueueIndex;
  VkQueue m_computeQueue;
  VkCommandPool m_graphicsCommandPool;
  VkCommandPool m_computeCommandPool;
  std::vector<VkCommandBuffer> m_graphicsCommandBuffers;
  std::vector<VkCommandBuffer> m_computeCommandBuffers;
  VmaAllocator m_allocator;
private:
};

#endif // KRDEVICE_H
