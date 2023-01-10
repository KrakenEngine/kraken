//
//  KRTextureCube.cpp
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

#include "KRTextureCube.h"
#include "KRTexture2D.h"
#include "KRContext.h"

KRTextureCube::KRTextureCube(KRContext& context, std::string name) : KRTexture(context, name)
{

  m_max_lod_max_dim = 2048;
  m_min_lod_max_dim = 64;

  for (int i = 0; i < 6; i++) {
    m_textures[i] = NULL;
    std::string faceName = getName() + SUFFIXES[i];
    m_textures[i] = (KRTexture2D*)getContext().getTextureManager()->getTexture(faceName);
    if (m_textures[i]) {
      if (m_textures[i]->getMaxMipMap() < (int)m_max_lod_max_dim) m_max_lod_max_dim = m_textures[i]->getMaxMipMap();
      if (m_textures[i]->getMinMipMap() > (int)m_min_lod_max_dim) m_min_lod_max_dim = m_textures[i]->getMinMipMap();
    } else {
      assert(false);
    }
  }
}

KRTextureCube::~KRTextureCube()
{}

bool KRTextureCube::createGPUTexture(int lod_max_dim)
{
  assert(!m_haveNewHandles); // Only allow one resize per frame

  bool success = true;

  int prev_lod_max_dim = m_new_lod_max_dim;
  m_new_lod_max_dim = 0;
  bool bMipMaps = false;

  Vector2i dimensions = Vector2i::Zero();

  for (int i = 0; i < 6; i++) {
    if (!m_textures[i]) {
      success = false;
    } else {
      KRTexture2D& tex = *m_textures[i];
      Vector2i texDimensions = tex.getDimensions();
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

  KRDeviceManager* deviceManager = getContext().getDeviceManager();

  for (auto deviceItr = deviceManager->getDevices().begin(); deviceItr != deviceManager->getDevices().end(); deviceItr++) {
    KRDevice& device = *(*deviceItr).second;
    KrDeviceHandle deviceHandle = (*deviceItr).first;
    VmaAllocator allocator = device.getAllocator();
    KRTexture::TextureHandle& texture = m_newHandles.emplace_back();
    texture.device = deviceHandle;
    texture.allocation = VK_NULL_HANDLE;
    texture.image = VK_NULL_HANDLE;

    if (!device.createImage(dimensions, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texture.image, &texture.allocation
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
        // TODO - Vulkan refactoring.  We need to create a cube map texture rather than individual 2d textures.
        m_textures[i]->uploadTexture(device, texture.image, lod_max_dim, m_new_lod_max_dim);
      }
    }
  }
  if (success) {
    m_haveNewHandles = true;
  } else {
    destroyHandles();
    m_new_lod_max_dim = prev_lod_max_dim;
  }

  return success;
}

long KRTextureCube::getMemRequiredForSize(int max_dim)
{
  int target_dim = max_dim;
  if (target_dim < (int)m_min_lod_max_dim) target_dim = m_min_lod_max_dim;

  long memoryRequired = 0;
  for (int i = 0; i < 6; i++) {
    if (m_textures[i]) {
      memoryRequired += m_textures[i]->getMemRequiredForSize(target_dim);
    }
  }
  return memoryRequired;
}


void KRTextureCube::resetPoolExpiry(float lodCoverage, texture_usage_t textureUsage)
{
    KRTexture::resetPoolExpiry(lodCoverage, textureUsage);
    for(int i=0; i<6; i++) {
        if(m_textures[i]) {
            m_textures[i]->resetPoolExpiry(lodCoverage, textureUsage); // Ensure that side of cube maps do not expire from the texture pool prematurely, as they are referenced indirectly
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

bool KRTextureCube::save(KRDataBlock& data)
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
