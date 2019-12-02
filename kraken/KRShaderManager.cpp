//
//  ShaderManager.cpp
//  KREngine
//
//  Copyright 2019 Kearwood Gilbert. All rights reserved.
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

#include "KRShaderManager.h"
#include "KREngine-common.h"

KRShaderManager::KRShaderManager(KRContext &context) : KRContextObject(context)
{
    
}

KRShaderManager::~KRShaderManager()
{
    for(unordered_map<std::string, unordered_map<std::string, KRShader *> >::iterator extension_itr = m_shaders.begin(); extension_itr != m_shaders.end(); extension_itr++) {
        for(unordered_map<std::string, KRShader *>::iterator name_itr=(*extension_itr).second.begin(); name_itr != (*extension_itr).second.end(); name_itr++) {
            delete (*name_itr).second;
        }
    }
}

unordered_map<std::string, unordered_map<std::string, KRShader *> > &KRShaderManager::getShaders()
{
    return m_shaders;
}

void KRShaderManager::add(KRShader *shader)
{
    std::string lower_name = shader->getName();
    std::string lower_extension = shader->getExtension();
    
    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
    std::transform(lower_extension.begin(), lower_extension.end(), lower_extension.begin(), ::tolower);
    
    unordered_map<std::string, unordered_map<std::string, KRShader *> >::iterator extension_itr = m_shaders.find(lower_extension);
    if(extension_itr == m_shaders.end()) {
        m_shaders[lower_extension] = unordered_map<std::string, KRShader *>();
        extension_itr = m_shaders.find(lower_extension);
    }
    
    unordered_map<std::string, KRShader *>::iterator name_itr = (*extension_itr).second.find(lower_name);
    if(name_itr != (*extension_itr).second.end()) {
        delete (*name_itr).second;
        (*name_itr).second = shader;
    } else {
        (*extension_itr).second[lower_name] = shader;
    }
}

KRShader *KRShaderManager::load(const std::string &name, const std::string &extension, KRDataBlock *data)
{
    KRShader *shader = new KRShader(getContext(), name, extension, data);
    if(shader) add(shader);
    return shader;
}

KRShader *KRShaderManager::get(const std::string &name, const std::string &extension)
{
    std::string lower_name = name;
    std::string lower_extension = extension;
    
    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
    std::transform(lower_extension.begin(), lower_extension.end(), lower_extension.begin(), ::tolower);
    
    return m_shaders[lower_extension][lower_name];
}


const unordered_map<std::string, KRShader *> &KRShaderManager::get(const std::string &extension)
{
    std::string lower_extension = extension;
    std::transform(lower_extension.begin(), lower_extension.end(), lower_extension.begin(), ::tolower);
    return m_shaders[lower_extension];
}

