//
//  KRBundleManager.cpp
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

#include "KRBundleManager.h"

#include "KRBundle.h"

KRBundleManager::KRBundleManager(KRContext &context) : KRResourceManager(context) {
    
}

KRBundleManager::~KRBundleManager() {
    for(unordered_map<std::string, KRBundle *>::iterator itr = m_bundles.begin(); itr != m_bundles.end(); ++itr){
        delete (*itr).second;
    }
    m_bundles.clear();
}

KRResource* KRBundleManager::loadResource(const std::string& name, const std::string& extension, KRDataBlock* data)
{
  if (extension.compare("krbundle") == 0) {
    return loadBundle(name.c_str() , data);
  }
  return nullptr;
}
KRResource* KRBundleManager::getResource(const std::string& name, const std::string& extension)
{
  if (extension.compare("krbundle") == 0) {
    return getBundle(name.c_str());
  }
  return nullptr;
}

KRBundle *KRBundleManager::loadBundle(const char *szName, KRDataBlock *pData)
{
    KRBundle *pBundle = new KRBundle(*m_pContext, szName, pData);
    m_bundles[szName] = pBundle;
    return pBundle;
}

KRBundle *KRBundleManager::createBundle(const char *szName)
{
  // TODO: Check for name conflicts
  KRBundle *pBundle = new KRBundle(*m_pContext, szName);
  m_bundles[szName] = pBundle;
  return pBundle;
}

KRBundle *KRBundleManager::getBundle(const char *szName) {
    return m_bundles[szName];
}

unordered_map<std::string, KRBundle *> KRBundleManager::getBundles() {
    return m_bundles;
}