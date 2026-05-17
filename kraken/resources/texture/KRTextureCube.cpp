//
//  KRTextureCube.cpp
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

#include "KRTextureCube.h"
#include "KRTexture2D.h"
#include "KRContext.h"

using namespace mimir;
using namespace hydra;

KRTextureCube::KRTextureCube(KRContext& context, std::string name) : KRTexture(context, name)
{
  m_lod_count = 0;

  for (int i = 0; i < 6; i++) {
    m_textures[i] = NULL;
    std::string faceName = getName() + SUFFIXES[i];
    m_textures[i] = (KRTexture2D*)getContext().getTextureManager()->getTexture(faceName);
    if (m_textures[i]) {
	  m_lod_count = std::max(m_lod_count, m_textures[i]->getLodCount());
    } else {
      assert(false);
    }
  }
}

KRTextureCube::~KRTextureCube()
{}

bool KRTextureCube::createGPUTexture(int lod)
{
  assert(!m_haveNewHandles); // Only allow one resize per frame

  bool success = true;

  int target_lod = m_new_lod;
  m_new_lod = -1;
  bool bMipMaps = false;

  Vector2i dimensions = Vector2i::Zero();

  for (int i = 0; i < 6; i++) {
    if (!m_textures[i]) {
      success = false;
    } else {
      KRTexture2D& tex = *m_textures[i];
      Vector2i texDimensions = tex.getDimensions().xy();
      if (dimensions.x == 0) {
        dimensions = texDimensions;
      } else if (dimensions != texDimensions) {
        success = false;
      }
      if (tex.hasMipmaps()) {
        bMipMaps = true;
      }
    }
  }
  if (!success) {
    // Not all face images were loaded, or they have
    // mismatched dimensions
    // TODO - Perhaps we should have multiple error result codes.
    return false;
  }

  size_t bufferSizes[6] = {};
  void* buffers[6] = {};
  for (int i = 0; i < 6; i++) {
      bufferSizes[i] = getMemRequiredForLodRange(lod);
      buffers[i] = malloc(bufferSizes[i]);
	  m_textures[i]->getLodData(buffers[i], lod);
  }

  KRDeviceManager* deviceManager = getContext().getDeviceManager();

  for (auto deviceItr = deviceManager->getDevices().begin(); deviceItr != deviceManager->getDevices().end(); deviceItr++) {
    KRDevice& device = *(*deviceItr).second;
    KrDeviceHandle deviceHandle = (*deviceItr).first;
    VmaAllocator allocator = device.getAllocator();
    KRTexture::TextureHandle& texture = m_newHandles.emplace_back();
    texture.device = deviceHandle;
    texture.allocation = VK_NULL_HANDLE;
    texture.image = VK_NULL_HANDLE;

    if (!allocate(device, target_lod, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texture.image, &texture.allocation
#if KRENGINE_DEBUG_GPU_LABELS
      , getName().c_str()
#endif
      )) {
      success = false;
      break;
    }

    for (int i = 0; i < 6; i++) {
      std::string faceName = getName() + SUFFIXES[i];
      if (m_textures[i]) {
          VkBufferImageCopy region{};
          region.bufferOffset = 0;
          region.bufferRowLength = 0;
          region.bufferImageHeight = 0;

          region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
          region.imageSubresource.mipLevel = 0;
          region.imageSubresource.baseArrayLayer = 0;
          region.imageSubresource.layerCount = 1;

          region.imageOffset = { 0, 0, 0 };
          region.imageExtent = {
              (unsigned int)dimensions.x,
              (unsigned int)dimensions.y,
              (unsigned int)1
          };

          // TODO - Vulkan refactoring.  We need to create a cube map texture rather than individual 2d textures.
          device.streamUpload(buffers[i], bufferSizes[i], texture.image, &region, 1);
      }
    }
  }
  if (success) {
    m_haveNewHandles = true;
  } else {
    destroyHandles();
    m_new_lod = target_lod;
  }

  for (int i = 0; i < 6; i++) {
      if (buffers[i]) {
          delete buffers[i];
      }
  }

  return success;
}

long KRTextureCube::getMemRequiredForLod(int lod)
{
  long memoryRequired = 0;
  for (int i = 0; i < 6; i++) {
    if (m_textures[i]) {
      memoryRequired += m_textures[i]->getMemRequiredForLod(lod);
    }
  }
  return memoryRequired;
}

void KRTextureCube::requestResidency(float lodCoverage, texture_usage_t textureUsage)
{
    KRTexture::requestResidency(lodCoverage, textureUsage);
    for(int i=0; i<6; i++) {
        if(m_textures[i]) {
            m_textures[i]->requestResidency(lodCoverage, textureUsage); // Ensure that side of cube maps do not expire from the texture pool prematurely, as they are referenced indirectly
        }
    }
}

std::string KRTextureCube::getExtension()
{
  return ""; // Cube maps are just references; there are no files to output
}

bool KRTextureCube::save(const std::string& path)
{
  return true; // Cube maps are just references; there are no files to output
}

bool KRTextureCube::save(Block& data)
{
  return true; // Cube maps are just references; there are no files to output
}

int KRTextureCube::getFaceCount() const
{
  return 6;
}

VkFormat KRTextureCube::getFormat() const
{
  // TODO - Implement
  return VK_FORMAT_UNDEFINED;
}

hydra::Vector3i KRTextureCube::getDimensions() const
{
	return m_textures[0] ? m_textures[0]->getDimensions() : Vector3i::Zero();
}
