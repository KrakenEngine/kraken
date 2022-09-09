//
//  KRShader.cpp
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

#include "KRShader.h"
#include "spirv_reflect.h"

VkShaderStageFlagBits getShaderStageFromExtension(const char* extension)
{
  if (strcmp(extension, "vert") == 0) {
    return VK_SHADER_STAGE_VERTEX_BIT;
  } else if (strcmp(extension, "frag") == 0) {
    return VK_SHADER_STAGE_FRAGMENT_BIT;
  } else if (strcmp(extension, "tesc") == 0) {
    return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
  } else if (strcmp(extension, "tese") == 0) {
    return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
  } else if (strcmp(extension, "geom") == 0) {
    return VK_SHADER_STAGE_GEOMETRY_BIT;
  } else if (strcmp(extension, "comp") == 0) {
    return VK_SHADER_STAGE_COMPUTE_BIT;
  } else if (strcmp(extension, "mesh") == 0) {
    return VK_SHADER_STAGE_MESH_BIT_NV;
  } else if (strcmp(extension, "task") == 0) {
    return VK_SHADER_STAGE_TASK_BIT_NV;
  } else if (strcmp(extension, "rgen") == 0) {
    return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
  } else if (strcmp(extension, "rint") == 0) {
    return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
  } else if (strcmp(extension, "rahit") == 0) {
    return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
  } else if (strcmp(extension, "rchit") == 0) {
    return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
  } else if (strcmp(extension, "rmiss") == 0) {
    return VK_SHADER_STAGE_MISS_BIT_KHR;
  } else if (strcmp(extension, "rmiss") == 0) {
    return VK_SHADER_STAGE_CALLABLE_BIT_KHR;
  } else {
    return (VkShaderStageFlagBits)0;
  }
}

KRShader::KRShader(KRContext& context, std::string name, std::string extension) : KRResource(context, name)
{
  m_pData = new KRDataBlock();
  m_extension = extension;
  m_subExtension = KRResource::GetFileExtension(name);
  m_stage = getShaderStageFromExtension(m_subExtension.c_str());
  m_reflectionValid = false;

  getReflection();
}

KRShader::KRShader(KRContext& context, std::string name, std::string extension, KRDataBlock* data) : KRResource(context, name)
{
  m_pData = data;
  m_extension = extension;
  m_subExtension = KRResource::GetFileExtension(name);
  m_stage = getShaderStageFromExtension(m_subExtension.c_str());
  m_reflectionValid = false;
}

KRShader::~KRShader()
{
  freeReflection();
  delete m_pData;
}

std::string KRShader::getExtension()
{
  return m_extension;
}

std::string& KRShader::getSubExtension()
{
  return m_subExtension;
}

bool KRShader::save(KRDataBlock& data)
{
  data.append(*m_pData);
  return true;
}

KRDataBlock* KRShader::getData()
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

  VkResult result = vkCreateShaderModule(device, &createInfo, nullptr, &module);

  if (result != VK_SUCCESS) {
    success = false;
  }
  m_pData->unlock();

#if KRENGINE_DEBUG_GPU_LABELS
  if (success) {
    std::string& name = getName();
    VkDebugUtilsObjectNameInfoEXT debugInfo{};
    debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    debugInfo.objectHandle = (uint64_t)module;
    debugInfo.objectType = VK_OBJECT_TYPE_SHADER_MODULE;
    debugInfo.pObjectName = name.c_str();
    VkResult res = vkSetDebugUtilsObjectNameEXT(device, &debugInfo);
  }
#endif // KRENGINE_DEBUG_GPU_LABELS
  return success;
}

void KRShader::parseReflection()
{
  if (m_reflectionValid) {
    return;
  }
  m_pData->lock();

  // Generate reflection data for a shader
  SpvReflectResult result = spvReflectCreateShaderModule(m_pData->getSize(), m_pData->getStart(), &m_reflection);
  if (result != SPV_REFLECT_RESULT_SUCCESS) {
    // TODO - Log error
    return;
  }

  m_reflectionValid = true;
  m_pData->unlock();
}

void KRShader::freeReflection()
{
  if (!m_reflectionValid) {
    return;
  }
  spvReflectDestroyShaderModule(&m_reflection);
  m_reflectionValid = false;
}


const SpvReflectShaderModule* KRShader::getReflection()
{
  parseReflection();
  if (m_reflectionValid) {
    return &m_reflection;
  }
  return nullptr;
}

VkShaderStageFlagBits KRShader::getShaderStage() const
{
  return m_stage;
}