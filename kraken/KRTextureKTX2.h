//
//  KRTextureKTX2.h
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

#include "KRTexture2D.h"

class KRTextureKTX2 : public KRTexture2D
{
public:
  KRTextureKTX2(KRContext& context, KRDataBlock* data, std::string name);
  virtual ~KRTextureKTX2();
  virtual std::string getExtension();

  bool uploadTexture(KRDevice& device, VkImage& image, int lod_max_dim, int& current_lod_max_dim, bool premultiply_alpha = false) override;

  virtual long getMemRequiredForSize(int max_dim);
  virtual Vector2i getDimensions() const override;
  virtual int getFaceCount() override;

protected:

  typedef struct
  {
    __uint8_t identifier[12];
    __uint32_t vkFormat;
    __uint32_t typeSize;
    __uint32_t pixelWidth;
    __uint32_t pixelHeight;
    __uint32_t pixelDepth;
    __uint32_t layerCount;
    __uint32_t faceCount;
    __uint32_t levelCount;
    __uint32_t supercompressionScheme;
    // Index 
    __uint32_t dfdByteOffset;
    __uint32_t dfdByteLength;
    __uint32_t kvdByteOffset;
    __uint32_t kvdByteLength;
    __uint32_t sgdByteOffset;
    __uint32_t sgdByteLength;
  } KTX2Header;

  typedef struct
  {
    __uint64_t byteOffset;
    __uint64_t byteLength;
    __uint64_t uncompressedByteLength;
  } KTX2LevelIndex;

  KTX2Header m_header;
};
