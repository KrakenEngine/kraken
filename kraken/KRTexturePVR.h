//
//  KRTexturePVR.h
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

#pragma once

#include "KRTexture2D.h"

class KRTexturePVR : public KRTexture2D
{
public:
  KRTexturePVR(KRContext& context, KRDataBlock* data, std::string name);
  virtual ~KRTexturePVR();
  virtual std::string getExtension();

  bool uploadTexture(KRDevice& device, VkImage& image, int lod_max_dim, int& current_lod_max_dim, bool premultiply_alpha = false) override;

  virtual long getMemRequiredForSize(int max_dim);
  virtual Vector2i getDimensions() const override;
  virtual VkFormat getFormat() const override;
  virtual int getFaceCount() const override;

protected:

  uint32_t  m_iWidth;
  uint32_t  m_iHeight;
  unsigned int    m_internalFormat;
  bool      m_bHasAlpha;

  std::list<KRDataBlock*> m_blocks;
};
