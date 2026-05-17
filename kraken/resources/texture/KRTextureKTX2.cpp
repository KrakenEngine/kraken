//
//  KRTextureKTX2.cpp
//  Kraken Engine
//
//  Copyright 2026 Kearwood Gilbert. All rights reserved.
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

#include "KRTextureKTX2.h"
#include "KRTextureManager.h"

#include "KREngine-common.h"

using namespace mimir;
using namespace hydra;

__uint8_t _KTX2FileIdentifier[12] = {
    0xAB, 0x4B, 0x54, 0x58, 0x20, 0x32, 0x30, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A
};

KRTextureKTX2::KRTextureKTX2(KRContext& context, Block* data, std::string name) : KRTexture2D(context, data, name)
{
  m_pData->copy(&m_header, 0, sizeof(KTX2Header));
  if (memcmp(_KTX2FileIdentifier, m_header.identifier, 12) != 0) {
    assert(false); // Header not recognized
  }
  if (m_header.pixelDepth != 0) {
    assert(false); // 3d textures not (yet) supported
  }
  if (m_header.layerCount != 0) {
    assert(false); // Array textures not (yet) supported
  }
  if (m_header.faceCount != 1 && m_header.faceCount != 6) {
    assert(false);
  }
  if (m_header.supercompressionScheme != 0) {
    assert(false); // Not yet supported
  }

  uint32_t width = m_header.pixelWidth >> m_header.levelCount;
  uint32_t height = m_header.pixelHeight >> m_header.levelCount;
  if (width < 1) {
    width = 1;
  }
  if (height < 1) {
    height = 1;
  }
  m_lod_count = (int)KRMAX(m_header.levelCount, 1);
}

KRTextureKTX2::~KRTextureKTX2()
{
}

Vector3i KRTextureKTX2::getDimensions() const
{
  return Vector3i::Create(Vector3i::Create(m_header.pixelWidth, m_header.pixelHeight, m_header.pixelDepth));
}

long KRTextureKTX2::getMemRequiredForLod(int lod)
{
  int target_lod = KRMIN(lod, m_lod_count - 1);

  KTX2LevelIndex levelIndex;
  m_pData->copy(&levelIndex, sizeof(m_header) + sizeof(KTX2LevelIndex) * target_lod, sizeof(KTX2LevelIndex));

  return (long)levelIndex.byteLength;
}

bool KRTextureKTX2::getLodData(void* buffer, int lod)
{
  unsigned char* converted_image = (unsigned char*)buffer;
  int target_lod = KRMIN(lod, m_lod_count - 1);

  KTX2LevelIndex levelIndex;
  m_pData->copy(&levelIndex, sizeof(m_header) + sizeof(KTX2LevelIndex) * target_lod, sizeof(KTX2LevelIndex));

  // TODO - Implement copy of buffer data
  assert(false);

  return true;
}

std::string KRTextureKTX2::getExtension()
{
  return "ktx2";
}


int KRTextureKTX2::getFaceCount() const
{
  return m_header.faceCount;
}


VkFormat KRTextureKTX2::getFormat() const
{
  // TODO - Implement
  return VK_FORMAT_UNDEFINED;
}