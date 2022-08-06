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
    "material_ambient", // Uniform::material_ambient
    "material_diffuse", // Uniform::material_diffuse
    "material_specular", // Uniform::material_specular
    "material_reflection", // Uniform::material_reflection
    "material_alpha", // Uniform::material_alpha
    "material_shininess", // Uniform::material_shininess
    "light_position", // Uniform::light_position
    "light_direction_model_space", // Uniform::light_direction_model_space
    "light_direction_view_space", // Uniform::light_direction_view_space
    "light_color", //    Uniform::light_color
    "light_decay_start", //    Uniform::light_decay_start
    "light_cutoff", //    Uniform::light_cutoff
    "light_intensity", //    Uniform::light_intensity
    "flare_size", //    Uniform::flare_size
    "view_space_model_origin", //    Uniform::view_space_model_origin
    "mvp_matrix", //    Uniform::mvp
    "inv_projection_matrix", //    Uniform::invp
    "inv_mvp_matrix", //    Uniform::invmvp
    "inv_mvp_matrix_no_translate", //    Uniform::invmvp_no_translate
    "model_view_inverse_transpose_matrix", //    Uniform::model_view_inverse_transpose
    "model_inverse_transpose_matrix", //    Uniform::model_inverse_transpose
    "model_view_matrix", //    Uniform::model_view
    "model_matrix", //    Uniform::model_matrix
    "projection_matrix", //    Uniform::projection_matrix
    "camera_position_model_space", //    Uniform::camerapos_model_space
    "viewport", //    Uniform::viewport
    "viewport_downsample", //     Uniform::viewport_downsample
    "diffuseTexture", //    Uniform::diffusetexture
    "specularTexture", //    Uniform::speculartexture
    "reflectionCubeTexture", //    Uniform::reflectioncubetexture
    "reflectionTexture", //    Uniform::reflectiontexture
    "normalTexture", //    Uniform::normaltexture
    "diffuseTexture_Scale", //    Uniform::diffusetexture_scale
    "specularTexture_Scale", //    Uniform::speculartexture_scale
    "reflectionTexture_Scale", //    Uniform::reflectiontexture_scale
    "normalTexture_Scale", //    Uniform::normaltexture_scale
    "normalTexture_Scale", //    Uniform::ambienttexture_scale
    "diffuseTexture_Offset", //    Uniform::diffusetexture_offset
    "specularTexture_Offset", //    Uniform::speculartexture_offset
    "reflectionTexture_Offset", //    Uniform::reflectiontexture_offset
    "normalTexture_Offset", //    Uniform::normaltexture_offset
    "ambientTexture_Offset", //    Uniform::ambienttexture_offset
    "shadow_mvp1", //    Uniform::shadow_mvp1
    "shadow_mvp2", //    Uniform::shadow_mvp2
    "shadow_mvp3", //    Uniform::shadow_mvp3
    "shadowTexture1", //    Uniform::shadowtexture1
    "shadowTexture2", //    Uniform::shadowtexture2
    "shadowTexture3", //    Uniform::shadowtexture3
    "lightmapTexture", //    Uniform::lightmaptexture
    "gbuffer_frame", //    Uniform::gbuffer_frame
    "gbuffer_depth", //    Uniform::gbuffer_depth
    "depthFrame", //    Uniform::depth_frame
    "volumetricEnvironmentFrame", //    Uniform::volumetric_environment_frame
    "renderFrame", //    Uniform::render_frame
    "time_absolute", //    Uniform::absolute_time
    "fog_near", //    Uniform::fog_near
    "fog_far", //    Uniform::fog_far
    "fog_density", //    Uniform::fog_density
    "fog_color", //    Uniform::fog_color
    "fog_scale", //    Uniform::fog_scale
    "fog_density_premultiplied_exponential", //    Uniform::density_premultiplied_exponential
    "fog_density_premultiplied_squared", //    Uniform::density_premultiplied_squared
    "slice_depth_scale", //    Uniform::slice_depth_scale
    "particle_origin", //    Uniform::particle_origin
    "bone_transforms", //    Uniform::bone_transforms
    "rim_color", // Uniform::rim_color
    "rim_power", // Uniform::rim_power
    "fade_color", // Uniform::fade_color
};

KRPipeline::KRPipeline(KRContext& context, KRSurface& surface, const PipelineInfo& info, const char* szKey, const std::vector<KRShader*>& shaders, uint32_t vertexAttributes, ModelFormat modelFormat)
  : KRContextObject(context)
{
  for (PushConstantStageInfo& pushConstants : m_pushConstants) {
    pushConstants.buffer = nullptr;
    pushConstants.bufferSize = 0;
    memset(pushConstants.size, 0, kUniformCount);
    memset(pushConstants.offset, 0, kUniformCount * sizeof(int));
    pushConstants.layout = nullptr;
  }

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

      initPushConstantStage(ShaderStages::vertex, reflection);

    }
    else if (shader->getSubExtension().compare("frag") == 0) {
      stageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
      initPushConstantStage(ShaderStages::fragment, reflection);
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
  case ModelFormat::KRENGINE_MODEL_FORMAT_INDEXED_TRIANGLES:
  case ModelFormat::KRENGINE_MODEL_FORMAT_TRIANGLES:
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    break;
  case ModelFormat::KRENGINE_MODEL_FORMAT_INDEXED_STRIP:
  case ModelFormat::KRENGINE_MODEL_FORMAT_STRIP:
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
  switch (info.cullMode) {
  case CullMode::kCullBack:
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    break;
  case CullMode::kCullFront:
    rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
    break;
  case CullMode::kCullNone:
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    break;
  }
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
  case RasterMode::kOpaque:
  case RasterMode::kOpaqueLessTest:
  case RasterMode::kOpaqueNoTest:
  case RasterMode::kOpaqueNoDepthWrite:
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    break;
  case RasterMode::kAlphaBlend:
  case RasterMode::kAlphaBlendNoTest:
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    break;
  case RasterMode::kAdditive:
  case RasterMode::kAdditiveNoTest:
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

  int iStage = 0;
  for (PushConstantStageInfo& pushConstants : m_pushConstants) {
    if (pushConstants.buffer) {
      VkPipelineLayoutCreateInfo pushConstantsLayoutInfo{};
      pushConstantsLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
      pushConstantsLayoutInfo.setLayoutCount = 0;
      pushConstantsLayoutInfo.pSetLayouts = nullptr;
      pushConstantsLayoutInfo.pushConstantRangeCount = 0;
      pushConstantsLayoutInfo.pPushConstantRanges = nullptr;

      // TODO - We need to support push constants for other shader stages
      VkPushConstantRange push_constant{};
      push_constant.offset = 0;
      push_constant.size = pushConstants.bufferSize;

      switch (static_cast<ShaderStages>(iStage)) {
      case ShaderStages::vertex:
          push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
          break;
      case ShaderStages::fragment:
          push_constant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
          break;
      case ShaderStages::geometry:
        push_constant.stageFlags = VK_SHADER_STAGE_GEOMETRY_BIT;
        break;
      case ShaderStages::compute:
        push_constant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        break;
      }
      
      pushConstantsLayoutInfo.pPushConstantRanges = &push_constant;
      pushConstantsLayoutInfo.pushConstantRangeCount = 1;

      if (vkCreatePipelineLayout(device->m_logicalDevice, &pushConstantsLayoutInfo, nullptr, &pushConstants.layout) != VK_SUCCESS) {
        // failed! TODO - Error handling
      }
    }
    iStage++;
  }

  VkPipelineDepthStencilStateCreateInfo depthStencil{};
  depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  switch (info.rasterMode) {
  case RasterMode::kOpaque:
  case RasterMode::kOpaqueLessTest:
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    break;
  case RasterMode::kOpaqueNoTest:
    depthStencil.depthTestEnable = VK_FALSE;
    depthStencil.depthWriteEnable = VK_TRUE;
    break;
  case RasterMode::kOpaqueNoDepthWrite:
  case RasterMode::kAlphaBlend:
  case RasterMode::kAdditive:
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_FALSE;
    break;
  case RasterMode::kAlphaBlendNoTest:
  case RasterMode::kAdditiveNoTest:
    depthStencil.depthTestEnable = VK_FALSE;
    depthStencil.depthWriteEnable = VK_FALSE;
    break;
  }

  if (info.rasterMode == RasterMode::kOpaqueLessTest) {
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

KRPipeline::~KRPipeline() {
  if (m_graphicsPipeline) {
    // TODO: vkDestroyPipeline(device, m_graphicsPipeline, nullptr);
  }
  if (m_pipelineLayout) {
    // TODO: vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
  }
  for (PushConstantStageInfo& pushConstants : m_pushConstants) {
    if (pushConstants.layout) {
      // TODO: vkDestroyPipelineLayout(device, pushConstants.layout, nullptr);
    }
  }


  if(getContext().getPipelineManager()->m_active_pipeline == this) {
      getContext().getPipelineManager()->m_active_pipeline = NULL;
  }
  if (m_pushConstants[0].buffer) {
    delete m_pushConstants[0].buffer;
    m_pushConstants[0].buffer = nullptr;
  }
}

void KRPipeline::initPushConstantStage(ShaderStages stage, const SpvReflectShaderModule* reflection)
{
  PushConstantStageInfo& pushConstants = m_pushConstants[static_cast<int>(stage)];
  for (int i = 0; i < reflection->push_constant_block_count; i++) {
    const SpvReflectBlockVariable& block = reflection->push_constant_blocks[i];
    if (stricmp(block.name, "constants") == 0) {
      if (block.size > 0) {
        pushConstants.buffer = (__uint8_t*)malloc(block.size);
        memset(pushConstants.buffer, 0, block.size);
        pushConstants.bufferSize = block.size;

        // Get push constant offsets
        for (int iUniform = 0; iUniform < kUniformCount; iUniform++) {
          for (int iMember = 0; iMember < block.member_count; iMember++) {
            const SpvReflectBlockVariable& member = block.members[iMember];
            if (stricmp(KRENGINE_UNIFORM_NAMES[iUniform], member.name) == 0)
            {
              pushConstants.offset[iUniform] = member.offset;
              pushConstants.size[iUniform] = member.size;
            }
          }
        }
      }
    }
  }
}

bool KRPipeline::hasUniform(Uniform location) const
{
  for (const PushConstantStageInfo& stageConstants : m_pushConstants) {
    if (stageConstants.size[static_cast<size_t>(location)]) {
      return true;
    }
  }
  return false;
}

void KRPipeline::setUniform(Uniform location, float value)
{
  for (PushConstantStageInfo& stageConstants : m_pushConstants) {
    if (stageConstants.size[static_cast<size_t>(location)] == sizeof(value)) {
      float* constant = (float*)(m_pushConstants[0].buffer + stageConstants.offset[static_cast<size_t>(location)]);
      *constant = value;
    }
  }
}


void KRPipeline::setUniform(Uniform location, int value)
{
  for (PushConstantStageInfo& stageConstants : m_pushConstants) {
    if (stageConstants.size[static_cast<size_t>(location)] == sizeof(value)) {
      int* constant = (int*)(m_pushConstants[0].buffer + stageConstants.offset[static_cast<size_t>(location)]);
      *constant = value;
    }
  }
}

void KRPipeline::setUniform(Uniform location, const Vector2 &value)
{
  for (PushConstantStageInfo& stageConstants : m_pushConstants) {
    if (stageConstants.size[static_cast<size_t>(location)] == sizeof(value)) {
      Vector2* constant = (Vector2*)(m_pushConstants[0].buffer + stageConstants.offset[static_cast<size_t>(location)]);
      *constant = value;
    }
  }
}
void KRPipeline::setUniform(Uniform location, const Vector3 &value)
{
  for (PushConstantStageInfo& stageConstants : m_pushConstants) {
    if (stageConstants.size[static_cast<size_t>(location)] == sizeof(value)) {
      Vector3* constant = (Vector3*)(m_pushConstants[0].buffer + stageConstants.offset[static_cast<size_t>(location)]);
      *constant = value;
    }
  }
}

void KRPipeline::setUniform(Uniform location, const Vector4 &value)
{
  for (PushConstantStageInfo& stageConstants : m_pushConstants) {
    if (stageConstants.size[static_cast<size_t>(location)] == sizeof(value)) {
      Vector4* constant = (Vector4*)(m_pushConstants[0].buffer + stageConstants.offset[static_cast<size_t>(location)]);
      *constant = value;
    }
  }
}

void KRPipeline::setUniform(Uniform location, const Matrix4 &value)
{
  for (PushConstantStageInfo& stageConstants : m_pushConstants) {
    if (stageConstants.size[static_cast<size_t>(location)] == sizeof(value)) {
      Matrix4* constant = (Matrix4*)(m_pushConstants[0].buffer + m_pushConstants[0].offset[static_cast<size_t>(location)]);
      *constant = value;
    }
  }
}

void KRPipeline::setUniform(Uniform location, const Matrix4* value, const size_t count)
{
  for (PushConstantStageInfo& stageConstants : m_pushConstants) {
    // TODO - Vulkan refactoring
    // GLDEBUG(glUniformMatrix4fv(pShader->m_pushConstants[0].offset[KRPipeline::Uniform::bone_transforms], (GLsizei)bones.size(), GL_FALSE, bone_mats));
  }
}

bool KRPipeline::bind(VkCommandBuffer& commandBuffer, KRCamera &camera, const KRViewport &viewport, const Matrix4 &matModel, const std::vector<KRPointLight *> *point_lights, const std::vector<KRDirectionalLight *> *directional_lights, const std::vector<KRSpotLight *> *spot_lights, const KRNode::RenderPass &renderPass)
{
    setUniform(Uniform::absolute_time, getContext().getAbsoluteTime());
    
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
            if (hasUniform(Uniform::shadowtexture1) && cShadowBuffers > 0) {
              if (m_pContext->getTextureManager()->selectTexture(GL_TEXTURE_2D, 3, directional_light->getShadowTextures()[0])) {
                GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
                GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
              }

              m_pContext->getTextureManager()->_setWrapModeS(3, GL_CLAMP_TO_EDGE);
              m_pContext->getTextureManager()->_setWrapModeT(3, GL_CLAMP_TO_EDGE);
            }

            if (hasUniform(Uniform::shadowtexture2) && cShadowBuffers > 1 && camera.settings.m_cShadowBuffers > 1) {
              if (m_pContext->getTextureManager()->selectTexture(GL_TEXTURE_2D, 4, directional_light->getShadowTextures()[1])) {
                GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
                GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
              }
              m_pContext->getTextureManager()->_setWrapModeS(4, GL_CLAMP_TO_EDGE);
              m_pContext->getTextureManager()->_setWrapModeT(4, GL_CLAMP_TO_EDGE);
            }

            if (hasUniform(Uniform::shadowtexture3) && cShadowBuffers > 2 && camera.settings.m_cShadowBuffers > 2) {
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
              setUniform(static_cast<Uniform>(static_cast<int>(Uniform::shadow_mvp1) + iShadow), matModel * directional_light->getShadowViewports()[iShadow].getViewProjectionMatrix() * matBias);
            }

            if (hasUniform(Uniform::light_direction_model_space)) {
              Matrix4 inverseModelMatrix = matModel;
              inverseModelMatrix.invert();

              // Bind the light direction vector
              Vector3 lightDirObject = Matrix4::Dot(inverseModelMatrix, directional_light->getWorldLightDirection());
              lightDirObject.normalize();
              setUniform(Uniform::light_direction_model_space, lightDirObject);
            }
          }

          light_directional_count++;
        }
      }

        //light_point_count = point_lights.size();
        //light_spot_count = spot_lights.size();
    }

    if(hasUniform(Uniform::camerapos_model_space)) {
        Matrix4 inverseModelMatrix = matModel;
        inverseModelMatrix.invert();
        
        if(hasUniform(Uniform::camerapos_model_space)) {
            // Transform location of camera to object space for calculation of specular halfVec
            Vector3 cameraPosObject = Matrix4::Dot(inverseModelMatrix, viewport.getCameraPosition());
            setUniform(Uniform::camerapos_model_space, cameraPosObject);
        }
    }
    
    if(hasUniform(Uniform::mvp) || hasUniform(KRPipeline::Uniform::invmvp)) {
        // Bind our modelmatrix variable to be a uniform called mvpmatrix in our shaderprogram
        Matrix4 mvpMatrix = matModel * viewport.getViewProjectionMatrix();
        setUniform(Uniform::mvp, mvpMatrix);
        
        if(hasUniform(KRPipeline::Uniform::invmvp)) {
            setUniform(KRPipeline::Uniform::invmvp, Matrix4::Invert(mvpMatrix));
        }
    }
    
    if(hasUniform(KRPipeline::Uniform::view_space_model_origin) || hasUniform(Uniform::model_view_inverse_transpose) || hasUniform(KRPipeline::Uniform::model_view)) {
        Matrix4 matModelView = matModel * viewport.getViewMatrix();
        setUniform(Uniform::model_view, matModelView);
        
        
        if(hasUniform(KRPipeline::Uniform::view_space_model_origin)) {
            Vector3 view_space_model_origin = Matrix4::Dot(matModelView, Vector3::Zero()); // Origin point of model space is the light source position.  No perspective, so no w divide required
            setUniform(Uniform::view_space_model_origin, view_space_model_origin);
        }
        
        if(hasUniform(Uniform::model_view_inverse_transpose)) {
            Matrix4 matModelViewInverseTranspose = matModelView;
            matModelViewInverseTranspose.transpose();
            matModelViewInverseTranspose.invert();
            setUniform(Uniform::model_view_inverse_transpose, matModelViewInverseTranspose);
        }
    }
    
    if(hasUniform(Uniform::model_inverse_transpose)) {
        Matrix4 matModelInverseTranspose = matModel;
        matModelInverseTranspose.transpose();
        matModelInverseTranspose.invert();
        setUniform(Uniform::model_inverse_transpose, matModelInverseTranspose);
    }
    
    if(hasUniform(KRPipeline::Uniform::invp)) {
        setUniform(Uniform::invp, viewport.getInverseProjectionMatrix());
    }
    
    if(hasUniform(KRPipeline::Uniform::invmvp_no_translate)) {
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
        setUniform(Uniform::invmvp_no_translate, matInvMVPNoTranslate);
    }
    
    setUniform(Uniform::model_matrix, matModel);
    if(hasUniform(Uniform::projection_matrix)) {
        setUniform(Uniform::projection_matrix, viewport.getProjectionMatrix());
    }
    
    if(hasUniform(Uniform::viewport)) {
        setUniform(Uniform::viewport, Vector4::Create(
                (float)0.0,
                (float)0.0,
                (float)viewport.getSize().x,
                (float)viewport.getSize().y
            )
        );
    }
    
    if(hasUniform(Uniform::viewport_downsample)) {
        setUniform(Uniform::viewport_downsample, camera.getDownsample());
    }
    
    // Fog parameters
    setUniform(Uniform::fog_near, camera.settings.fog_near);
    setUniform(Uniform::fog_far, camera.settings.fog_far);
    setUniform(Uniform::fog_density, camera.settings.fog_density);
    setUniform(Uniform::fog_color, camera.settings.fog_color);
    
    if(hasUniform(Uniform::fog_scale)) {
        setUniform(Uniform::fog_scale, 1.0f / (camera.settings.fog_far - camera.settings.fog_near));
    }
    if(hasUniform(Uniform::density_premultiplied_exponential)) {
        setUniform(Uniform::density_premultiplied_exponential, -camera.settings.fog_density * 1.442695f); // -fog_density / log(2)
    }
    if(hasUniform(Uniform::density_premultiplied_squared)) {
        setUniform(Uniform::density_premultiplied_squared, (float)(-camera.settings.fog_density * camera.settings.fog_density * 1.442695)); // -fog_density * fog_density / log(2)
    }
    
    // Sets the diffuseTexture variable to the first texture unit
    setUniform(Uniform::diffusetexture, 0);
    
    // Sets the specularTexture variable to the second texture unit
    setUniform(Uniform::speculartexture, 1);
    
    // Sets the normalTexture variable to the third texture unit
    setUniform(Uniform::normaltexture, 2);
    
    // Sets the shadowTexture variable to the fourth texture unit
    setUniform(Uniform::shadowtexture1, 3);
    setUniform(Uniform::shadowtexture2, 4);
    setUniform(Uniform::shadowtexture3, 5);
    setUniform(Uniform::reflectioncubetexture, 4);
    setUniform(Uniform::lightmaptexture, 5);
    setUniform(Uniform::gbuffer_frame, 6);
    setUniform(Uniform::gbuffer_depth, 7); // Texture unit 7 is used for reading the depth buffer in gBuffer pass #2 and in post-processing pass
    setUniform(Uniform::reflectiontexture, 7); // Texture unit 7 is used for the reflection map textures in gBuffer pass #3 and when using forward rendering
    setUniform(Uniform::depth_frame, 0);
    setUniform(Uniform::render_frame, 1);
    setUniform(Uniform::volumetric_environment_frame, 2);

    for (PushConstantStageInfo& pushConstants : m_pushConstants) {
      if (pushConstants.buffer) {
        vkCmdPushConstants(commandBuffer, pushConstants.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, pushConstants.bufferSize, pushConstants.buffer);
      }
    }

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

    return true;
}

const char *KRPipeline::getKey() const {
    return m_szKey;
}

VkPipeline& KRPipeline::getPipeline()
{
  return m_graphicsPipeline;
}
