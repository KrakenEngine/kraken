//
//  KRPipeline.cpp
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

#include "KRPipeline.h"
#include "assert.h"
#include "KRLight.h"
#include "KRDirectionalLight.h"
#include "KRSpotLight.h"
#include "KRPointLight.h"
#include "KRContext.h"
#include "KRRenderPass.h"


const char *KRPipeline::KRENGINE_UNIFORM_NAMES[] = {
    "material_ambient", // KRENGINE_UNIFORM_MATERIAL_AMBIENT
    "material_diffuse", // KRENGINE_UNIFORM_MATERIAL_DIFFUSE
    "material_specular", // KRENGINE_UNIFORM_MATERIAL_SPECULAR
    "material_reflection", // KRENGINE_UNIFORM_MATERIAL_REFLECTION
    "material_alpha", // KRENGINE_UNIFORM_MATERIAL_ALPHA
    "material_shininess", // KRENGINE_UNIFORM_MATERIAL_SHININESS
    "light_position", // KRENGINE_UNIFORM_LIGHT_POSITION
    "light_direction_model_space", // KRENGINE_UNIFORM_LIGHT_DIRECTION_MODEL_SPACE
    "light_direction_view_space", // KRENGINE_UNIFORM_LIGHT_DIRECTION_VIEW_SPACE
    "light_color", //    KRENGINE_UNIFORM_LIGHT_COLOR
    "light_decay_start", //    KRENGINE_UNIFORM_LIGHT_DECAY_START
    "light_cutoff", //    KRENGINE_UNIFORM_LIGHT_CUTOFF
    "light_intensity", //    KRENGINE_UNIFORM_LIGHT_INTENSITY
    "flare_size", //    KRENGINE_UNIFORM_FLARE_SIZE
    "view_space_model_origin", //    KRENGINE_UNIFORM_VIEW_SPACE_MODEL_ORIGIN
    "mvp_matrix", //    KRENGINE_UNIFORM_MVP
    "inv_projection_matrix", //    KRENGINE_UNIFORM_INVP
    "inv_mvp_matrix", //    KRENGINE_UNIFORM_INVMVP
    "inv_mvp_matrix_no_translate", //    KRENGINE_UNIFORM_INVMVP_NO_TRANSLATE
    "model_view_inverse_transpose_matrix", //    KRENGINE_UNIFORM_MODEL_VIEW_INVERSE_TRANSPOSE
    "model_inverse_transpose_matrix", //    KRENGINE_UNIFORM_MODEL_INVERSE_TRANSPOSE
    "model_view_matrix", //    KRENGINE_UNIFORM_MODEL_VIEW
    "model_matrix", //    KRENGINE_UNIFORM_MODEL_MATRIX
    "projection_matrix", //    KRENGINE_UNIFORM_PROJECTION_MATRIX
    "camera_position_model_space", //    KRENGINE_UNIFORM_CAMERAPOS_MODEL_SPACE
    "viewport", //    KRENGINE_UNIFORM_VIEWPORT
    "viewport_downsample", //     KRENGINE_UNIFORM_VIEWPORT_DOWNSAMPLE
    "diffuseTexture", //    KRENGINE_UNIFORM_DIFFUSETEXTURE
    "specularTexture", //    KRENGINE_UNIFORM_SPECULARTEXTURE
    "reflectionCubeTexture", //    KRENGINE_UNIFORM_REFLECTIONCUBETEXTURE
    "reflectionTexture", //    KRENGINE_UNIFORM_REFLECTIONTEXTURE
    "normalTexture", //    KRENGINE_UNIFORM_NORMALTEXTURE
    "diffuseTexture_Scale", //    KRENGINE_UNIFORM_DIFFUSETEXTURE_SCALE
    "specularTexture_Scale", //    KRENGINE_UNIFORM_SPECULARTEXTURE_SCALE
    "reflectionTexture_Scale", //    KRENGINE_UNIFORM_REFLECTIONTEXTURE_SCALE
    "normalTexture_Scale", //    KRENGINE_UNIFORM_NORMALTEXTURE_SCALE
    "normalTexture_Scale", //    KRENGINE_UNIFORM_AMBIENTTEXTURE_SCALE
    "diffuseTexture_Offset", //    KRENGINE_UNIFORM_DIFFUSETEXTURE_OFFSET
    "specularTexture_Offset", //    KRENGINE_UNIFORM_SPECULARTEXTURE_OFFSET
    "reflectionTexture_Offset", //    KRENGINE_UNIFORM_REFLECTIONTEXTURE_OFFSET
    "normalTexture_Offset", //    KRENGINE_UNIFORM_NORMALTEXTURE_OFFSET
    "ambientTexture_Offset", //    KRENGINE_UNIFORM_AMBIENTTEXTURE_OFFSET
    "shadow_mvp1", //    KRENGINE_UNIFORM_SHADOWMVP1
    "shadow_mvp2", //    KRENGINE_UNIFORM_SHADOWMVP2
    "shadow_mvp3", //    KRENGINE_UNIFORM_SHADOWMVP3
    "shadowTexture1", //    KRENGINE_UNIFORM_SHADOWTEXTURE1
    "shadowTexture2", //    KRENGINE_UNIFORM_SHADOWTEXTURE2
    "shadowTexture3", //    KRENGINE_UNIFORM_SHADOWTEXTURE3
    "lightmapTexture", //    KRENGINE_UNIFORM_LIGHTMAPTEXTURE
    "gbuffer_frame", //    KRENGINE_UNIFORM_GBUFFER_FRAME
    "gbuffer_depth", //    KRENGINE_UNIFORM_GBUFFER_DEPTH
    "depthFrame", //    KRENGINE_UNIFORM_DEPTH_FRAME
    "volumetricEnvironmentFrame", //    KRENGINE_UNIFORM_VOLUMETRIC_ENVIRONMENT_FRAME
    "renderFrame", //    KRENGINE_UNIFORM_RENDER_FRAME
    "time_absolute", //    KRENGINE_UNIFORM_ABSOLUTE_TIME
    "fog_near", //    KRENGINE_UNIFORM_FOG_NEAR
    "fog_far", //    KRENGINE_UNIFORM_FOG_FAR
    "fog_density", //    KRENGINE_UNIFORM_FOG_DENSITY
    "fog_color", //    KRENGINE_UNIFORM_FOG_COLOR
    "fog_scale", //    KRENGINE_UNIFORM_FOG_SCALE
    "fog_density_premultiplied_exponential", //    KRENGINE_UNIFORM_DENSITY_PREMULTIPLIED_EXPONENTIAL
    "fog_density_premultiplied_squared", //    KRENGINE_UNIFORM_DENSITY_PREMULTIPLIED_SQUARED
    "slice_depth_scale", //    KRENGINE_UNIFORM_SLICE_DEPTH_SCALE
    "particle_origin", //    KRENGINE_UNIFORM_PARTICLE_ORIGIN
    "bone_transforms", //    KRENGINE_UNIFORM_BONE_TRANSFORMS
    "rim_color", // KRENGINE_UNIFORM_RIM_COLOR
    "rim_power", // KRENGINE_UNIFORM_RIM_POWER
    "fade_color", // KRENGINE_UNIFORM_FADE_COLOR
};

KRPipeline::KRPipeline(KRContext& context, KRSurface& surface, const PipelineInfo& info, const char* szKey, const std::vector<KRShader*>& shaders, uint32_t vertexAttributes, KRMesh::model_format_t modelFormat)
  : KRContextObject(context)
  , m_iProgram(0) // not used for Vulkan
{
  m_pipelineLayout = nullptr;
  m_graphicsPipeline = nullptr;

  std::unique_ptr<KRDevice>& device = surface.getDevice();
  // TODO - Handle device removal

  strcpy(m_szKey, szKey);

  const int kMaxStages = 4;
  VkPipelineShaderStageCreateInfo stages[kMaxStages];
  memset(static_cast<void*>(stages), 0, sizeof(VkPipelineShaderStageCreateInfo) * kMaxStages);
  size_t stage_count = 0;

  // TODO - Refactor this...  These lookup tables should be in KRMesh...
  static const KRMesh::vertex_attrib_t attribute_mapping[KRMesh::KRENGINE_NUM_ATTRIBUTES] = {
    KRMesh::KRENGINE_ATTRIB_VERTEX,
    KRMesh::KRENGINE_ATTRIB_NORMAL,
    KRMesh::KRENGINE_ATTRIB_TANGENT,
    KRMesh::KRENGINE_ATTRIB_TEXUVA,
    KRMesh::KRENGINE_ATTRIB_TEXUVB,
    KRMesh::KRENGINE_ATTRIB_BONEINDEXES,
    KRMesh::KRENGINE_ATTRIB_BONEWEIGHTS,
    KRMesh::KRENGINE_ATTRIB_VERTEX,
    KRMesh::KRENGINE_ATTRIB_NORMAL,
    KRMesh::KRENGINE_ATTRIB_TANGENT,
    KRMesh::KRENGINE_ATTRIB_TEXUVA,
    KRMesh::KRENGINE_ATTRIB_TEXUVB,
  };

  uint32_t attribute_locations[KRMesh::KRENGINE_NUM_ATTRIBUTES] = {};

  for (KRShader* shader : shaders) {
    VkShaderModule shaderModule;
    if (!shader->createShaderModule(device->m_logicalDevice, shaderModule)) {
      // failed! TODO - Error handling
    }
    const SpvReflectShaderModule* reflection = shader->getReflection();
    VkPipelineShaderStageCreateInfo& stageInfo = stages[stage_count++];
    stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    if (shader->getSubExtension().compare("vert") == 0) {
      stageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

      for (uint32_t i = 0; i < reflection->input_variable_count; i++) {
        // TODO - We should have an interface to allow classes such as KRMesh to expose bindings
        SpvReflectInterfaceVariable& input_var = *reflection->input_variables[i];
        if (strcmp(input_var.name, "vertex_position") == 0) {
          attribute_locations[KRMesh::KRENGINE_ATTRIB_VERTEX] = input_var.location + 1;
        }
        else if (strcmp(input_var.name, "vertex_normal") == 0) {
          attribute_locations[KRMesh::KRENGINE_ATTRIB_NORMAL] = input_var.location + 1;
        }
        else if (strcmp(input_var.name, "vertex_tangent") == 0) {
          attribute_locations[KRMesh::KRENGINE_ATTRIB_TANGENT] = input_var.location + 1;
        }
        else if (strcmp(input_var.name, "vertex_uv") == 0) {
          attribute_locations[KRMesh::KRENGINE_ATTRIB_TEXUVA] = input_var.location + 1;
        }
        else if (strcmp(input_var.name, "vertex_lightmap_uv") == 0) {
          attribute_locations[KRMesh::KRENGINE_ATTRIB_TEXUVB] = input_var.location + 1;
        }
        else if (strcmp(input_var.name, "bone_indexes") == 0) {
          attribute_locations[KRMesh::KRENGINE_ATTRIB_BONEINDEXES] = input_var.location + 1;
        }
        else if (strcmp(input_var.name, "bone_weights") == 0) {
          attribute_locations[KRMesh::KRENGINE_ATTRIB_BONEWEIGHTS] = input_var.location + 1;
        }
      }

    }
    else if (shader->getSubExtension().compare("frag") == 0) {
      stageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    else {
      // failed! TODO - Error handling
    }
    stageInfo.module = shaderModule;
    stageInfo.pName = "main";
  }

  VkVertexInputBindingDescription bindingDescription{};
  bindingDescription.binding = 0;
  bindingDescription.stride = KRMesh::VertexSizeForAttributes(vertexAttributes);
  bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  uint32_t vertexAttributeCount = 0;
  VkVertexInputAttributeDescription vertexAttributeDescriptions[KRMesh::KRENGINE_NUM_ATTRIBUTES]{};

  for (int i = KRMesh::KRENGINE_ATTRIB_VERTEX; i < KRMesh::KRENGINE_NUM_ATTRIBUTES; i++) {
    KRMesh::vertex_attrib_t mesh_attrib = static_cast<KRMesh::vertex_attrib_t>(i);
    int location_attrib = attribute_mapping[i];
    if (KRMesh::has_vertex_attribute(vertexAttributes, (KRMesh::vertex_attrib_t)i) && attribute_locations[location_attrib]) {
      VkVertexInputAttributeDescription& desc = vertexAttributeDescriptions[vertexAttributeCount++];
      desc.binding = 0;
      desc.location = attribute_locations[location_attrib] - 1;
      desc.format = KRMesh::AttributeVulkanFormat(mesh_attrib);
      desc.offset = KRMesh::AttributeOffset(mesh_attrib, vertexAttributes);
    }
  }

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = 1;
  vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
  vertexInputInfo.vertexAttributeDescriptionCount = vertexAttributeCount;
  vertexInputInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions;

  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  switch (modelFormat) {
  case KRMesh::model_format_t::KRENGINE_MODEL_FORMAT_INDEXED_TRIANGLES:
  case KRMesh::model_format_t::KRENGINE_MODEL_FORMAT_TRIANGLES:
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    break;
  case KRMesh::model_format_t::KRENGINE_MODEL_FORMAT_INDEXED_STRIP:
  case KRMesh::model_format_t::KRENGINE_MODEL_FORMAT_STRIP:
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    break;
  }

  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(surface.getWidth());
  viewport.height = static_cast<float>(surface.getHeight());
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.offset = { 0, 0 };
  scissor.extent.width = surface.getWidth();
  scissor.extent.height = surface.getHeight();

  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;
  rasterizer.depthBiasConstantFactor = 0.0f;
  rasterizer.depthBiasClamp = 0.0f;
  rasterizer.depthBiasSlopeFactor = 0.0f;

  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading = 1.0f;
  multisampling.pSampleMask = nullptr;
  multisampling.alphaToCoverageEnable = VK_FALSE;
  multisampling.alphaToOneEnable = VK_FALSE;

  VkPipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

  switch (info.rasterMode) {
  case PipelineInfo::RasterMode::kOpaque:
  case PipelineInfo::RasterMode::kOpaqueLessTest:
  case PipelineInfo::RasterMode::kOpaqueNoTest:
  case PipelineInfo::RasterMode::kOpaqueNoDepthWrite:
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    break;
  case PipelineInfo::RasterMode::kAlphaBlend:
  case PipelineInfo::RasterMode::kAlphaBlendNoTest:
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    break;
  case PipelineInfo::RasterMode::kAdditive:
  case PipelineInfo::RasterMode::kAdditiveNoTest:
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    break;
  }
  colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

  VkPipelineColorBlendStateCreateInfo colorBlending{};
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f;
  colorBlending.blendConstants[1] = 0.0f;
  colorBlending.blendConstants[2] = 0.0f;
  colorBlending.blendConstants[3] = 0.0f;

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;
  pipelineLayoutInfo.pSetLayouts = nullptr;
  pipelineLayoutInfo.pushConstantRangeCount = 0;
  pipelineLayoutInfo.pPushConstantRanges = nullptr;

  if (vkCreatePipelineLayout(device->m_logicalDevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
    // failed! TODO - Error handling
  }

  VkPipelineDepthStencilStateCreateInfo depthStencil{};
  depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  switch (info.rasterMode) {
  case PipelineInfo::RasterMode::kOpaque:
  case PipelineInfo::RasterMode::kOpaqueLessTest:
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    break;
  case PipelineInfo::RasterMode::kOpaqueNoTest:
    depthStencil.depthTestEnable = VK_FALSE;
    depthStencil.depthWriteEnable = VK_TRUE;
    break;
  case PipelineInfo::RasterMode::kOpaqueNoDepthWrite:
  case PipelineInfo::RasterMode::kAlphaBlend:
  case PipelineInfo::RasterMode::kAdditive:
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_FALSE;
    break;
  case PipelineInfo::RasterMode::kAlphaBlendNoTest:
  case PipelineInfo::RasterMode::kAdditiveNoTest:
    depthStencil.depthTestEnable = VK_FALSE;
    depthStencil.depthWriteEnable = VK_FALSE;
    break;
  }

  if (info.rasterMode == PipelineInfo::RasterMode::kOpaqueLessTest) {
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
  } else {
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
  }
  
  depthStencil.depthBoundsTestEnable = VK_FALSE;
  depthStencil.minDepthBounds = 0.0f;
  depthStencil.maxDepthBounds = 1.0f;
  depthStencil.stencilTestEnable = VK_FALSE;
  depthStencil.front = {};
  depthStencil.back = {};

  KRRenderPass& renderPass = surface.getForwardOpaquePass(); // TODO - This needs to be selected dynamically from info.render_pass

  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = stage_count;
  pipelineInfo.pStages = stages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pDepthStencilState = &depthStencil;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = nullptr;
  pipelineInfo.layout = m_pipelineLayout;
  pipelineInfo.renderPass = renderPass.m_renderPass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineInfo.basePipelineIndex = -1;

  if (vkCreateGraphicsPipelines(device->m_logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS) {
    // Failed! TODO - Error handling
  }
}

KRPipeline::KRPipeline(KRContext &context, char *szKey, std::string options, std::string vertShaderSource, const std::string fragShaderSource) : KRContextObject(context)
{
    strcpy(m_szKey, szKey);
    m_iProgram = 0;
    
    
    GLuint vertexShader = 0, fragShader = 0;

    const GLchar *vertSource[2] = {options.c_str(), vertShaderSource.c_str()};
    const GLchar *fragSource[2] = {options.c_str(), fragShaderSource.c_str()};
    
    // Create shader program.
    GLDEBUG(m_iProgram = glCreateProgram());
    
    // Create and compile vertex shader.
    GLDEBUG(vertexShader = glCreateShader(GL_VERTEX_SHADER));
    GLDEBUG(glShaderSource(vertexShader, 2, vertSource, NULL));
    GLDEBUG(glCompileShader(vertexShader));
    
    // Report any compile issues to stderr
    GLint logLength = 0;
    GLDEBUG(glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &logLength));
    if (logLength > 0) {
        GLchar *log = (GLchar *)malloc(logLength + 1);
        assert(log != NULL);
        log[0] = '\0'; // In case glGetShaderInfoLog fails
        GLDEBUG(glGetShaderInfoLog(vertexShader, logLength, &logLength, log));
        log[logLength] = '\0';
        KRContext::Log(KRContext::LOG_LEVEL_ERROR, "KREngine - Failed to compile vertex shader: %s\nShader compile log:\n%s", szKey, log);
        free(log);
    }

    
    // Create and compile vertex shader.
    GLDEBUG(fragShader = glCreateShader(GL_FRAGMENT_SHADER));
    GLDEBUG(glShaderSource(fragShader, 2, fragSource, NULL));
    GLDEBUG(glCompileShader(fragShader));
    
    // Report any compile issues to stderr
    logLength = 0; // In case glGetShaderiv fails
    GLDEBUG(glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &logLength));
    if (logLength > 0) {
        GLchar *log = (GLchar *)malloc(logLength + 1);
        assert(log != NULL);
        log[0] = '\0'; // In case glGetShaderInfoLog fails
        GLDEBUG(glGetShaderInfoLog(fragShader, logLength, &logLength, log));
        log[logLength] = '\0';
        KRContext::Log(KRContext::LOG_LEVEL_ERROR, "KREngine - Failed to compile fragment shader: %s\nShader compile log:\n%s", szKey, log);
        free(log);
    }
    
    // Attach vertex shader to program.
    GLDEBUG(glAttachShader(m_iProgram, vertexShader));
    
    // Attach fragment shader to program.
    GLDEBUG(glAttachShader(m_iProgram, fragShader));
    
    // Bind attribute locations.
    // This needs to be done prior to linking.
    GLDEBUG(glBindAttribLocation(m_iProgram, KRMesh::KRENGINE_ATTRIB_VERTEX, "vertex_position"));
    GLDEBUG(glBindAttribLocation(m_iProgram, KRMesh::KRENGINE_ATTRIB_NORMAL, "vertex_normal"));
    GLDEBUG(glBindAttribLocation(m_iProgram, KRMesh::KRENGINE_ATTRIB_TANGENT, "vertex_tangent"));
    GLDEBUG(glBindAttribLocation(m_iProgram, KRMesh::KRENGINE_ATTRIB_TEXUVA, "vertex_uv"));
    GLDEBUG(glBindAttribLocation(m_iProgram, KRMesh::KRENGINE_ATTRIB_TEXUVB, "vertex_lightmap_uv"));
    GLDEBUG(glBindAttribLocation(m_iProgram, KRMesh::KRENGINE_ATTRIB_BONEINDEXES, "bone_indexes"));
    GLDEBUG(glBindAttribLocation(m_iProgram, KRMesh::KRENGINE_ATTRIB_BONEWEIGHTS, "bone_weights"));
    
    // Link program.
    GLDEBUG(glLinkProgram(m_iProgram));
    
    GLint link_success = GL_FALSE;
    GLDEBUG(glGetProgramiv(m_iProgram, GL_LINK_STATUS, &link_success));
    
    if(link_success != GL_TRUE) {
        // Report any linking issues to stderr
        KRContext::Log(KRContext::LOG_LEVEL_ERROR, "KREngine - Failed to link shader program: %s", szKey);
        logLength = 0; // In case glGetProgramiv fails
        GLDEBUG(glGetProgramiv(m_iProgram, GL_INFO_LOG_LENGTH, &logLength));
        if (logLength > 0)
        {
            GLchar *log = (GLchar *)malloc(logLength + 1);
            assert(log != NULL);
            log[0] = '\0'; // In case glGetProgramInfoLog fails
            GLDEBUG(glGetProgramInfoLog(m_iProgram, logLength, &logLength, log));
            log[logLength] = '\0';
            KRContext::Log(KRContext::LOG_LEVEL_ERROR, "Program link log:\n%s", log);
            free(log);
        }
        GLDEBUG(glDeleteProgram(m_iProgram));
        m_iProgram = 0;
    } else {
    
        // Get uniform locations
        for(int i=0; i < KRENGINE_NUM_UNIFORMS; i++ ){
            GLDEBUG(m_uniforms[i] = glGetUniformLocation(m_iProgram, KRENGINE_UNIFORM_NAMES[i]));
            m_uniform_value_index[i] = -1;
        }
    }

    // Release vertex and fragment shaders.
    if (vertexShader) {
        GLDEBUG(glDeleteShader(vertexShader));
	}
    if (fragShader) {
        GLDEBUG(glDeleteShader(fragShader));
	}
}

KRPipeline::~KRPipeline() {
  if (m_graphicsPipeline) {
    // TODO: vkDestroyPipeline(device, m_graphicsPipeline, nullptr);
  }
  if (m_pipelineLayout) {
    // TODO: vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
  }

    if(m_iProgram) {
        GLDEBUG(glDeleteProgram(m_iProgram));
        if(getContext().getPipelineManager()->m_active_pipeline == this) {
            getContext().getPipelineManager()->m_active_pipeline = NULL;
        }
    }
}

void KRPipeline::setUniform(int location, float value)
{
    if(m_uniforms[location] != -1) {
        int value_index = m_uniform_value_index[location];
        bool needs_update = true;
        if(value_index == -1) {
            m_uniform_value_index[location] = (int)m_uniform_value_float.size();
            m_uniform_value_float.push_back(value);
        } else if(m_uniform_value_float[value_index] == value) {
            needs_update = false;
        } else {
            m_uniform_value_float[value_index] = value;
        }
        if(needs_update) {
            GLDEBUG(glUniform1f(m_uniforms[location], value));
        }
    }
}
void KRPipeline::setUniform(int location, int value)
{
    if(m_uniforms[location] != -1) {
        int value_index = m_uniform_value_index[location];
        bool needs_update = true;
        if(value_index == -1) {
            m_uniform_value_index[location] = (int)m_uniform_value_int.size();
            m_uniform_value_int.push_back(value);
        } else if(m_uniform_value_int[value_index] == value) {
            needs_update = false;
        } else {
            m_uniform_value_int[value_index] = value;
        }
        if(needs_update) {
            GLDEBUG(glUniform1i(m_uniforms[location], value));
        }
    }
}

void KRPipeline::setUniform(int location, const Vector2 &value)
{
    if(m_uniforms[location] != -1) {
        int value_index = m_uniform_value_index[location];
        bool needs_update = true;
        if(value_index == -1) {
            m_uniform_value_index[location] = (int)m_uniform_value_vector2.size();
            m_uniform_value_vector2.push_back(value);
        } else if(m_uniform_value_vector2[value_index] == value) {
            needs_update = false;
        } else {
            m_uniform_value_vector2[value_index] = value;
        }
        if(needs_update) {
            GLDEBUG(glUniform2f(m_uniforms[location], value.x, value.y));
        }
    }
}
void KRPipeline::setUniform(int location, const Vector3 &value)
{
    if(m_uniforms[location] != -1) {
        int value_index = m_uniform_value_index[location];
        bool needs_update = true;
        if(value_index == -1) {
            m_uniform_value_index[location] = (int)m_uniform_value_vector3.size();
            m_uniform_value_vector3.push_back(value);
        } else if(m_uniform_value_vector3[value_index] == value) {
            needs_update = false;
        } else {
            m_uniform_value_vector3[value_index] = value;
        }
        if(needs_update) {
            GLDEBUG(glUniform3f(m_uniforms[location], value.x, value.y, value.z));
        }
    }
}
void KRPipeline::setUniform(int location, const Vector4 &value)
{
    if(m_uniforms[location] != -1) {
        int value_index = m_uniform_value_index[location];
        bool needs_update = true;
        if(value_index == -1) {
            m_uniform_value_index[location] = (int)m_uniform_value_vector4.size();
            m_uniform_value_vector4.push_back(value);
        } else if(m_uniform_value_vector4[value_index] == value) {
            needs_update = false;
        } else {
            m_uniform_value_vector4[value_index] = value;
        }
        if(needs_update) {
            GLDEBUG(glUniform4f(m_uniforms[location], value.x, value.y, value.z, value.w));
        }
    }
}

void KRPipeline::setUniform(int location, const Matrix4 &value)
{
    if(m_uniforms[location] != -1) {
        int value_index = m_uniform_value_index[location];
        bool needs_update = true;
        if(value_index == -1) {
            m_uniform_value_index[location] = (int)m_uniform_value_mat4.size();
            m_uniform_value_mat4.push_back(value);
        } else if(m_uniform_value_mat4[value_index] == value) {
            needs_update = false;
        } else {
            m_uniform_value_mat4[value_index] = value;
        }
        if(needs_update) {
            GLDEBUG(glUniformMatrix4fv(m_uniforms[location], 1, GL_FALSE, value.c));
        }
    }
}

void KRPipeline::bind(VkCommandBuffer& commandBuffer)
{
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
}

bool KRPipeline::bind(KRCamera &camera, const KRViewport &viewport, const Matrix4 &matModel, const std::vector<KRPointLight *> *point_lights, const std::vector<KRDirectionalLight *> *directional_lights, const std::vector<KRSpotLight *> *spot_lights, const KRNode::RenderPass &renderPass, const Vector3 &rim_color, float rim_power, const Vector4 &fade_color) {
    if(m_iProgram == 0) {
        return false;
    }
    
    bool shander_changed = false;
    if(getContext().getPipelineManager()->m_active_pipeline != this) {
        getContext().getPipelineManager()->m_active_pipeline = this;
        GLDEBUG(glUseProgram(m_iProgram));
        shander_changed = true;
    }
    
    
    setUniform(KRENGINE_UNIFORM_ABSOLUTE_TIME, getContext().getAbsoluteTime());
    
    int light_directional_count = 0;
    //int light_point_count = 0;
    //int light_spot_count = 0;
    // TODO - Need to support multiple lights and more light types in forward rendering
    if(renderPass != KRNode::RENDER_PASS_DEFERRED_LIGHTS && renderPass != KRNode::RENDER_PASS_DEFERRED_GBUFFER && renderPass != KRNode::RENDER_PASS_DEFERRED_OPAQUE && renderPass != KRNode::RENDER_PASS_GENERATE_SHADOWMAPS) {
        
        
      if (directional_lights) {
        for (std::vector<KRDirectionalLight*>::const_iterator light_itr = directional_lights->begin(); light_itr != directional_lights->end(); light_itr++) {
          KRDirectionalLight* directional_light = (*light_itr);
          if (light_directional_count == 0) {
            int cShadowBuffers = directional_light->getShadowBufferCount();
            if (m_uniforms[KRENGINE_UNIFORM_SHADOWTEXTURE1] != -1 && cShadowBuffers > 0) {
              if (m_pContext->getTextureManager()->selectTexture(GL_TEXTURE_2D, 3, directional_light->getShadowTextures()[0])) {
                GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
                GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
              }

              m_pContext->getTextureManager()->_setWrapModeS(3, GL_CLAMP_TO_EDGE);
              m_pContext->getTextureManager()->_setWrapModeT(3, GL_CLAMP_TO_EDGE);
            }

            if (m_uniforms[KRENGINE_UNIFORM_SHADOWTEXTURE2] != -1 && cShadowBuffers > 1 && camera.settings.m_cShadowBuffers > 1) {
              if (m_pContext->getTextureManager()->selectTexture(GL_TEXTURE_2D, 4, directional_light->getShadowTextures()[1])) {
                GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
                GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
              }
              m_pContext->getTextureManager()->_setWrapModeS(4, GL_CLAMP_TO_EDGE);
              m_pContext->getTextureManager()->_setWrapModeT(4, GL_CLAMP_TO_EDGE);
            }

            if (m_uniforms[KRENGINE_UNIFORM_SHADOWTEXTURE3] != -1 && cShadowBuffers > 2 && camera.settings.m_cShadowBuffers > 2) {
              if (m_pContext->getTextureManager()->selectTexture(GL_TEXTURE_2D, 5, directional_light->getShadowTextures()[2])) {
                GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
                GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
              }
              m_pContext->getTextureManager()->_setWrapModeS(5, GL_CLAMP_TO_EDGE);
              m_pContext->getTextureManager()->_setWrapModeT(5, GL_CLAMP_TO_EDGE);
            }

            Matrix4 matBias;
            matBias.translate(1.0, 1.0, 1.0);
            matBias.scale(0.5);
            for (int iShadow = 0; iShadow < cShadowBuffers; iShadow++) {
              setUniform(KRENGINE_UNIFORM_SHADOWMVP1 + iShadow, matModel * directional_light->getShadowViewports()[iShadow].getViewProjectionMatrix() * matBias);
            }

            if (m_uniforms[KRENGINE_UNIFORM_LIGHT_DIRECTION_MODEL_SPACE] != -1) {
              Matrix4 inverseModelMatrix = matModel;
              inverseModelMatrix.invert();

              // Bind the light direction vector
              Vector3 lightDirObject = Matrix4::Dot(inverseModelMatrix, directional_light->getWorldLightDirection());
              lightDirObject.normalize();
              setUniform(KRENGINE_UNIFORM_LIGHT_DIRECTION_MODEL_SPACE, lightDirObject);
            }
          }

          light_directional_count++;
        }
      }

        //light_point_count = point_lights.size();
        //light_spot_count = spot_lights.size();
    }
    

    
    if(m_uniforms[KRENGINE_UNIFORM_CAMERAPOS_MODEL_SPACE] != -1) {
        Matrix4 inverseModelMatrix = matModel;
        inverseModelMatrix.invert();
        
        if(m_uniforms[KRENGINE_UNIFORM_CAMERAPOS_MODEL_SPACE] != -1) {
            // Transform location of camera to object space for calculation of specular halfVec
            Vector3 cameraPosObject = Matrix4::Dot(inverseModelMatrix, viewport.getCameraPosition());
            setUniform(KRENGINE_UNIFORM_CAMERAPOS_MODEL_SPACE, cameraPosObject);
        }
    }
    
    if(m_uniforms[KRENGINE_UNIFORM_MVP] != -1 || m_uniforms[KRPipeline::KRENGINE_UNIFORM_INVMVP] != -1) {
        // Bind our modelmatrix variable to be a uniform called mvpmatrix in our shaderprogram
        Matrix4 mvpMatrix = matModel * viewport.getViewProjectionMatrix();
        setUniform(KRENGINE_UNIFORM_MVP, mvpMatrix);
        
        if(m_uniforms[KRPipeline::KRENGINE_UNIFORM_INVMVP] != -1) {
            setUniform(KRPipeline::KRENGINE_UNIFORM_INVMVP, Matrix4::Invert(mvpMatrix));
        }
    }
    
    if(m_uniforms[KRPipeline::KRENGINE_UNIFORM_VIEW_SPACE_MODEL_ORIGIN] != -1 || m_uniforms[KRENGINE_UNIFORM_MODEL_VIEW_INVERSE_TRANSPOSE] != -1 || m_uniforms[KRPipeline::KRENGINE_UNIFORM_MODEL_VIEW] != -1) {
        Matrix4 matModelView = matModel * viewport.getViewMatrix();
        setUniform(KRENGINE_UNIFORM_MODEL_VIEW, matModelView);
        
        
        if(m_uniforms[KRPipeline::KRENGINE_UNIFORM_VIEW_SPACE_MODEL_ORIGIN] != -1) {
            Vector3 view_space_model_origin = Matrix4::Dot(matModelView, Vector3::Zero()); // Origin point of model space is the light source position.  No perspective, so no w divide required
            setUniform(KRENGINE_UNIFORM_VIEW_SPACE_MODEL_ORIGIN, view_space_model_origin);
        }
        
        if(m_uniforms[KRENGINE_UNIFORM_MODEL_VIEW_INVERSE_TRANSPOSE] != -1) {
            Matrix4 matModelViewInverseTranspose = matModelView;
            matModelViewInverseTranspose.transpose();
            matModelViewInverseTranspose.invert();
            setUniform(KRENGINE_UNIFORM_MODEL_VIEW_INVERSE_TRANSPOSE, matModelViewInverseTranspose);
        }
    }
    
    if(m_uniforms[KRENGINE_UNIFORM_MODEL_INVERSE_TRANSPOSE] != -1) {
        Matrix4 matModelInverseTranspose = matModel;
        matModelInverseTranspose.transpose();
        matModelInverseTranspose.invert();
        setUniform(KRENGINE_UNIFORM_MODEL_INVERSE_TRANSPOSE, matModelInverseTranspose);
    }
    
    if(m_uniforms[KRPipeline::KRENGINE_UNIFORM_INVP] != -1) {
        setUniform(KRENGINE_UNIFORM_INVP, viewport.getInverseProjectionMatrix());
    }
    
    if(m_uniforms[KRPipeline::KRENGINE_UNIFORM_INVMVP_NO_TRANSLATE] != -1) {
        Matrix4 matInvMVPNoTranslate = matModel * viewport.getViewMatrix();;
        // Remove the translation
        matInvMVPNoTranslate.getPointer()[3] = 0;
        matInvMVPNoTranslate.getPointer()[7] = 0;
        matInvMVPNoTranslate.getPointer()[11] = 0;
        matInvMVPNoTranslate.getPointer()[12] = 0;
        matInvMVPNoTranslate.getPointer()[13] = 0;
        matInvMVPNoTranslate.getPointer()[14] = 0;
        matInvMVPNoTranslate.getPointer()[15] = 1.0;
        matInvMVPNoTranslate = matInvMVPNoTranslate * viewport.getProjectionMatrix();
        matInvMVPNoTranslate.invert();
        setUniform(KRENGINE_UNIFORM_INVMVP_NO_TRANSLATE, matInvMVPNoTranslate);
    }
    
    setUniform(KRENGINE_UNIFORM_MODEL_MATRIX, matModel);
    if(m_uniforms[KRENGINE_UNIFORM_PROJECTION_MATRIX] != -1) {
        setUniform(KRENGINE_UNIFORM_PROJECTION_MATRIX, viewport.getProjectionMatrix());
    }
    
    if(m_uniforms[KRENGINE_UNIFORM_VIEWPORT] != -1) {
        setUniform(KRENGINE_UNIFORM_VIEWPORT, Vector4::Create(
                (float)0.0,
                (float)0.0,
                (float)viewport.getSize().x,
                (float)viewport.getSize().y
            )
        );
    }
    
    if(m_uniforms[KRENGINE_UNIFORM_VIEWPORT_DOWNSAMPLE] != -1) {
        setUniform(KRENGINE_UNIFORM_VIEWPORT_DOWNSAMPLE, camera.getDownsample());
    }
    
    // Rim highlighting parameters
    setUniform(KRENGINE_UNIFORM_RIM_COLOR, rim_color);
    setUniform(KRENGINE_UNIFORM_RIM_POWER, rim_power);
    
    // Fade parameters
    setUniform(KRENGINE_UNIFORM_FADE_COLOR, fade_color);
    
    // Fog parameters
    setUniform(KRENGINE_UNIFORM_FOG_NEAR, camera.settings.fog_near);
    setUniform(KRENGINE_UNIFORM_FOG_FAR, camera.settings.fog_far);
    setUniform(KRENGINE_UNIFORM_FOG_DENSITY, camera.settings.fog_density);
    setUniform(KRENGINE_UNIFORM_FOG_COLOR, camera.settings.fog_color);
    
    if(m_uniforms[KRENGINE_UNIFORM_FOG_SCALE] != -1) {
        setUniform(KRENGINE_UNIFORM_FOG_SCALE, 1.0f / (camera.settings.fog_far - camera.settings.fog_near));
    }
    if(m_uniforms[KRENGINE_UNIFORM_DENSITY_PREMULTIPLIED_EXPONENTIAL] != -1) {
        setUniform(KRENGINE_UNIFORM_DENSITY_PREMULTIPLIED_EXPONENTIAL, -camera.settings.fog_density * 1.442695f); // -fog_density / log(2)
    }
    if(m_uniforms[KRENGINE_UNIFORM_DENSITY_PREMULTIPLIED_SQUARED] != -1) {
        setUniform(KRENGINE_UNIFORM_DENSITY_PREMULTIPLIED_SQUARED, (float)(-camera.settings.fog_density * camera.settings.fog_density * 1.442695)); // -fog_density * fog_density / log(2)
    }
    
    // Sets the diffuseTexture variable to the first texture unit
    setUniform(KRENGINE_UNIFORM_DIFFUSETEXTURE, 0);
    
    // Sets the specularTexture variable to the second texture unit
    setUniform(KRENGINE_UNIFORM_SPECULARTEXTURE, 1);
    
    // Sets the normalTexture variable to the third texture unit
    setUniform(KRENGINE_UNIFORM_NORMALTEXTURE, 2);
    
    // Sets the shadowTexture variable to the fourth texture unit
    setUniform(KRENGINE_UNIFORM_SHADOWTEXTURE1, 3);
    setUniform(KRENGINE_UNIFORM_SHADOWTEXTURE2, 4);
    setUniform(KRENGINE_UNIFORM_SHADOWTEXTURE3, 5);
    setUniform(KRENGINE_UNIFORM_REFLECTIONCUBETEXTURE, 4);
    setUniform(KRENGINE_UNIFORM_LIGHTMAPTEXTURE, 5);
    setUniform(KRENGINE_UNIFORM_GBUFFER_FRAME, 6);
    setUniform(KRENGINE_UNIFORM_GBUFFER_DEPTH, 7); // Texture unit 7 is used for reading the depth buffer in gBuffer pass #2 and in post-processing pass
    setUniform(KRENGINE_UNIFORM_REFLECTIONTEXTURE, 7); // Texture unit 7 is used for the reflection map textures in gBuffer pass #3 and when using forward rendering
    setUniform(KRENGINE_UNIFORM_DEPTH_FRAME, 0);
    setUniform(KRENGINE_UNIFORM_RENDER_FRAME, 1);
    setUniform(KRENGINE_UNIFORM_VOLUMETRIC_ENVIRONMENT_FRAME, 2);
    
#if defined(DEBUG)
    if(shander_changed) { // FINDME!! KIP!! HACK!!
        GLint logLength;
        
        GLint validate_status = GL_FALSE;
        GLDEBUG(glValidateProgram(m_iProgram));
        GLDEBUG(glGetProgramiv(m_iProgram, GL_VALIDATE_STATUS, &validate_status));
        if(validate_status != GL_TRUE) {
            KRContext::Log(KRContext::LOG_LEVEL_ERROR, "KREngine - Failed to validate shader program: %s", m_szKey);
            logLength = 0; // In case glGetProgramiv fails
            GLDEBUG(glGetProgramiv(m_iProgram, GL_INFO_LOG_LENGTH, &logLength));
            if (logLength > 0)
            {
                GLchar *log = (GLchar *)malloc(logLength + 1);
                assert(log != NULL);
                log[0] = '\0'; // In case glGetProgramInfoLog fails
                GLDEBUG(glGetProgramInfoLog(m_iProgram, logLength, &logLength, log));
                log[logLength] = '\0';
                KRContext::Log(KRContext::LOG_LEVEL_ERROR, "Program validate log:\n%s", log);
                free(log);
                
            }
            return false;
        }
    }
#endif

    return true;
}

const char *KRPipeline::getKey() const {
    return m_szKey;
}

VkPipeline& KRPipeline::getPipeline()
{
  return m_graphicsPipeline;
}
