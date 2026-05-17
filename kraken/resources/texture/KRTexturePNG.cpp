//
//  KRTexturePNG.cpp
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
#include "KRTexturePNG.h"
#include "KREngine-common.h"
#include "KRContext.h"
#include "KRTextureKTX2.h"

using namespace hydra;

#define SWAP_2(x) ( (((x) & 0xff) << 8) | ((uint16_t)(x) >> 8) )
#define SWAP_4(x) ( ((x) << 24) | (((x) << 8) & 0x00ff0000) |  (((x) >> 8) & 0x0000ff00) |  ((x) >> 24))

#pragma pack(1)
struct PNG_CHUNK_HEADER
{
    uint32_t length;
    char type[4];
};

struct PNG_CHUNK_IHDR
{
    PNG_CHUNK_HEADER header;
    uint32_t width;
    uint32_t height;
    uint8_t depth;
    uint8_t colorType;
    uint8_t compressionMethod;
    uint8_t filterMethod;
    uint8_t interlateMethod;
};

struct PNG_HEADER
{
    char magic[8]; // 137 80 78 71 13 10 26 10
    PNG_CHUNK_IHDR chunk_IHDR; // IHDR always comes first
};
#pragma pack()

KRTexturePNG::KRTexturePNG(KRContext& context, Block* data, std::string name) : KRTexture2D(context, data, name)
{
  data->lock();
  PNG_HEADER* pHeader = (PNG_HEADER*)data->getStart();
  uint8_t expected_magic[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };
  if (memcmp(pHeader->magic, expected_magic, 8) != 0) {
    assert(false);
    data->unlock();
    return; // Not a valid PNG file
  }

  m_dimensions.x = SWAP_4(pHeader->chunk_IHDR.width);
  m_dimensions.y = SWAP_4(pHeader->chunk_IHDR.height);
  m_lod_count = 1; // Mipmaps not yet supported for PNG images
  switch (pHeader->chunk_IHDR.colorType) {
  case 0:
      // greyscale
      m_imageSize = m_dimensions.x * m_dimensions.y * pHeader->chunk_IHDR.depth / 8;
      break;
  case 2:
      // RGB
      m_imageSize = m_dimensions.x * m_dimensions.y * pHeader->chunk_IHDR.depth / 8 * 3;
      break;
  case 3:
      // Palette
      m_imageSize = m_dimensions.x * m_dimensions.y * 3;
      break;
  case 4:
      // Greyscale + alpha
      m_imageSize = m_dimensions.x * m_dimensions.y * pHeader->chunk_IHDR.depth / 8 * 2;
      break;
  case 6:
      // RGB + Alpha
      m_imageSize = m_dimensions.x * m_dimensions.y * pHeader->chunk_IHDR.depth / 8 * 4;
      break;
  default:
      assert(false);
      break;
  }

  data->unlock();
}

KRTexturePNG::~KRTexturePNG()
{

}

bool KRTexturePNG::getLodData(void* buffer, int lod)
{
  unsigned char* converted_image = (unsigned char*)buffer;
  // TODO - Vulkan Refactoring - Perhaps it would be more efficient to reformat the color channels during the copy to the staging buffer.
  m_pData->lock();
  PNG_HEADER* pHeader = (PNG_HEADER*)m_pData->getStart();

  PNG_CHUNK_IHDR* chunk_IHDR = &pHeader->chunk_IHDR;

  PNG_CHUNK_HEADER* chunk = &chunk_IHDR->header;

  uint8_t* palette_data = nullptr;
  size_t palette_length = 0;
  uint8_t* alpha_data = nullptr;
  size_t alpha_length = 0;
  while(chunk != nullptr)
  {
	  chunk = (PNG_CHUNK_HEADER*)((uint8_t*)chunk + sizeof(PNG_CHUNK_HEADER) + SWAP_4(chunk->length));
	  assert(chunk < (PNG_CHUNK_HEADER*)m_pData->getEnd());
      if (memcmp(chunk->type, "IEND", 4) == 0) {
          chunk = nullptr;
      } else if (memcmp(chunk->type, "PLTE", 4) == 0) {
          palette_data = (uint8_t*)chunk + sizeof(PNG_CHUNK_HEADER);
		  palette_length = SWAP_4(chunk->length);
      } else if (memcmp(chunk->type, "tRNS", 4) == 0) {
          alpha_data = (uint8_t*)chunk + sizeof(PNG_CHUNK_HEADER);
          alpha_length = SWAP_4(chunk->length);
      } else if (memcmp(chunk->type, "IDAT", 4) == 0) {
          break;
	  }
  }

  return true;
}

#if !TARGET_OS_IPHONE && !defined(ANDROID)

KRTexture* KRTexturePNG::compress(bool premultiply_alpha)
{
  //  TODO - Vulkan refactoring... Not Implemented...
  assert(false);
  return nullptr;
}
#endif

long KRTexturePNG::getMemRequiredForLod(int lod)
{
  return m_imageSize;
}

Vector3i KRTexturePNG::getDimensions() const
{
  return Vector3i::Create(m_dimensions.x, m_dimensions.y, 1);
}

VkFormat KRTexturePNG::getFormat() const
{
  // TODO - We should not automatically add the alpha channel on import
  return VK_FORMAT_R8G8B8A8_SRGB;
}

std::string KRTexturePNG::getExtension()
{
  return "png";
}

int KRTexturePNG::getFaceCount() const
{
  return 1;
}
