//
//  KRSurfaceManager.cpp
//  Kraken Engine
//
//  Copyright 2025 Kearwood Gilbert. All rights reserved.
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

#include "KRSurfaceManager.h"


KRSurfaceManager::KRSurfaceManager(KRContext& context)
  : KRContextObject(context)
  , m_topSurfaceHandle(0)
{}

KRSurfaceManager::~KRSurfaceManager()
{
  destroySurfaces();
}

void KRSurfaceManager::destroySurfaces()
{
  const std::lock_guard<std::mutex> surfaceLock(KRContext::g_SurfaceInfoMutex);
  m_surfaces.clear();
}

KrResult KRSurfaceManager::create(void* platformHandle, KrSurfaceHandle& surfaceHandle)
{
  surfaceHandle = 0;

  std::unique_ptr<KRSurface> surface = std::make_unique<KRSurface>(*m_pContext, m_topSurfaceHandle, platformHandle);

  KrResult initialize_result = surface->initialize();
  if (initialize_result != KR_SUCCESS) {
    return initialize_result;
  }

  surfaceHandle = ++m_topSurfaceHandle;
  m_surfaces.insert(std::pair<KrSurfaceHandle, std::unique_ptr<KRSurface>>(surfaceHandle, std::move(surface)));

  return KR_SUCCESS;
}

KrResult KRSurfaceManager::destroy(KrSurfaceHandle& surfaceHandle)
{
  auto itr = m_surfaces.find(surfaceHandle);
  if (itr == m_surfaces.end()) {
    return KR_ERROR_NOT_FOUND;
  }
  m_surfaces.erase(itr);
  return KR_SUCCESS;
}

KRSurface& KRSurfaceManager::get(KrSurfaceHandle surfaceHandle)
{
  auto itr = m_surfaces.find(surfaceHandle);
  if (itr == m_surfaces.end()) {
    assert(false);
  }
  return *m_surfaces[surfaceHandle];
}


unordered_map<KrSurfaceHandle, std::unique_ptr<KRSurface>>& KRSurfaceManager::getSurfaces()
{
  return m_surfaces;
}
