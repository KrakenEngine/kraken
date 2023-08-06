//
//  KRTexture.cpp
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

#include "KREngine-common.h"
#include "KRTexture.h"
#include "block.h"
#include "KRContext.h"
#include "KRTextureManager.h"

KRTexture::KRTexture(KRContext& context, std::string name) : KRResource(context, name)
{
  m_current_lod_max_dim = 0;
  m_new_lod_max_dim = 0;
  m_textureMemUsed = 0;
  m_newTextureMemUsed = 0;
  m_last_frame_used = 0;
  m_last_frame_max_lod_coverage = 0.0f;
  m_last_frame_usage = TEXTURE_USAGE_NONE;
  m_handle_lock.clear();
  m_haveNewHandles = false;
}

KRTexture::~KRTexture()
{
  releaseHandles();
}

void KRTexture::TextureHandle::destroy(KRDeviceManager* deviceManager)
{
  std::unique_ptr<KRDevice>& d = deviceManager->getDevice(device);
  // TODO - Validate that device has not been lost
  if (fullImageView != VK_NULL_HANDLE) {
    vkDestroyImageView(d->m_logicalDevice, fullImageView, nullptr);
    fullImageView = VK_NULL_HANDLE;
  }
  if (image != VK_NULL_HANDLE) {
    VmaAllocator allocator = d->getAllocator();
    vmaDestroyImage(allocator, image, allocation);
  }
}

void KRTexture::destroyHandles()
{
  KRDeviceManager* deviceManager = getContext().getDeviceManager();
  for (TextureHandle t : m_handles) {
    t.destroy(deviceManager);
  }
  m_handles.clear();
  m_textureMemUsed = 0;
}

void KRTexture::destroyNewHandles()
{
  KRDeviceManager* deviceManager = getContext().getDeviceManager();
  for (TextureHandle t : m_newHandles) {
    t.destroy(deviceManager);
  }
  m_newHandles.clear();
  m_newTextureMemUsed = 0;
}

void KRTexture::releaseHandles()
{
  long mem_size = getMemSize();

  while (m_handle_lock.test_and_set()); // Spin lock

  destroyNewHandles();
  destroyHandles();

  m_current_lod_max_dim = 0;
  m_new_lod_max_dim = 0;

  m_handle_lock.clear();

  getContext().getTextureManager()->memoryChanged(-mem_size);
}

long KRTexture::getMemSize()
{
  return m_textureMemUsed + m_newTextureMemUsed; // TODO - This is not 100% accurate, as loaded format may differ in size while in GPU memory
}

long KRTexture::getReferencedMemSize()
{
  // Return the amount of memory used by other textures referenced by this texture (for cube maps and animated textures)
  return 0;
}

void KRTexture::resize(int max_dim)
{
  while (m_handle_lock.test_and_set()) {
  }; // Spin lock

  if (!m_haveNewHandles) {
    if (max_dim > 0) {
      int target_dim = max_dim;
      if (target_dim < (int)m_min_lod_max_dim) target_dim = m_min_lod_max_dim;

      if (m_new_lod_max_dim != target_dim || m_handles.empty()) {
        assert(m_newTextureMemUsed == 0);
        m_newTextureMemUsed = getMemRequiredForSize(target_dim);

        getContext().getTextureManager()->memoryChanged(m_newTextureMemUsed);
        getContext().getTextureManager()->addMemoryTransferredThisFrame(m_newTextureMemUsed);

        if (createGPUTexture(target_dim)) {
          m_new_lod_max_dim = target_dim;
        } else {
          getContext().getTextureManager()->memoryChanged(-m_newTextureMemUsed);
          m_newTextureMemUsed = 0;
          assert(false);  // Failed to create the texture
        }
      }
    }
  }

  m_handle_lock.clear();
}

void KRTexture::resetPoolExpiry(float lodCoverage, KRTexture::texture_usage_t textureUsage)
{
  long current_frame = getContext().getCurrentFrame();
  if (current_frame != m_last_frame_used) {
    m_last_frame_used = current_frame;
    m_last_frame_max_lod_coverage = 0.0f;
    m_last_frame_usage = TEXTURE_USAGE_NONE;

    getContext().getTextureManager()->primeTexture(this);
  }
  m_last_frame_max_lod_coverage = KRMAX(lodCoverage, m_last_frame_max_lod_coverage);
  m_last_frame_usage = static_cast<texture_usage_t>(static_cast<int>(m_last_frame_usage) | static_cast<int>(textureUsage));
}

kraken_stream_level KRTexture::getStreamLevel(KRTexture::texture_usage_t textureUsage)
{
  if (m_current_lod_max_dim == 0) {
    return kraken_stream_level::STREAM_LEVEL_OUT;
  } else if (m_current_lod_max_dim == KRMIN(getContext().KRENGINE_MAX_TEXTURE_DIM, (int)m_max_lod_max_dim)) {
    return kraken_stream_level::STREAM_LEVEL_IN_HQ;
  } else if (m_current_lod_max_dim >= KRMAX(getContext().KRENGINE_MIN_TEXTURE_DIM, (int)m_min_lod_max_dim)) {
    return kraken_stream_level::STREAM_LEVEL_IN_LQ;
  } else {
    return kraken_stream_level::STREAM_LEVEL_OUT;
  }
}

float KRTexture::getStreamPriority()
{
  long current_frame = getContext().getCurrentFrame();
  if (current_frame > m_last_frame_used + 5) {
    return 1.0f - KRCLAMP((float)(current_frame - m_last_frame_used) / 60.0f, 0.0f, 1.0f);
  } else {
    float priority = 100.0f;
    if (m_last_frame_usage & (TEXTURE_USAGE_UI | TEXTURE_USAGE_SHADOW_DEPTH)) {
      priority += 10000000.0f;
    }
    if (m_last_frame_usage & (TEXTURE_USAGE_SKY_CUBE | TEXTURE_USAGE_PARTICLE | TEXTURE_USAGE_SPRITE | TEXTURE_USAGE_LIGHT_FLARE)) {
      priority += 1000000.0f;
    }
    if (m_last_frame_usage & (TEXTURE_USAGE_DIFFUSE_MAP | TEXTURE_USAGE_AMBIENT_MAP | TEXTURE_USAGE_SPECULAR_MAP | TEXTURE_USAGE_NORMAL_MAP | TEXTURE_USAGE_REFLECTION_MAP)) {
      priority += 100000.0f;
    }
    if (m_last_frame_usage & (TEXTURE_USAGE_LIGHT_MAP)) {
      priority += 100000.0f;
    }
    if (m_last_frame_usage & (TEXTURE_USAGE_REFECTION_CUBE)) {
      priority += 100000.0f;
    }
    priority += m_last_frame_max_lod_coverage * 10.0f;
    return priority;
  }
}

float KRTexture::getLastFrameLodCoverage() const
{
  return m_last_frame_max_lod_coverage;
}

long KRTexture::getLastFrameUsed()
{
  return m_last_frame_used;
}
bool KRTexture::isAnimated()
{
  return false;
}

KRTexture* KRTexture::compress(bool premultiply_alpha)
{
  return NULL;
}

int KRTexture::getCurrentLodMaxDim()
{
  return m_current_lod_max_dim;
}

int KRTexture::getNewLodMaxDim()
{
  return m_new_lod_max_dim;
}

int KRTexture::getMaxMipMap()
{
  return m_max_lod_max_dim;
}

int KRTexture::getMinMipMap()
{
  return m_min_lod_max_dim;
}

bool KRTexture::hasMipmaps()
{
  return m_max_lod_max_dim != m_min_lod_max_dim;
}

void KRTexture::_swapHandles()
{
  //while(m_handle_lock.test_and_set()); // Spin lock
  if (!m_handle_lock.test_and_set()) {
    if (m_haveNewHandles) {
      destroyHandles();
      m_handles.swap(m_newHandles);
      m_textureMemUsed = (long)m_newTextureMemUsed;
      m_newTextureMemUsed = 0;
      m_current_lod_max_dim = m_new_lod_max_dim;
      m_haveNewHandles = false;
    }
    m_handle_lock.clear();
  }
}

VkImageView KRTexture::getFullImageView(KrDeviceHandle device)
{
  for (TextureHandle& handle : m_handles) {
    if (handle.device == device) {
      return handle.fullImageView;
    }
  }
  return VK_NULL_HANDLE;
}

VkImage KRTexture::getImage(KrDeviceHandle device)
{
  for (TextureHandle& handle : m_handles) {
    if (handle.device == device) {
      return handle.image;
    }
  }
  return VK_NULL_HANDLE;
}

bool KRTexture::allocate(KRDevice& device, hydra::Vector2i dimensions, VkImageCreateFlags imageCreateFlags, VkMemoryPropertyFlags properties, VkImage* image, VmaAllocation* allocation
#if KRENGINE_DEBUG_GPU_LABELS  
, const char* debug_label
#endif
)
{
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = static_cast<uint32_t>(dimensions.x);
  imageInfo.extent.height = static_cast<uint32_t>(dimensions.y);
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.flags = imageCreateFlags;


  uint32_t queueFamilyIndices[2] = {};
  imageInfo.pQueueFamilyIndices = queueFamilyIndices;
  imageInfo.queueFamilyIndexCount = 0;
  device.getQueueFamiliesForSharing(queueFamilyIndices, &imageInfo.queueFamilyIndexCount, &imageInfo.sharingMode);

  VmaAllocationCreateInfo allocInfo = {};
  allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
  allocInfo.requiredFlags = properties;

  VkResult res = vmaCreateImage(device.getAllocator(), &imageInfo, &allocInfo, image, allocation, nullptr);
  if (res != VK_SUCCESS) {
    return false;
  }
#if KRENGINE_DEBUG_GPU_LABELS
  device.setDebugLabel(*image, debug_label);
#endif
  return true;
}
