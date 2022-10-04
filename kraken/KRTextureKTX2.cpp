//
//  KRTextureKTX2.cpp
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

#include "KRTextureKTX2.h"
#include "KRTextureManager.h"

#include "KREngine-common.h"

__uint8_t _KTX2FileIdentifier[12] = {
    0xAB, 0x4B, 0x54, 0x58, 0x20, 0x32, 0x30, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A
};

KRTextureKTX2::KRTextureKTX2(KRContext& context, KRDataBlock* data, std::string name) : KRTexture2D(context, data, name)
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
  m_max_lod_max_dim = KRMAX(m_header.pixelWidth, m_header.pixelHeight);
  m_min_lod_max_dim = KRMAX(width, height);
}

KRTextureKTX2::~KRTextureKTX2()
{
}

Vector2i KRTextureKTX2::getDimensions() const
{
  return Vector2i::Create(Vector2i::Create(m_header.pixelWidth, m_header.pixelHeight));
}

long KRTextureKTX2::getMemRequiredForSize(int max_dim)
{
  int target_dim = max_dim;
  if (target_dim < (int)m_min_lod_max_dim) target_dim = target_dim;

  // Determine how much memory will be consumed

  int width = m_header.pixelWidth;
  int height = m_header.pixelHeight;
  long memoryRequired = 0;

  for (__uint32_t level = 0; level < m_header.levelCount; level++) {
    KTX2LevelIndex levelIndex;
    m_pData->copy(&levelIndex, sizeof(m_header) + sizeof(KTX2LevelIndex) * level, sizeof(KTX2LevelIndex));
    if (width <= target_dim && height <= target_dim) {
      memoryRequired += (long)levelIndex.byteLength;
    }

    width = width >> 1;
    if (width < 1) {
      width = 1;
    }
    height = height >> 1;
    if (height < 1) {
      height = 1;
    }
  }

  return memoryRequired;
}

bool KRTextureKTX2::uploadTexture(KRDevice& device, VkImage& image, int lod_max_dim, int& current_lod_max_dim, bool premultiply_alpha)
{
  int target_dim = lod_max_dim;
  if (target_dim < (int)m_min_lod_max_dim) target_dim = m_min_lod_max_dim;

  // Determine how much memory will be consumed
  int width = m_header.pixelWidth;
  int height = m_header.pixelHeight;
  long memoryRequired = 0;
  long memoryTransferred = 0;


  // Upload texture data
  int destination_level = 0;
  int source_level = 0;

  for (__uint32_t level = 0; level < m_header.levelCount; level++) {
    KTX2LevelIndex levelIndex;
    m_pData->copy(&levelIndex, sizeof(m_header) + sizeof(KTX2LevelIndex) * level, sizeof(KTX2LevelIndex));

    if (width <= target_dim && height <= target_dim) {

      if (width > current_lod_max_dim) {
        current_lod_max_dim = width;
      }
      if (height > current_lod_max_dim) {
        current_lod_max_dim = height;
      }

      /*
      * TODO - Vulkan Refactoring
      GLDEBUG(glCompressedTexImage2D(target, destination_level, (unsigned int)m_header.glInternalFormat, width, height, 0, (int)block->getSize(), block->getStart()));
      */

      memoryTransferred += (long)levelIndex.byteLength; // memoryTransferred does not include throughput of mipmap levels copied through glCopyTextureLevelsAPPLE
      memoryRequired += (long)levelIndex.byteLength;
      //
      //            err = glGetError();
      //            if (err != GL_NO_ERROR) {
      //                assert(false);
      //                return false;
      //            }
      //

      destination_level++;
    }

    if (width <= m_current_lod_max_dim && height <= m_current_lod_max_dim) {
      source_level++;
    }

    width = width >> 1;
    if (width < 1) {
      width = 1;
    }
    height = height >> 1;
    if (height < 1) {
      height = 1;
    }
  }

  return true;

}

std::string KRTextureKTX2::getExtension()
{
  return "ktx2";
}
