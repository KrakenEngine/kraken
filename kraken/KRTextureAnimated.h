//
//  KRTextureAnimated.h
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

#include "KRTexture.h"
#include "KRTexture2D.h"

class KRTextureAnimated : public KRTexture
{
public:
  KRTextureAnimated(KRContext& context, std::string name);
  virtual ~KRTextureAnimated();
  virtual std::string getExtension();
  virtual bool save(const std::string& path);
  virtual bool save(KRDataBlock& data);

  virtual void bind(GLuint texture_unit);
  virtual long getMemRequiredForSize(int max_dim);
  virtual void resetPoolExpiry(float lodCoverage, texture_usage_t textureUsage);

  virtual long getReferencedMemSize();

  virtual bool isAnimated();
  virtual void resize(int max_dim);

private:
  bool createGPUTexture(int lod_max_dim) override;

  float m_frame_rate;
  int m_frame_count;

  std::string m_texture_base_name;
  std::string textureNameForFrame(int frame);
  KRTexture2D* textureForFrame(int frame);
};
