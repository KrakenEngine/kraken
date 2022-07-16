//
//  KRTexture.cpp
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

#include "KREngine-common.h"
#include "KRTexture.h"
#include "KRDataBlock.h"
#include "KRContext.h"
#include "KRTextureManager.h"

KRTexture::KRTexture(KRContext &context, std::string name) : KRResource(context, name)
{
    m_current_lod_max_dim = 0;
    m_new_lod_max_dim = 0;
    m_iHandle = 0;
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

void KRTexture::releaseHandles() {
    long mem_size = getMemSize();
    KRDeviceManager* deviceManager = getContext().getDeviceManager();
    
    while(m_handle_lock.test_and_set()); // Spin lock
    
    for (TextureHandle t : m_newHandles) {
      std::unique_ptr<KRDevice>& device = deviceManager->getDevice(t.device);
      VmaAllocator allocator = device->getAllocator();
      vmaDestroyImage(allocator, t.image, t.allocation);
    }
    m_newHandles.clear();
    m_newTextureMemUsed = 0;

    for (TextureHandle t : m_handles) {
      std::unique_ptr<KRDevice>& device = deviceManager->getDevice(t.device);
      VmaAllocator allocator = device->getAllocator();
      vmaDestroyImage(allocator, t.image, t.allocation);
    }
    m_handles.clear();
    m_textureMemUsed = 0;

    m_current_lod_max_dim = 0;
    m_new_lod_max_dim = 0;
    
    m_handle_lock.clear();
    
    getContext().getTextureManager()->memoryChanged(-mem_size);
}

long KRTexture::getMemSize() {
    return m_textureMemUsed + m_newTextureMemUsed; // TODO - This is not 100% accurate, as loaded format may differ in size while in GPU memory
}

long KRTexture::getReferencedMemSize() {
    // Return the amount of memory used by other textures referenced by this texture (for cube maps and animated textures)
    return 0;
}

void KRTexture::resize(int max_dim)
{
    while(m_handle_lock.test_and_set()) {}; // Spin lock

    if(!m_haveNewHandles) {
        if(max_dim > 0) {
            int target_dim = max_dim;
            if(target_dim < (int)m_min_lod_max_dim) target_dim = m_min_lod_max_dim;

            if(m_new_lod_max_dim != target_dim || m_handles.empty()) {
                assert(m_newTextureMemUsed == 0);
                m_newTextureMemUsed = getMemRequiredForSize(target_dim);
                
                getContext().getTextureManager()->memoryChanged(m_newTextureMemUsed);
                getContext().getTextureManager()->addMemoryTransferredThisFrame(m_newTextureMemUsed);
                
                if(!createGPUTexture(target_dim)) {
                    getContext().getTextureManager()->memoryChanged(-m_newTextureMemUsed);
                    m_newTextureMemUsed = 0;
                    assert(false);  // Failed to create the texture
                }
            }
        }
    }
    
    m_handle_lock.clear();
}

GLuint KRTexture::getHandle() {
    resetPoolExpiry(0.0f, KRTexture::TEXTURE_USAGE_NONE); // TODO - Pass through getHandle() arguements to replace extraneous resetPoolExpiry calls?
    return m_iHandle;
}

void KRTexture::resetPoolExpiry(float lodCoverage, KRTexture::texture_usage_t textureUsage)
{
    long current_frame = getContext().getCurrentFrame();
    if(current_frame != m_last_frame_used) {
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
    if(m_current_lod_max_dim == 0) {
        return kraken_stream_level::STREAM_LEVEL_OUT;
    } else if(m_current_lod_max_dim == KRMIN(getContext().KRENGINE_MAX_TEXTURE_DIM, (int)m_max_lod_max_dim)) {
        return kraken_stream_level::STREAM_LEVEL_IN_HQ;
    } else if(m_current_lod_max_dim >= KRMAX(getContext().KRENGINE_MIN_TEXTURE_DIM, (int)m_min_lod_max_dim)) {
        return kraken_stream_level::STREAM_LEVEL_IN_LQ;
    } else {
        return kraken_stream_level::STREAM_LEVEL_OUT;
    }
}

float KRTexture::getStreamPriority()
{
    long current_frame = getContext().getCurrentFrame();
    if(current_frame > m_last_frame_used + 5) {
        return 1.0f - KRCLAMP((float)(current_frame - m_last_frame_used) / 60.0f, 0.0f, 1.0f);
    } else {
        float priority = 100.0f;
        if(m_last_frame_usage & (TEXTURE_USAGE_UI | TEXTURE_USAGE_SHADOW_DEPTH)) {
            priority += 10000000.0f;
        }
        if(m_last_frame_usage & (TEXTURE_USAGE_SKY_CUBE | TEXTURE_USAGE_PARTICLE | TEXTURE_USAGE_SPRITE | TEXTURE_USAGE_LIGHT_FLARE)) {
            priority += 1000000.0f;
        }
        if(m_last_frame_usage & (TEXTURE_USAGE_DIFFUSE_MAP | TEXTURE_USAGE_AMBIENT_MAP | TEXTURE_USAGE_SPECULAR_MAP | TEXTURE_USAGE_NORMAL_MAP | TEXTURE_USAGE_REFLECTION_MAP)) {
            priority += 100000.0f;
        }
        if(m_last_frame_usage & (TEXTURE_USAGE_LIGHT_MAP)) {
            priority += 100000.0f;
        }
        if(m_last_frame_usage & (TEXTURE_USAGE_REFECTION_CUBE)) {
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

KRTexture *KRTexture::compress(bool premultiply_alpha)
{
    return NULL;
}

int KRTexture::getCurrentLodMaxDim() {
    return m_current_lod_max_dim;
}

int KRTexture::getNewLodMaxDim() {
    return m_new_lod_max_dim;
}

int KRTexture::getMaxMipMap() {
    return m_max_lod_max_dim;
}

int KRTexture::getMinMipMap() {
    return m_min_lod_max_dim;
}

bool KRTexture::hasMipmaps() {
    return m_max_lod_max_dim != m_min_lod_max_dim;
}

void KRTexture::bind(GLuint texture_unit) {
    
}

void KRTexture::_swapHandles()
{
    KRDeviceManager* deviceManager = getContext().getDeviceManager();

    //while(m_handle_lock.test_and_set()); // Spin lock
    if(!m_handle_lock.test_and_set()) {
        if(m_haveNewHandles) {
            for (TextureHandle t : m_handles) {
              std::unique_ptr<KRDevice>& device = deviceManager->getDevice(t.device);
              VmaAllocator allocator = device->getAllocator();
              vmaDestroyImage(allocator, t.image, t.allocation);
            }
            m_handles.clear();
            m_handles.swap(m_newHandles);
            m_textureMemUsed = (long)m_newTextureMemUsed;
            m_newTextureMemUsed = 0;
            m_current_lod_max_dim = m_new_lod_max_dim;
            m_haveNewHandles = false;
        }
        m_handle_lock.clear();
    }
}

