//
//  KRShader.cpp
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

#include "KRShader.h"

KRShader::KRShader(KRContext &context, std::string name, std::string extension) : KRResource(context, name)
{
    m_pData = new KRDataBlock();
    m_extension = extension;
}

KRShader::KRShader(KRContext &context, std::string name, std::string extension, KRDataBlock *data) : KRResource(context, name)
{
    m_pData = data;
    m_extension = extension;
}

KRShader::~KRShader()
{
    delete m_pData;
}

std::string KRShader::getExtension()
{
    return m_extension;
}

bool KRShader::save(KRDataBlock &data)
{
    data.append(*m_pData);
    return true;
}

KRDataBlock *KRShader::getData()
{
    return m_pData;
}

bool KRShader::createShaderModule(VkDevice& device, VkShaderModule& module)
{
  bool success = true;
  VkShaderModuleCreateInfo createInfo{};
  m_pData->lock();
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = m_pData->getSize();
  createInfo.pCode = reinterpret_cast<const uint32_t*>(m_pData->getStart());
  
  VkShaderModule shaderModule;
  if (vkCreateShaderModule(device, &createInfo, nullptr, &module) != VK_SUCCESS) {
    success = false;
  }
  m_pData->unlock();
  return success;
}
