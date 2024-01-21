//
//  KRTextureManager.cpp
//  Kraken Engine
//
//  Copyright 2024 Kearwood Gilbert. All rights reserved.
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
#include "KRTextureManager.h"
#include "KRContext.h"
#include "KRTexture2D.h"
#include "KRTexturePVR.h"
#include "KRTextureTGA.h"
#include "KRTextureKTX.h"
#include "KRTextureKTX2.h"
#include "KRTextureCube.h"
#include "KRTextureAnimated.h"
#include "KRContext.h"

KRTextureManager::KRTextureManager(KRContext& context) : KRResourceManager(context)
{
  m_textureMemUsed = 0;

  m_memoryTransferredThisFrame = 0;
  m_streamerComplete = true;
}

void KRTextureManager::destroy()
{
  for (unordered_map<std::string, KRTexture*>::iterator itr = m_textures.begin(); itr != m_textures.end(); ++itr) {
    delete (*itr).second;
  }
  m_textures.clear();
}

KRTextureManager::~KRTextureManager()
{
  // Must call destroy() first
  assert(m_textures.empty());
}

void KRTextureManager::setMaxAnisotropy(float max_anisotropy)
{
  m_maxAnisotropy = max_anisotropy;
}

KRResource* KRTextureManager::loadResource(const std::string& name, const std::string& extension, Block* data)
{
  if (extension.compare("pvr") == 0 ||
    extension.compare("ktx") == 0 ||
    extension.compare("ktx2") == 0 ||
    extension.compare("tga") == 0) {
    return loadTexture(name.c_str(), extension.c_str(), data);
  }
  return nullptr;
}

KRResource* KRTextureManager::getResource(const std::string& name, const std::string& extension)
{
  if (extension.compare("pvr") == 0 ||
      extension.compare("ktx") == 0 ||
      extension.compare("ktx2") == 0 ||
      extension.compare("tga") == 0) {
    // TODO - Currently textures must have a unique name, without consideration
    //        of extensions.  When textures are compressed, the uncompressed versions
    //        are removed.  Both compressed and un-compressed textures should co-exist
    //        with the renderer prioritizing as necessary.  This will facilitate more
    //        ergonomic usage within toolchain and editor GUI.
    return getTexture(name);
  }
  return nullptr;
}

KRTexture* KRTextureManager::loadTexture(const char* szName, const char* szExtension, Block* data)
{
  KRTexture* pTexture = NULL;

  std::string lowerName = szName;
  std::transform(lowerName.begin(), lowerName.end(),
                 lowerName.begin(), ::tolower);

  std::string lowerExtension = szExtension;
  std::transform(lowerExtension.begin(), lowerExtension.end(),
                 lowerExtension.begin(), ::tolower);


  if (strcmp(szExtension, "pvr") == 0) {
    pTexture = new KRTexturePVR(getContext(), data, szName);
  } else if (strcmp(szExtension, "tga") == 0) {
    pTexture = new KRTextureTGA(getContext(), data, szName);
  } else if (strcmp(szExtension, "ktx") == 0) {
    pTexture = new KRTextureKTX(getContext(), data, szName);
  } else if (strcmp(szExtension, "ktx2") == 0) {
    pTexture = new KRTextureKTX2(getContext(), data, szName);
  }

  if (pTexture) {
    m_textures[lowerName] = pTexture;
  }
  return pTexture;
}

KRTexture* KRTextureManager::getTextureCube(const char* szName)
{
  std::string lowerName = szName;
  std::transform(lowerName.begin(), lowerName.end(),
                 lowerName.begin(), ::tolower);

  unordered_map<std::string, KRTexture*>::iterator itr = m_textures.find(lowerName);
  if (itr == m_textures.end()) {

    // Defer resolving the texture cube until its referenced textures are ready
    const char* SUFFIXES[6] = {
        "_positive_x",
        "_negative_x",
        "_positive_y",
        "_negative_y",
        "_positive_z",
        "_negative_z"
    };
    bool found_all = true;
    for (int i = 0; i < 6; i++) {
      std::string faceName = lowerName + SUFFIXES[i];
      KRTexture* faceTexture = dynamic_cast<KRTexture2D*>(getContext().getTextureManager()->getTexture(faceName));
      if (faceTexture == NULL) {
        found_all = false;
      }
    }

    if (found_all) {
      KRTextureCube* pTexture = new KRTextureCube(getContext(), lowerName);

      m_textures[lowerName] = pTexture;
      return pTexture;
    } else {
      return NULL;
    }
  } else {
    return (*itr).second;
  }
}

KRTexture* KRTextureManager::getTexture(const std::string& name)
{

  std::string lowerName = name;
  std::transform(lowerName.begin(), lowerName.end(),
                 lowerName.begin(), ::tolower);

  unordered_map<std::string, KRTexture*>::iterator itr = m_textures.find(lowerName);
  if (itr == m_textures.end()) {
    if (lowerName.length() <= 8) {
      return NULL;
    } else if (lowerName.compare(0, 8, "animate:", 0, 8) == 0) {
      // This is an animated texture, create KRTextureAnimated's on-demand
      KRTextureAnimated* pTexture = new KRTextureAnimated(getContext(), lowerName);
      m_textures[lowerName] = pTexture;
      return pTexture;
    } else {
      // Not found
      // fprintf(stderr, "ERROR: Texture not found: %s\n", name.c_str());
      return NULL;
    }
  } else {
    return (*itr).second;
  }
}

bool KRTextureManager::selectTexture(unsigned int target, int iTextureUnit, int iTextureHandle)
{
  // TODO - Vulkan Refactoring
  return true;
}

long KRTextureManager::getMemUsed()
{
  return m_textureMemUsed;
}

long KRTextureManager::getMemActive()
{
  long mem_active = 0;
  for (std::set<KRTexture*>::iterator itr = m_activeTextures.begin(); itr != m_activeTextures.end(); itr++) {
    KRTexture* activeTexture = *itr;
    mem_active += activeTexture->getMemSize();
  }

  return mem_active;
}

void KRTextureManager::startFrame(float deltaTime)
{
  // TODO - Implement proper double-buffering to reduce copy operations
  m_streamerFenceMutex.lock();

  if (m_streamerComplete) {
    assert(m_activeTextures_streamer_copy.size() == 0); // The streamer should have emptied this if it really did complete

    const long KRENGINE_TEXTURE_EXPIRY_FRAMES = 10;

    std::set<KRTexture*> expiredTextures;
    for (std::set<KRTexture*>::iterator itr = m_activeTextures.begin(); itr != m_activeTextures.end(); itr++) {
      KRTexture* activeTexture = *itr;
      activeTexture->_swapHandles();
      if (activeTexture->getLastFrameUsed() + KRENGINE_TEXTURE_EXPIRY_FRAMES < getContext().getCurrentFrame()) {
        // Expire textures that haven't been used in a long time
        expiredTextures.insert(activeTexture);
        activeTexture->releaseHandles();
      } else {
        float priority = activeTexture->getStreamPriority();
        m_activeTextures_streamer_copy.push_back(std::pair<float, KRTexture*>(priority, activeTexture));
      }
    }
    for (std::set<KRTexture*>::iterator itr = expiredTextures.begin(); itr != expiredTextures.end(); itr++) {
      m_activeTextures.erase(*itr);
    }

    if (m_activeTextures_streamer_copy.size() > 0) {
      m_streamerComplete = false;
    }
  }

  m_streamerFenceMutex.unlock();

  m_memoryTransferredThisFrame = 0;
}

void KRTextureManager::endFrame(float deltaTime)
{

}

void KRTextureManager::doStreaming(long& memoryRemaining, long& memoryRemainingThisFrame)
{

  // TODO - Implement proper double-buffering to reduce copy operations
  m_streamerFenceMutex.lock();
  m_activeTextures_streamer = std::move(m_activeTextures_streamer_copy);
  m_streamerFenceMutex.unlock();

  if (m_activeTextures_streamer.size() > 0) {
    balanceTextureMemory(memoryRemaining, memoryRemainingThisFrame);

    m_streamerFenceMutex.lock();
    m_streamerComplete = true;
    m_streamerFenceMutex.unlock();
  } else {
    memoryRemaining -= getMemUsed();
  }
}

void KRTextureManager::balanceTextureMemory(long& memoryRemaining, long& memoryRemainingThisFrame)
{
  // Balance texture memory by reducing and increasing the maximum mip-map level of both active and inactive textures
  // Favour performance over maximum texture resolution when memory is insufficient for textures at full resolution.

  /*
   NEW ALGORITHM:

   Textures are assigned a “weight” by tuneable criteria:
   - Area of screen coverage taken by objects containing material (more accurate and generic than distance)
   - Type of texture (separate weight for normal, diffuse, spec maps)
   - Last used time (to keep textures loaded for recently seen objects that are outside of the view frustum)
   Those factors combine together to give a “weight”, which represents a proportion relative to all other textures weights
   Mipmap levels are stripped off of each texture until they occupy the amount of memory they should proportionally have
   This is in contrast to the global ceiling of texture resolution that slowly drops until the textures fit

   */

   // ---------------

   //long MAX_STREAM_TIME = 66;
   //long startTime = getContext().getAbsoluteTimeMilliseconds();

  std::sort(m_activeTextures_streamer.begin(), m_activeTextures_streamer.end(), std::greater<std::pair<float, KRTexture*>>());

  for (auto itr = m_activeTextures_streamer.begin(); itr != m_activeTextures_streamer.end(); itr++) {
    KRTexture* texture = (*itr).second;
    int min_mip_level = KRMAX(getContext().KRENGINE_MIN_TEXTURE_DIM, texture->getMinMipMap());
    long minLodMem = texture->getMemRequiredForSize(min_mip_level);
    memoryRemaining -= minLodMem;

    if (memoryRemainingThisFrame > minLodMem && texture->getNewLodMaxDim() < min_mip_level) {
      memoryRemainingThisFrame -= minLodMem;
      texture->resize(min_mip_level);
    }
  }

  //long minMipTime = getContext().getAbsoluteTimeMilliseconds() - startTime;

  std::vector<int> mipPercents = { 75, 75, 50, 50, 50 };
  int mip_drop = -1;
  auto mip_itr = mipPercents.begin();
  long memoryRemainingThisMip = 0;

  for (auto itr = m_activeTextures_streamer.begin(); itr != m_activeTextures_streamer.end(); itr++) {
    if (memoryRemainingThisMip <= 0) {
      if (mip_itr == mipPercents.end()) {
        break;
      } else {
        memoryRemainingThisMip = memoryRemaining / 100L * (long)(*mip_itr);
        mip_drop++;
        mip_itr++;
      }
    }

    KRTexture* texture = (*itr).second;
    int min_mip_level = KRMAX(getContext().KRENGINE_MIN_TEXTURE_DIM, texture->getMinMipMap());
    int max_mip_level = KRMIN(getContext().KRENGINE_MAX_TEXTURE_DIM, texture->getMaxMipMap());
    int target_mip_level = (max_mip_level >> mip_drop);
    long targetMem = texture->getMemRequiredForSize(target_mip_level);
    long additionalMemRequired = targetMem - texture->getMemRequiredForSize(min_mip_level);
    memoryRemainingThisMip -= additionalMemRequired;
    memoryRemaining -= additionalMemRequired;
    if (memoryRemainingThisMip > 0 && memoryRemainingThisFrame > targetMem) {
      int current_mip_level = texture->getNewLodMaxDim();
      if (current_mip_level == (target_mip_level >> 1) || target_mip_level < current_mip_level) {
        memoryRemainingThisFrame -= targetMem;
        texture->resize(target_mip_level);
      } else if (current_mip_level == (target_mip_level >> 2)) {
        memoryRemainingThisFrame -= texture->getMemRequiredForSize(target_mip_level >> 1);
        texture->resize(target_mip_level >> 1);
      } else if (current_mip_level < (target_mip_level >> 2)) {
        memoryRemainingThisFrame -= texture->getMemRequiredForSize(target_mip_level >> 2);
        texture->resize(target_mip_level >> 2);
      }
    }

    //if(getContext().getAbsoluteTimeMilliseconds() - startTime > MAX_STREAM_TIME) {
    //    return; // Bail out early if we spend too long
    //}
  }

  //long streamerTime = getContext().getAbsoluteTimeMilliseconds() - startTime;
  //fprintf(stderr, "%i / %i\n", (int)minMipTime, (int)streamerTime);

}

long KRTextureManager::getMemoryTransferedThisFrame()
{
  return m_memoryTransferredThisFrame;
}

void KRTextureManager::addMemoryTransferredThisFrame(long memoryTransferred)
{
  m_memoryTransferredThisFrame += memoryTransferred;
}

void KRTextureManager::memoryChanged(long memoryDelta)
{
  m_textureMemUsed += memoryDelta;
  //fprintf(stderr, "Texture Memory: %ld / %i\n", (long)m_textureMemUsed, KRContext::KRENGINE_GPU_MEM_MAX);
}

unordered_map<std::string, KRTexture*>& KRTextureManager::getTextures()
{
  return m_textures;
}

void KRTextureManager::compress(bool premultiply_alpha)
{
  std::vector<KRTexture*> textures_to_remove;
  std::vector<KRTexture*> textures_to_add;

  for (unordered_map<std::string, KRTexture*>::iterator itr = m_textures.begin(); itr != m_textures.end(); itr++) {
    KRTexture* texture = (*itr).second;
    KRTexture* compressed_texture = texture->compress(premultiply_alpha);
    if (compressed_texture) {
      textures_to_remove.push_back(texture);
      textures_to_add.push_back(compressed_texture);
    } else {
      assert(false);
    }
  }

  for (std::vector<KRTexture*>::iterator itr = textures_to_remove.begin(); itr != textures_to_remove.end(); itr++) {
    KRTexture* texture = *itr;
    std::string lowerName = texture->getName();
    std::transform(lowerName.begin(), lowerName.end(),
                   lowerName.begin(), ::tolower);
    m_textures.erase(lowerName);
    delete texture;
  }

  for (std::vector<KRTexture*>::iterator itr = textures_to_add.begin(); itr != textures_to_add.end(); itr++) {
    KRTexture* texture = *itr;
    std::string lowerName = texture->getName();
    std::transform(lowerName.begin(), lowerName.end(),
                   lowerName.begin(), ::tolower);
    m_textures[lowerName] = texture;
  }
}


std::set<KRTexture*>& KRTextureManager::getActiveTextures()
{
  return m_activeTextures;
}

void KRTextureManager::primeTexture(KRTexture* texture)
{
  if (m_activeTextures.find(texture) == m_activeTextures.end()) {
    m_activeTextures.insert(texture);
  }
}

