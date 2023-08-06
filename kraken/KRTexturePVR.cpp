//
//  KRTexturePVR.cpp
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

#include "KRTexturePVR.h"
#include "KRTextureManager.h"

#include "KREngine-common.h"

using namespace mimir;
using namespace hydra;


#define PVR_TEXTURE_FLAG_TYPE_MASK	0xff

static char gPVRTexIdentifier[5] = "PVR!";

enum
{
  kPVRTextureFlagTypePVRTC_2 = 24,
  kPVRTextureFlagTypePVRTC_4
};

typedef struct _PVRTexHeader
{
  uint32_t headerLength;
  uint32_t height;
  uint32_t width;
  uint32_t numMipmaps;
  uint32_t flags;
  uint32_t dataLength;
  uint32_t bpp;
  uint32_t bitmaskRed;
  uint32_t bitmaskGreen;
  uint32_t bitmaskBlue;
  uint32_t bitmaskAlpha;
  uint32_t pvrTag;
  uint32_t numSurfs;
} PVRTexHeader;

KRTexturePVR::KRTexturePVR(KRContext& context, Block* data, std::string name) : KRTexture2D(context, data, name)
{
#if TARGET_OS_IPHONE

  PVRTexHeader header;
  m_pData->copy(&header, 0, sizeof(PVRTexHeader));

  uint32_t formatFlags = header.flags & PVR_TEXTURE_FLAG_TYPE_MASK;
  if (formatFlags == kPVRTextureFlagTypePVRTC_4) {
    m_internalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
  } else if (formatFlags == kPVRTextureFlagTypePVRTC_2) {
    m_internalFormat = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
  } else {
    assert(false);
  }

  uint32_t pvrTag = header.pvrTag;
  if (gPVRTexIdentifier[0] != ((pvrTag >> 0) & 0xff) ||
      gPVRTexIdentifier[1] != ((pvrTag >> 8) & 0xff) ||
      gPVRTexIdentifier[2] != ((pvrTag >> 16) & 0xff) ||
      gPVRTexIdentifier[3] != ((pvrTag >> 24) & 0xff)) {
    assert(false);
  }

  m_iWidth = header.width; // Note: call __builtin_bswap32 when needed to switch endianness
  m_iHeight = header.height;
  m_bHasAlpha = header.bitmaskAlpha;

  uint32_t dataStart = sizeof(PVRTexHeader);
  uint32_t dataLength = header.dataLength, dataOffset = 0, dataSize = 0;
  uint32_t width = m_iWidth, height = m_iHeight, bpp = 4;
  uint32_t blockSize = 0, widthBlocks = 0, heightBlocks = 0;

  // Calculate the data size for each texture level and respect the minimum number of blocks
  while (dataOffset < dataLength) {
    if (formatFlags == kPVRTextureFlagTypePVRTC_4) {
      blockSize = 4 * 4; // Pixel by pixel block size for 4bpp
      widthBlocks = width / 4;
      heightBlocks = height / 4;
      bpp = 4;
    } else {
      blockSize = 8 * 4; // Pixel by pixel block size for 2bpp
      widthBlocks = width / 8;
      heightBlocks = height / 4;
      bpp = 2;
    }

    // Clamp to minimum number of blocks
    if (widthBlocks < 2) {
      widthBlocks = 2;
    }
    if (heightBlocks < 2) {
      heightBlocks = 2;
    }
    dataSize = widthBlocks * heightBlocks * ((blockSize * bpp) / 8);

    m_blocks.push_back(m_pData->getSubBlock(dataStart + dataOffset, dataSize));

    dataOffset += dataSize;

    width = width >> 1;
    if (width < 1) {
      width = 1;
    }
    height = height >> 1;
    if (height < 1) {
      height = 1;
    }
  }

  m_max_lod_max_dim = m_iWidth > m_iHeight ? m_iWidth : m_iHeight;
  m_min_lod_max_dim = width > height ? width : height;
#endif
}

KRTexturePVR::~KRTexturePVR()
{
  for (std::list<Block*>::iterator itr = m_blocks.begin(); itr != m_blocks.end(); itr++) {
    Block* block = *itr;
    delete block;
  }
  m_blocks.clear();
}

Vector2i KRTexturePVR::getDimensions() const
{
  return Vector2i::Create(m_iWidth, m_iHeight);
}

VkFormat KRTexturePVR::getFormat() const
{
  PVRTexHeader header;
  m_pData->copy(&header, 0, sizeof(PVRTexHeader));

  uint32_t formatFlags = header.flags & PVR_TEXTURE_FLAG_TYPE_MASK;
  switch (formatFlags) {
  case kPVRTextureFlagTypePVRTC_2:
    return VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG;
  case kPVRTextureFlagTypePVRTC_4:
    return VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG;
    break;
  default:
    return VK_FORMAT_UNDEFINED;
    break;
  }
}

long KRTexturePVR::getMemRequiredForSize(int max_dim)
{
  int target_dim = max_dim;
  if (target_dim < (int)m_min_lod_max_dim) target_dim = target_dim;

  // Determine how much memory will be consumed
  int width = m_iWidth;
  int height = m_iHeight;
  long memoryRequired = 0;

  for (std::list<Block*>::iterator itr = m_blocks.begin(); itr != m_blocks.end(); itr++) {
    Block* block = *itr;
    if (width <= target_dim && height <= target_dim) {
      memoryRequired += (long)block->getSize();
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

bool KRTexturePVR::uploadTexture(KRDevice& device, VkImage& image, int lod_max_dim, int& current_lod_max_dim, bool premultiply_alpha)
{
  int target_dim = lod_max_dim;
  if (target_dim < (int)m_min_lod_max_dim) target_dim = m_min_lod_max_dim;

  if (m_blocks.size() == 0) {
    return false;
  }

  // Determine how much memory will be consumed
  int width = m_iWidth;
  int height = m_iHeight;
  long memoryRequired = 0;
  long memoryTransferred = 0;

  // Upload texture data
  int destination_level = 0;
  int source_level = 0;
  for (std::list<Block*>::iterator itr = m_blocks.begin(); itr != m_blocks.end(); itr++) {
    Block* block = *itr;
    if (width <= target_dim && height <= target_dim) {

      if (width > current_lod_max_dim) {
        current_lod_max_dim = width;
      }
      if (height > current_lod_max_dim) {
        current_lod_max_dim = height;
      }

      block->lock();
      /*
       * TODO - Vulkan Refactoring
      GLDEBUG(glCompressedTexImage2D(target, destination_level, m_internalFormat, width, height, 0, (int)block->getSize(), block->getStart()));
      */
      block->unlock();
      memoryTransferred += (long)block->getSize(); // memoryTransferred does not include throughput of mipmap levels copied through glCopyTextureLevelsAPPLE
      memoryRequired += (long)block->getSize();
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

std::string KRTexturePVR::getExtension()
{
  return "pvr";
}

int KRTexturePVR::getFaceCount() const
{
  return 1;
}
