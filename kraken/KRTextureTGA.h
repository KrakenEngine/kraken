//
//  KRTextureTGA.h
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

#include "KRTexture2D.h"

using namespace mimir;

class KRTextureTGA : public KRTexture2D
{
public:
  KRTextureTGA(KRContext& context, Block* data, std::string name);
  virtual ~KRTextureTGA();
  virtual std::string getExtension() override;

  bool uploadTexture(KRDevice& device, VkImage& image, int lod_max_dim, int& current_lod_max_dim, bool premultiply_alpha = false) override;

#if !TARGET_OS_IPHONE && !defined(ANDROID)
  virtual KRTexture* compress(bool premultiply_alpha = false) override;
#endif

  virtual long getMemRequiredForSize(int max_dim) override;
  virtual hydra::Vector2i getDimensions() const override;
  virtual VkFormat getFormat() const override;
  virtual int getFaceCount() const override;
private:
  long m_imageSize;
  hydra::Vector2i m_dimensions;
};
