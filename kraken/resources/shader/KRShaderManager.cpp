//
//  ShaderManager.cpp
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

#include "KRShaderManager.h"
#include "KREngine-common.h"
#include "KRContext.h"
#include "resources/source/KRSourceManager.h"
#include "resources/unknown/KRUnknownManager.h"
#include "resources/unknown/KRUnknown.h"

#include "mimir.h"

using namespace mimir;

KRShaderManager::KRShaderManager(KRContext& context) : KRResourceManager(context)
, m_initializedGlslang(false)
, m_includer(&context)
{

}

KRShaderManager::~KRShaderManager()
{
  for (unordered_map<std::string, unordered_map<std::string, KRShader*> >::iterator extension_itr = m_shaders.begin(); extension_itr != m_shaders.end(); extension_itr++) {
    for (unordered_map<std::string, KRShader*>::iterator name_itr = (*extension_itr).second.begin(); name_itr != (*extension_itr).second.end(); name_itr++) {
      delete (*name_itr).second;
    }
  }
  if (m_initializedGlslang) {
    glslang::FinalizeProcess();
  }
}

KRResource* KRShaderManager::loadResource(const std::string& name, const std::string& extension, Block* data)
{
  if (extension.compare("spv") == 0) {
    return load(name, extension, data);
  }
  return nullptr;
}
KRResource* KRShaderManager::getResource(const std::string& name, const std::string& extension)
{
  if (extension.compare("spv") == 0) {
    return get(name, extension);
  }
  return nullptr;
}

unordered_map<std::string, unordered_map<std::string, KRShader*> >& KRShaderManager::getShaders()
{
  return m_shaders;
}

void KRShaderManager::add(KRShader* shader)
{
  std::string lower_name = shader->getName();
  std::string lower_extension = shader->getExtension();

  std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
  std::transform(lower_extension.begin(), lower_extension.end(), lower_extension.begin(), ::tolower);

  unordered_map<std::string, unordered_map<std::string, KRShader*> >::iterator extension_itr = m_shaders.find(lower_extension);
  if (extension_itr == m_shaders.end()) {
    m_shaders[lower_extension] = unordered_map<std::string, KRShader*>();
    extension_itr = m_shaders.find(lower_extension);
  }

  unordered_map<std::string, KRShader*>::iterator name_itr = (*extension_itr).second.find(lower_name);
  if (name_itr != (*extension_itr).second.end()) {
    delete (*name_itr).second;
    (*name_itr).second = shader;
  } else {
    (*extension_itr).second[lower_name] = shader;
  }
}

KRShader* KRShaderManager::load(const std::string& name, const std::string& extension, Block* data)
{
  KRShader* shader = new KRShader(getContext(), name, extension, data);
  if (shader) add(shader);
  return shader;
}

KRShader* KRShaderManager::get(const std::string& name, const std::string& extension)
{
  std::string lower_name = name;
  std::string lower_extension = extension;

  std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
  std::transform(lower_extension.begin(), lower_extension.end(), lower_extension.begin(), ::tolower);

  return m_shaders[lower_extension][lower_name];
}


const unordered_map<std::string, KRShader*>& KRShaderManager::get(const std::string& extension)
{
  std::string lower_extension = extension;
  std::transform(lower_extension.begin(), lower_extension.end(), lower_extension.begin(), ::tolower);
  return m_shaders[lower_extension];
}

// From glslang/StandAlone/ResourceLimits.cpp
const TBuiltInResource DefaultTBuiltInResource = {
  /* .MaxLights = */ 32,
  /* .MaxClipPlanes = */ 6,
  /* .MaxTextureUnits = */ 32,
  /* .MaxTextureCoords = */ 32,
  /* .MaxVertexAttribs = */ 64,
  /* .MaxVertexUniformComponents = */ 4096,
  /* .MaxVaryingFloats = */ 64,
  /* .MaxVertexTextureImageUnits = */ 32,
  /* .MaxCombinedTextureImageUnits = */ 80,
  /* .MaxTextureImageUnits = */ 32,
  /* .MaxFragmentUniformComponents = */ 4096,
  /* .MaxDrawBuffers = */ 32,
  /* .MaxVertexUniformVectors = */ 128,
  /* .MaxVaryingVectors = */ 8,
  /* .MaxFragmentUniformVectors = */ 16,
  /* .MaxVertexOutputVectors = */ 16,
  /* .MaxFragmentInputVectors = */ 15,
  /* .MinProgramTexelOffset = */ -8,
  /* .MaxProgramTexelOffset = */ 7,
  /* .MaxClipDistances = */ 8,
  /* .MaxComputeWorkGroupCountX = */ 65535,
  /* .MaxComputeWorkGroupCountY = */ 65535,
  /* .MaxComputeWorkGroupCountZ = */ 65535,
  /* .MaxComputeWorkGroupSizeX = */ 1024,
  /* .MaxComputeWorkGroupSizeY = */ 1024,
  /* .MaxComputeWorkGroupSizeZ = */ 64,
  /* .MaxComputeUniformComponents = */ 1024,
  /* .MaxComputeTextureImageUnits = */ 16,
  /* .MaxComputeImageUniforms = */ 8,
  /* .MaxComputeAtomicCounters = */ 8,
  /* .MaxComputeAtomicCounterBuffers = */ 1,
  /* .MaxVaryingComponents = */ 60,
  /* .MaxVertexOutputComponents = */ 64,
  /* .MaxGeometryInputComponents = */ 64,
  /* .MaxGeometryOutputComponents = */ 128,
  /* .MaxFragmentInputComponents = */ 128,
  /* .MaxImageUnits = */ 8,
  /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
  /* .MaxCombinedShaderOutputResources = */ 8,
  /* .MaxImageSamples = */ 0,
  /* .MaxVertexImageUniforms = */ 0,
  /* .MaxTessControlImageUniforms = */ 0,
  /* .MaxTessEvaluationImageUniforms = */ 0,
  /* .MaxGeometryImageUniforms = */ 0,
  /* .MaxFragmentImageUniforms = */ 8,
  /* .MaxCombinedImageUniforms = */ 8,
  /* .MaxGeometryTextureImageUnits = */ 16,
  /* .MaxGeometryOutputVertices = */ 256,
  /* .MaxGeometryTotalOutputComponents = */ 1024,
  /* .MaxGeometryUniformComponents = */ 1024,
  /* .MaxGeometryVaryingComponents = */ 64,
  /* .MaxTessControlInputComponents = */ 128,
  /* .MaxTessControlOutputComponents = */ 128,
  /* .MaxTessControlTextureImageUnits = */ 16,
  /* .MaxTessControlUniformComponents = */ 1024,
  /* .MaxTessControlTotalOutputComponents = */ 4096,
  /* .MaxTessEvaluationInputComponents = */ 128,
  /* .MaxTessEvaluationOutputComponents = */ 128,
  /* .MaxTessEvaluationTextureImageUnits = */ 16,
  /* .MaxTessEvaluationUniformComponents = */ 1024,
  /* .MaxTessPatchComponents = */ 120,
  /* .MaxPatchVertices = */ 32,
  /* .MaxTessGenLevel = */ 64,
  /* .MaxViewports = */ 16,
  /* .MaxVertexAtomicCounters = */ 0,
  /* .MaxTessControlAtomicCounters = */ 0,
  /* .MaxTessEvaluationAtomicCounters = */ 0,
  /* .MaxGeometryAtomicCounters = */ 0,
  /* .MaxFragmentAtomicCounters = */ 8,
  /* .MaxCombinedAtomicCounters = */ 8,
  /* .MaxAtomicCounterBindings = */ 1,
  /* .MaxVertexAtomicCounterBuffers = */ 0,
  /* .MaxTessControlAtomicCounterBuffers = */ 0,
  /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
  /* .MaxGeometryAtomicCounterBuffers = */ 0,
  /* .MaxFragmentAtomicCounterBuffers = */ 1,
  /* .MaxCombinedAtomicCounterBuffers = */ 1,
  /* .MaxAtomicCounterBufferSize = */ 16384,
  /* .MaxTransformFeedbackBuffers = */ 4,
  /* .MaxTransformFeedbackInterleavedComponents = */ 64,
  /* .MaxCullDistances = */ 8,
  /* .MaxCombinedClipAndCullDistances = */ 8,
  /* .MaxSamples = */ 4,
  /* .maxMeshOutputVerticesNV = */ 256,
  /* .maxMeshOutputPrimitivesNV = */ 512,
  /* .maxMeshWorkGroupSizeX_NV = */ 32,
  /* .maxMeshWorkGroupSizeY_NV = */ 1,
  /* .maxMeshWorkGroupSizeZ_NV = */ 1,
  /* .maxTaskWorkGroupSizeX_NV = */ 32,
  /* .maxTaskWorkGroupSizeY_NV = */ 1,
  /* .maxTaskWorkGroupSizeZ_NV = */ 1,
  /* .maxMeshViewCountNV = */ 4,
  /* .maxMeshOutputVerticesEXT = */ 256,
  /* .maxMeshOutputPrimitivesEXT = */ 256,
  /* .maxMeshWorkGroupSizeX_EXT = */ 128,
  /* .maxMeshWorkGroupSizeY_EXT = */ 128,
  /* .maxMeshWorkGroupSizeZ_EXT = */ 128,
  /* .maxTaskWorkGroupSizeX_EXT = */ 128,
  /* .maxTaskWorkGroupSizeY_EXT = */ 128,
  /* .maxTaskWorkGroupSizeZ_EXT = */ 128,
  /* .maxMeshViewCountEXT = */ 4,
  /* .maxDualSourceDrawBuffersEXT = */ 1,

  /* .limits = */ {
    /* .nonInductiveForLoops = */ 1,
    /* .whileLoops = */ 1,
    /* .doWhileLoops = */ 1,
    /* .generalUniformIndexing = */ 1,
    /* .generalAttributeMatrixVectorIndexing = */ 1,
    /* .generalVaryingIndexing = */ 1,
    /* .generalSamplerIndexing = */ 1,
    /* .generalVariableIndexing = */ 1,
    /* .generalConstantMatrixVectorIndexing = */ 1,
} };


bool KRShaderManager::compileAll(KRBundle* outputBundle, KRUnknown* logResource)
{
  // TODO - Refactoring / cleanup needed once Vulkan shaders working...
  bool success = true;
  if (!m_initializedGlslang) {
    glslang::InitializeProcess();
    m_initializedGlslang = true;
  }

  KRSourceManager* pSourceManager = getContext().getSourceManager();
  const unordered_map<std::string, KRSource*> vertSources = pSourceManager->get("vert");
  for (const std::pair<const std::string, KRSource*> vertSourceEntry : vertSources) {
    KRSource* vertSource = vertSourceEntry.second;
    KRSource* fragSource = pSourceManager->get(vertSourceEntry.first, "frag");
    const char* programName = vertSourceEntry.first.c_str();

    TBuiltInResource resources;
    resources = DefaultTBuiltInResource;

    EShMessages messages = EShMsgDefault;
    bool desktop = true;
    const int defaultVersion = desktop ? 110 : 100;

    glslang::TShader::ForbidIncluder includer;
    glslang::TShader vertShader(EShLangVertex);
    glslang::TShader fragShader(EShLangFragment);
    vertShader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);
    vertShader.setEnvClient(glslang::EShClientVulkan, glslang::EshTargetClientVersion::EShTargetVulkan_1_1);
    fragShader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);
    fragShader.setEnvClient(glslang::EShClientVulkan, glslang::EshTargetClientVersion::EShTargetVulkan_1_1);
    glslang::TProgram program; // this must be declared after the TShader's to ensure it is deallocated before the TShader's
    if (vertSource) {
      vertSource->getData()->lock();
    }
    if (fragSource) {
      fragSource->getData()->lock();
    }
    const char* vertSourceText[1];
    const char* fragSourceText[1];
    int vertSourceLen[1];
    int fragSourceLen[1];
    const char* vertSourceNameStr[1];
    const char* fragSourceNameStr[1];
    std::string vertSourceName;
    std::string fragSourceName;

    auto parse_shader = [&](KRSource* source, glslang::TShader& shader, const char** sourceText, int* sourceLen, const char** sourceNameStr, std::string& sourceName) {
      if (source == nullptr) {
        return;
      }
      sourceName = source->getName() + "." + source->getExtension();
      sourceText[0] = (char*)source->getData()->getStart();
      sourceLen[0] = source->getData()->getSize();
      sourceNameStr[0] = sourceName.c_str();
      shader.setStringsWithLengthsAndNames(sourceText, sourceLen, sourceNameStr, 1);
      //shader.setStrings(&sourceStr, 1);

      if (shader.parse(&resources, defaultVersion, false, messages, m_includer)) {
        program.addShader(&shader);
      } else {
        const char* log = shader.getInfoLog();
        if (log[0] != '\0') {
          logResource->getData()->append(log);
          logResource->getData()->append("\n");
        }
        success = false;
      }
    };

    parse_shader(vertSource, vertShader, vertSourceText, vertSourceLen, vertSourceNameStr, vertSourceName);
    parse_shader(fragSource, fragShader, fragSourceText, fragSourceLen, fragSourceNameStr, fragSourceName);

    if (!program.link(messages)) {
      const char* log = program.getInfoLog();
      if (log[0] != '\0') {
        logResource->getData()->append(log);
        logResource->getData()->append("\n");
      }
      success = false;
    }

    if (success) {
      for (int stage = 0; stage < EShLangCount; ++stage) {

        if (program.getIntermediate((EShLanguage)stage)) {
          std::vector<unsigned int> spirv;
          spv::SpvBuildLogger logger;
          glslang::SpvOptions spvOptions;
          glslang::GlslangToSpv(*program.getIntermediate((EShLanguage)stage), spirv, &logger, &spvOptions);
          std::string messages = logger.getAllMessages();
          if (!messages.empty()) {
            logResource->getData()->append(messages.c_str());
            logResource->getData()->append("\n");
          }

          std::string shader_name;
          switch (stage) {
          case EShLangVertex:
            shader_name = vertSourceName;
            break;
          case EShLangFragment:
            shader_name = fragSourceName;
            break;
          }
          if (!shader_name.empty()) {
            Block* data = new Block();
            data->append(static_cast<void*>(spirv.data()), spirv.size() * sizeof(unsigned int));
            KRShader* shader = new KRShader(getContext(), shader_name, "spv", data);
            add(shader);
            if (outputBundle) {
              shader->moveToBundle(outputBundle);
            }
          }
        }
      }
    }

    if (vertSource) {
      vertSource->getData()->unlock();
    }
    if (fragSource) {
      fragSource->getData()->unlock();
    }
  }

  return success;
}

KRShaderManager::Includer::Includer(KRContext* context)
  : m_context(context)
{}

glslang::TShader::Includer::IncludeResult* KRShaderManager::Includer::includeSystem(
  const char* headerName,
  const char* includerName,
  size_t inclusionDepth)
{
  fprintf(stderr, "includeSystem(%s, %s, %llu)\n", headerName, includerName, inclusionDepth);
  return nullptr;
}
glslang::TShader::Includer::IncludeResult* KRShaderManager::Includer::includeLocal(
  const char* headerName,
  const char* includerName,
  size_t inclusionDepth)
{
  std::string name = util::GetFileBase(headerName);
  std::string extension = util::GetFileExtension(headerName);
  KRSource* source = m_context->getSourceManager()->get(name, extension);
  if (!source) {
    return nullptr;
  }
  Block* data = source->getData();
  data->lock();
  const char* sourceString = static_cast<const char*>(data->getStart());
  return new IncludeResult(std::string(headerName), sourceString, data->getSize(), static_cast<void*>(data));
}

void KRShaderManager::Includer::releaseInclude(IncludeResult* includeResult)
{
  Block* data = static_cast<Block*>(includeResult->userData);
  data->unlock();
}
