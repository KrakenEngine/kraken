//
//  KRTextureKTX.cpp
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

#include "KRTextureKTX.h"
#include "KRTextureManager.h"

#include "KREngine-common.h"

using namespace hydra;

__uint8_t _KTXFileIdentifier[12] = {
    0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A
};

KRTextureKTX::KRTextureKTX(KRContext& context, Block* data, std::string name) : KRTexture2D(context, data, name)
{
  m_pData->copy(&m_header, 0, sizeof(KTXHeader));
  if (memcmp(_KTXFileIdentifier, m_header.identifier, 12) != 0) {
    assert(false); // Header not recognized
  }
  if (m_header.endianness != 0x04030201) {
    assert(false); // Endianness not (yet) supported
  }
  if (m_header.pixelDepth != 0) {
    assert(false); // 3d textures not (yet) supported
  }
  if (m_header.numberOfArrayElements != 0) {
    assert(false); // Array textures not (yet) supported
  }
  if (m_header.numberOfFaces != 1) {
    assert(false); // Cube-map textures are only supported as a file for each separate face (for now)
  }

  uint32_t blockStart = sizeof(KTXHeader) + m_header.bytesOfKeyValueData;
  uint32_t width = m_header.pixelWidth, height = m_header.pixelHeight;

  for (int mipmap_level = 0; mipmap_level < (int)KRMAX(m_header.numberOfMipmapLevels, 1); mipmap_level++) {
    uint32_t blockLength;
    data->copy(&blockLength, blockStart, 4);
    blockStart += 4;

    m_blocks.push_back(m_pData->getSubBlock(blockStart, blockLength));

    blockStart += blockLength;
    blockStart = KRALIGN(blockStart);

    width = width >> 1;
    if (width < 1) {
      width = 1;
    }
    height = height >> 1;
    if (height < 1) {
      height = 1;
    }
  }

  m_max_lod_max_dim = KRMAX(m_header.pixelWidth, m_header.pixelHeight);
  m_min_lod_max_dim = KRMAX(width, height);
}

KRTextureKTX::KRTextureKTX(KRContext& context, std::string name, unsigned int internal_format, unsigned int base_internal_format, int width, int height, const std::list<Block*>& blocks) : KRTexture2D(context, new Block(), name)
{
  memcpy(m_header.identifier, _KTXFileIdentifier, 12);
  m_header.endianness = 0x04030201;
  m_header.glType = 0;
  m_header.glTypeSize = 1;
  m_header.glFormat = 0;
  m_header.glInternalFormat = internal_format;
  m_header.glBaseInternalFormat = base_internal_format;
  m_header.pixelWidth = width;
  m_header.pixelHeight = height;
  m_header.pixelDepth = 0;
  m_header.numberOfArrayElements = 0;
  m_header.numberOfFaces = 1;
  m_header.numberOfMipmapLevels = (__uint32_t)blocks.size();
  m_header.bytesOfKeyValueData = 0;

  m_pData->append(&m_header, sizeof(m_header));
  for (auto block_itr = blocks.begin(); block_itr != blocks.end(); block_itr++) {
    Block* source_block = *block_itr;
    __uint32_t block_size = (__uint32_t)source_block->getSize();
    m_pData->append(&block_size, 4);
    m_pData->append(*source_block);
    m_blocks.push_back(m_pData->getSubBlock((int)m_pData->getSize() - (int)block_size, (int)block_size));
    size_t alignment_padding_size = KRALIGN(m_pData->getSize()) - m_pData->getSize();
    __uint8_t alignment_padding[4] = { 0, 0, 0, 0 };
    if (alignment_padding_size > 0) {
      m_pData->append(&alignment_padding, alignment_padding_size);
    }
  }
}

KRTextureKTX::~KRTextureKTX()
{
  for (std::list<Block*>::iterator itr = m_blocks.begin(); itr != m_blocks.end(); itr++) {
    Block* block = *itr;
    delete block;
  }
  m_blocks.clear();
}

Vector2i KRTextureKTX::getDimensions() const
{
  return hydra::Vector2i::Create(Vector2i::Create(m_header.pixelWidth, m_header.pixelHeight));
}

int KRTextureKTX::getFaceCount() const
{
  return m_header.numberOfFaces;
}

VkFormat KRTextureKTX::getFormat() const
{
  if (m_header.glFormat != 0) {
    // Non-Compressed formats, from table 8.3 of OpenGL 4.4 spec:
    switch (m_header.glFormat) {
    case 0x1901: // GL_STENCIL_INDEX
    case 0x1902: // GL_DEPTH_COMPONENT
    case 0x84F9: // GL_DEPTH_STENCIL
      return VK_FORMAT_UNDEFINED;
    case 0x1903: // GL_RED
    case 0x1904: // GL_GREEN
    case 0x1905: // GL_BLUE
    case 0x8D94: // GL_RED_INTEGER
    case 0x8D95: // GL_GREEN_INTEGER
    case 0x8D96: // GL_BLUE_INTEGER
      // Types applicable to a single component from table 8.2 of OpenGL 4.4 spec
      switch (m_header.glType) {
      case 0x1401: // UNSIGNED_BYTE
        return VK_FORMAT_R8_UNORM;
      case 0x1400: // BYTE
        return VK_FORMAT_R8_SNORM;
      case 0x1403: // UNSIGNED_SHORT
        return VK_FORMAT_R16_UNORM;
      case 0x1402: // SHORT
        return VK_FORMAT_R16_SNORM;
      case 0x1405: // UNSIGNED_INT
        return VK_FORMAT_R32_UINT;
      case 0x1404: // INT
        return VK_FORMAT_R32_SINT;
      case 0x140B: // HALF_FLOAT
        return VK_FORMAT_R16_SFLOAT;
      case 0x1406: // FLOAT
        return VK_FORMAT_R32_SFLOAT;
      default:
        return VK_FORMAT_UNDEFINED;
      }
    case 0x8227: // GL_RG
    case 0x8228: // GL_RG_INTEGER
      // Types applicable to two components from table 8.2 of OpenGL 4.4 spec
      switch (m_header.glType) {
      case 0x1401: // UNSIGNED_BYTE
        return VK_FORMAT_R8G8_UNORM;
      case 0x1400: // BYTE
        return VK_FORMAT_R8G8_SNORM;
      case 0x1403: // UNSIGNED_SHORT
        return VK_FORMAT_R16G16_UNORM;
      case 0x1402: // SHORT
        return VK_FORMAT_R16G16_SNORM;
      case 0x1405: // UNSIGNED_INT
        return VK_FORMAT_R32G32_UINT;
      case 0x1404: // INT
        return VK_FORMAT_R32G32_SINT;
      case 0x140B: // HALF_FLOAT
        return VK_FORMAT_R16G16_SFLOAT;
      case 0x1406: // FLOAT
        return VK_FORMAT_R32G32_SFLOAT;
      default:
        return VK_FORMAT_UNDEFINED;
      }
    case 0x1907: // GL_RGB
    case 0x80E0: // GL_BGR
    case 0x8D98: // GL_RGB_INTEGER
    case 0x8D9A: // GL_BGR_INTEGER
      // Types applicable to three components from table 8.2 of OpenGL 4.4 spec
      switch (m_header.glType) {
      case 0x1401: // UNSIGNED_BYTE
        return VK_FORMAT_R8G8B8_UNORM;
      case 0x1400: // BYTE
        return VK_FORMAT_R8G8B8_SNORM;
      case 0x1403: // UNSIGNED_SHORT
        return VK_FORMAT_R16G16B16_UNORM;
      case 0x1402: // SHORT
        return VK_FORMAT_R16G16B16_SNORM;
      case 0x1405: // UNSIGNED_INT
        return VK_FORMAT_R32G32B32_UINT;
      case 0x1404: // INT
        return VK_FORMAT_R32G32B32_SINT;
      case 0x140B: // HALF_FLOAT
        return VK_FORMAT_R16G16B16_SFLOAT;
      case 0x1406: // FLOAT
        return VK_FORMAT_R32G32B32_SFLOAT;
      // UNSIGNED_BYTE_3_3_2 not supported
      // UNSIGNED_BYTE_2_3_3_REV not supported
      case 0x8363: // UNSIGNED_SHORT_5_6_5 ushort
        return VK_FORMAT_R5G6B5_UNORM_PACK16;
      case 0x8364: // UNSIGNED_SHORT_5_6_5_REV
        return VK_FORMAT_B5G6R5_UNORM_PACK16;
      case 0x8C3B: // UNSIGNED_INT_10F_11F_11F_REV
        return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
      default:
        return VK_FORMAT_UNDEFINED;
      }
    case 0x1908: // GL_RGBA
    case 0x80E1: // GL_BGRA
    case 0x8D99: // GL_RGBA_INTEGER
    case 0x8D9B: // GL_BGRA_INTEGER
      // Types applicable to three components from table 8.2 of OpenGL 4.4 spec
      switch (m_header.glType) {
      case 0x1401: // UNSIGNED_BYTE
        return VK_FORMAT_R8G8B8A8_UNORM;
      case 0x1400: // BYTE
        return VK_FORMAT_R8G8B8A8_SNORM;
      case 0x1403: // UNSIGNED_SHORT
        return VK_FORMAT_R16G16B16A16_UNORM;
      case 0x1402: // SHORT
        return VK_FORMAT_R16G16B16A16_SNORM;
      case 0x1405: // UNSIGNED_INT
        return VK_FORMAT_R32G32B32A32_UINT;
      case 0x1404: // INT
        return VK_FORMAT_R32G32B32A32_SINT;
      case 0x140B: // HALF_FLOAT
        return VK_FORMAT_R16G16B16A16_SFLOAT;
      case 0x1406: // FLOAT
        return VK_FORMAT_R32G32B32A32_SFLOAT;
      case 0x8033: // UNSIGNED_SHORT_4_4_4_4
      case 0x8365: // UNSIGNED_SHORT_4_4_4_4_REV
        return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
      case 0x8034: // UNSIGNED_SHORT_5_5_5_1
        return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
      case 0x8366: // UNSIGNED_SHORT_1_5_5_5_REV
        return VK_FORMAT_A1R5G5B5_UNORM_PACK16;
      case 0x8035: // UNSIGNED_INT_8_8_8_8 uint
        return VK_FORMAT_R8G8B8A8_UINT;
      case 0x8367: // UNSIGNED_INT_8_8_8_8_REV
        return VK_FORMAT_A8B8G8R8_UINT_PACK32;
      // UNSIGNED_INT_10_10_10_2 not supported
      case 0x8368: // UNSIGNED_INT_2_10_10_10_REV
        return VK_FORMAT_A2R10G10B10_UINT_PACK32;
      // UNSIGNED_INT_5_9_9_9_REV not supported
      default:
        return VK_FORMAT_UNDEFINED;
      }
    default:
      return VK_FORMAT_UNDEFINED;
    }
  }

  /*
  // Internal format for uncompressed textures
  switch (m_header.glInternalFormat) {
    // Sized internal color formats,  from table 8.12 of OpenGL 4.4 spec
  case 0x823A: // RG16UI RG ui16 ui16
  case 0x823B: // RG32I RG i32 i32
  case 0x823C: // RG32UI RG ui32 ui32
  case 0x8D8F: // RGB8I RGB i8 i8 i8
  case 0x8D7D: // RGB8UI RGB ui8 ui8 ui8
  case 0x8D89: // RGB16I RGB i16 i16 i16
  case 0x8D77: // RGB16UI RGB ui16 ui16 ui16
  case 0x8D83: // RGB32I RGB i32 i32 i32
  case 0x8D71: // RGB32UI RGB ui32 ui32 ui32
  case 0x8D8E: // RGBA8I RGBA i8 i8 i8 i8
  case 0x8D7C: // RGBA8UI RGBA ui8 ui8 ui8 ui8
  case 0x8D88: // RGBA16I RGBA i16 i16 i16 i16
  case 0x8D76: // RGBA16UI RGBA ui16 ui16 ui16 ui16
  case 0x8D82: // RGBA32I RGBA i32 i32 i32 i32
  case 0x8D70: // RGBA32UI RGBA ui32 ui32 ui32 ui32

    // Sized internal depth and stencil formats, from table 8.13 of OpenGL 4.4 spec
  case 0x81A5: // DEPTH_COMPONENT16 DEPTH_COMPONENT 16
  case 0x81A6: // DEPTH_COMPONENT24 DEPTH_COMPONENT 24
  case 0x81A7: // DEPTH_COMPONENT32 DEPTH_COMPONENT 32
  case 0x8CAC: // DEPTH_COMPONENT32F DEPTH_COMPONENT f32
  case 0x88F0: // DEPTH24_STENCIL8 DEPTH_STENCIL 24 ui8
  case 0x8CAD: // DEPTH32F_STENCIL8 DEPTH_STENCIL f32 ui8
  case 0x8D46: // STENCIL_INDEX1 STENCIL_INDEX ui1
  case 0x8D47: // STENCIL_INDEX4 STENCIL_INDEX ui4
  case 0x8D48: // STENCIL_INDEX8 STENCIL_INDEX ui8
  case 0x8D49: // STENCIL_INDEX16 STENCIL_INDEX ui16
  }
  */

  // Internal format for compressed textures
  switch (m_header.glInternalFormat) {
  // Compressed formats, from table 8.14 of OpenGL 4.4 spec:
  case 0x8225: // COMPRESSED_RED RED Generic unorm
  case 0x8226: // COMPRESSED_RG RG Generic unorm
  case 0x84ED: // COMPRESSED_RGB RGB Generic unorm
  case 0x84EE: // COMPRESSED_RGBA RGBA Generic unorm
  case 0x8C48: // COMPRESSED_SRGB RGB Generic unorm
  case 0x8C49: // COMPRESSED_SRGB_ALPHA RGBA Generic unorm
  case 0x8DBB: // COMPRESSED_RED_RGTC1 RED Specific unorm
    // Generic compressed formats not supported
    return VK_FORMAT_UNDEFINED;
    break;
  case 0x8DBC: // COMPRESSED_SIGNED_RED_RGTC1 RED Specific snorm
    return VK_FORMAT_BC4_SNORM_BLOCK;
  case 0x8DBD: // COMPRESSED_RG_RGTC2 RG Specific unorm
    return VK_FORMAT_BC5_UNORM_BLOCK;
  case 0x8DBE: // COMPRESSED_SIGNED_RG_RGTC2 RG Specific snorm
    return VK_FORMAT_BC5_SNORM_BLOCK;
  case 0x8E8C: // COMPRESSED_RGBA_BPTC_UNORM RGBA Specific unorm
    return VK_FORMAT_BC7_UNORM_BLOCK;
  case 0x8E8D: // COMPRESSED_SRGB_ALPHA_BPTC_UNORM RGBA Specific unorm
    return VK_FORMAT_BC7_SRGB_BLOCK;
  case 0x8E8E: // COMPRESSED_RGB_BPTC_SIGNED_FLOAT RGB Specific float
    return VK_FORMAT_BC6H_SFLOAT_BLOCK;
  case 0x8E8F: // COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT RGB Specific float
    return VK_FORMAT_BC6H_UFLOAT_BLOCK;
  case 0x9274: // COMPRESSED_RGB8_ETC2 RGB Specific unorm
    return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
  case 0x9275: // COMPRESSED_SRGB8_ETC2 RGB Specific unorm
    return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
  case 0x9276: // COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 RGB Specific unorm
    return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
  case 0x9277: // COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 RGB Specific unorm
    return VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;
  case 0x9278: // COMPRESSED_RGBA8_ETC2_EAC RGBA Specific unorm
    return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
  case 0x9279: // COMPRESSED_SRGB8_ALPHA8_ETC2_EAC RGBA Specific unorm
    return VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
  case 0x9270: // COMPRESSED_R11_EAC RED Specific unorm
    return VK_FORMAT_EAC_R11_UNORM_BLOCK;
  case 0x9271: // COMPRESSED_SIGNED_R11_EAC RED Specific snorm
    return VK_FORMAT_EAC_R11_SNORM_BLOCK;
  case 0x9272: // COMPRESSED_RG11_EAC RG Specific unorm
    return VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
  case 0x9273: // COMPRESSED_SIGNED_RG11_EAC RG Specific snorm
    return VK_FORMAT_EAC_R11G11_SNORM_BLOCK;

    default:
    return VK_FORMAT_UNDEFINED;
  }
}

long KRTextureKTX::getMemRequiredForSize(int max_dim)
{
  int target_dim = max_dim;
  if (target_dim < (int)m_min_lod_max_dim) target_dim = target_dim;

  // Determine how much memory will be consumed

  int width = m_header.pixelWidth;
  int height = m_header.pixelHeight;
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

bool KRTextureKTX::uploadTexture(KRDevice& device, VkImage& image, int lod_max_dim, int& current_lod_max_dim, bool premultiply_alpha)
{
  int target_dim = lod_max_dim;
  if (target_dim < (int)m_min_lod_max_dim) target_dim = m_min_lod_max_dim;

  if (m_blocks.size() == 0) {
    return false;
  }

  // Determine how much memory will be consumed
  int width = m_header.pixelWidth;
  int height = m_header.pixelHeight;
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
      GLDEBUG(glCompressedTexImage2D(target, destination_level, (unsigned int)m_header.glInternalFormat, width, height, 0, (int)block->getSize(), block->getStart()));
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

std::string KRTextureKTX::getExtension()
{
  return "ktx";
}

