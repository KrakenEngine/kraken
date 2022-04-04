//
//  SourceManager.cpp
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

#include "KRSourceManager.h"
#include "KREngine-common.h"

KRSourceManager::KRSourceManager(KRContext &context) : KRResourceManager(context)
{
    
}

KRSourceManager::~KRSourceManager()
{
    for(unordered_map<std::string, unordered_map<std::string, KRSource *> >::iterator extension_itr = m_sources.begin(); extension_itr != m_sources.end(); extension_itr++) {
        for(unordered_map<std::string, KRSource *>::iterator name_itr=(*extension_itr).second.begin(); name_itr != (*extension_itr).second.end(); name_itr++) {
            delete (*name_itr).second;
        }
    }
}

unordered_map<std::string, unordered_map<std::string, KRSource *> > &KRSourceManager::getSources()
{
    return m_sources;
}

void KRSourceManager::add(KRSource *source)
{
    std::string lower_name = source->getName();
    std::string lower_extension = source->getExtension();
    
    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
    std::transform(lower_extension.begin(), lower_extension.end(), lower_extension.begin(), ::tolower);
    
    unordered_map<std::string, unordered_map<std::string, KRSource *> >::iterator extension_itr = m_sources.find(lower_extension);
    if(extension_itr == m_sources.end()) {
        m_sources[lower_extension] = unordered_map<std::string, KRSource *>();
        extension_itr = m_sources.find(lower_extension);
    }
    
    unordered_map<std::string, KRSource *>::iterator name_itr = (*extension_itr).second.find(lower_name);
    if(name_itr != (*extension_itr).second.end()) {
        delete (*name_itr).second;
        (*name_itr).second = source;
    } else {
        (*extension_itr).second[lower_name] = source;
    }
}

KRResource* KRSourceManager::loadResource(const std::string& name, const std::string& extension, KRDataBlock* data)
{
  if (extension.compare("vert") == 0 ||
      extension.compare("frag") == 0 ||
      extension.compare("tesc") == 0 ||
      extension.compare("tese") == 0 ||
      extension.compare("geom") == 0 ||
      extension.compare("comp") == 0 ||
      extension.compare("mesh") == 0 ||
      extension.compare("task") == 0 ||
      extension.compare("rgen") == 0 ||
      extension.compare("rint") == 0 ||
      extension.compare("rahit") == 0 ||
      extension.compare("rchit") == 0 ||
      extension.compare("rmiss") == 0 ||
      extension.compare("rcall") == 0 ||
      extension.compare("glsl") == 0 ||
      extension.compare("options") == 0) {
    return load(name, extension, data);
  }
  return nullptr;
}

KRResource* KRSourceManager::getResource(const std::string& name, const std::string& extension)
{
  if (extension.compare("vert") == 0 ||
      extension.compare("frag") == 0 ||
      extension.compare("tesc") == 0 ||
      extension.compare("tese") == 0 ||
      extension.compare("geom") == 0 ||
      extension.compare("comp") == 0 ||
      extension.compare("mesh") == 0 ||
      extension.compare("task") == 0 ||
      extension.compare("rgen") == 0 ||
      extension.compare("rint") == 0 ||
      extension.compare("rahit") == 0 ||
      extension.compare("rchit") == 0 ||
      extension.compare("rmiss") == 0 ||
      extension.compare("rcall") == 0 ||
      extension.compare("glsl") == 0 ||
      extension.compare("options") == 0) {
    return get(name, extension);
  }
  return nullptr;
}

KRSource *KRSourceManager::load(const std::string &name, const std::string &extension, KRDataBlock *data)
{
    KRSource *source = new KRSource(getContext(), name, extension, data);
    if(source) add(source);
    return source;
}

KRSource *KRSourceManager::get(const std::string &name, const std::string &extension)
{
    std::string lower_name = name;
    std::string lower_extension = extension;
    
    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
    std::transform(lower_extension.begin(), lower_extension.end(), lower_extension.begin(), ::tolower);
    
    return m_sources[lower_extension][lower_name];
}


const unordered_map<std::string, KRSource *> &KRSourceManager::get(const std::string &extension)
{
    std::string lower_extension = extension;
    std::transform(lower_extension.begin(), lower_extension.end(), lower_extension.begin(), ::tolower);
    return m_sources[lower_extension];
}

