//
//  KRSamplerManager.h
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

#pragma once

#include "KREngine-common.h"

#include "KRDataBlock.h"

using std::map;
using std::vector;

#include "KRSampler.h"

class SamplerInfo
{
public:
  VkSamplerCreateInfo createInfo;
  bool operator==(const SamplerInfo& rhs) const;
};

struct SamplerInfoHasher
{
  std::size_t operator()(const SamplerInfo& s) const
  {
    // Compute a hash using the most commonly used sampler fields
    // Collisions are okay, but we need to balance cost of creating
    // hashes with cost of resolving collisions.
    std::size_t h = std::hash<uint32_t>{}(static_cast<uint32_t>((s.createInfo.flags)));
    h ^= std::hash<float>{}(s.createInfo.maxAnisotropy) << 1;
    h ^= std::hash<float>{}(s.createInfo.minLod) << 1;
    h ^= std::hash<float>{}(s.createInfo.maxLod) << 1;
    h ^= std::hash<float>{}(s.createInfo.maxLod) << 1;
    h ^= std::hash<uint32_t>{}(static_cast<uint32_t>((s.createInfo.minFilter))) << 1;
    h ^= std::hash<uint32_t>{}(static_cast<uint32_t>((s.createInfo.magFilter))) << 1;
    return h;
  }
};

class KRSamplerManager : public KRContextObject
{
public:
  KRSamplerManager(KRContext& context);
  virtual ~KRSamplerManager();
  void init();

  KRSampler* getSampler(const SamplerInfo& info);
  void destroy();

  KRSampler* DEFAULT_CLAMPED_SAMPLER;
  KRSampler* DEFAULT_WRAPPING_SAMPLER;
private:
  typedef std::unordered_map<SamplerInfo, KRSampler*, SamplerInfoHasher> SamplerMap;
  SamplerMap m_samplers;
};
