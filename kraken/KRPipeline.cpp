//
//  KRPipeline.cpp
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

#include "KRPipeline.h"
#include "assert.h"
#include "nodes/KRLight.h"
#include "nodes/KRDirectionalLight.h"
#include "nodes/KRSpotLight.h"
#include "nodes/KRPointLight.h"
#include "KRContext.h"
#include "KRRenderPass.h"
#include "KRModelView.h"

using namespace hydra;

KRPipeline::KRPipeline(KRContext& context, KrDeviceHandle deviceHandle, const KRRenderPass* renderPass, Vector2i viewport_size, Vector2i scissor_size, const PipelineInfo& info, const char* szKey, const std::vector<KRShader*>& shaders, uint32_t vertexAttributes, ModelFormat modelFormat)
  : KRContextObject(context)
  , m_deviceHandle(deviceHandle)
{
  std::unique_ptr<KRDevice>& device = getContext().getDeviceManager()->getDevice(m_deviceHandle);

  for (StageInfo& stageInfo : m_stages) {
    PushConstantInfo& pushConstants = stageInfo.pushConstants;
    pushConstants.buffer = nullptr;
    pushConstants.bufferSize = 0;
    memset(pushConstants.size, 0, kPushConstantCount);
    memset(pushConstants.offset, 0, kPushConstantCount * sizeof(int));
    pushConstants.layout = nullptr;
  }

  m_descriptorSetLayout = nullptr;
  m_pipelineLayout = nullptr;
  m_graphicsPipeline = nullptr;
  m_descriptorSets.reserve(KRENGINE_MAX_FRAMES_IN_FLIGHT);

  // TODO - Handle device removal

  strcpy(m_szKey, szKey);

  const int kMaxStages = 4;
  VkPipelineShaderStageCreateInfo stages[kMaxStages];
  memset(static_cast<void*>(stages), 0, sizeof(VkPipelineShaderStageCreateInfo) * kMaxStages);
  size_t stage_count = 0;

  std::vector<VkDescriptorSetLayoutBinding> uboLayoutBindings;

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

  uint32_t layout_binding_count = 0;
  for (KRShader* shader : shaders) {
    const SpvReflectShaderModule* reflection = shader->getReflection();
    layout_binding_count += reflection->descriptor_binding_count;
  }
  uboLayoutBindings.reserve(layout_binding_count);

  for (KRShader* shader : shaders) {
    VkShaderModule shaderModule;
    if (!shader->createShaderModule(device->m_logicalDevice, shaderModule)) {
      // failed! TODO - Error handling
    }
    const SpvReflectShaderModule* reflection = shader->getReflection();
    
    for (uint32_t b = 0; b < reflection->descriptor_binding_count; b++) {
      SpvReflectDescriptorBinding& binding_reflect = reflection->descriptor_bindings[b];
      VkDescriptorSetLayoutBinding& binding = uboLayoutBindings.emplace_back();
      memset(&binding, 0, sizeof(VkDescriptorSetLayoutBinding));
      binding.binding = binding_reflect.binding;
      // Note: VkDescriptorType and SpvReflectDescriptorType values match
      binding.descriptorType = static_cast<VkDescriptorType>(binding_reflect.descriptor_type);
      binding.descriptorCount = binding_reflect.count;
      binding.pImmutableSamplers = nullptr;
      binding.stageFlags = shader->getShaderStageFlagBits();
    }

    VkPipelineShaderStageCreateInfo& stageInfo = stages[stage_count++];
    stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo.stage = shader->getShaderStageFlagBits();
    if (stageInfo.stage == VK_SHADER_STAGE_VERTEX_BIT) {

      for (uint32_t i = 0; i < reflection->input_variable_count; i++) {
        // TODO - We should have an interface to allow classes such as KRMesh to expose bindings
        SpvReflectInterfaceVariable& input_var = *reflection->input_variables[i];
        if (strcmp(input_var.name, "vertex_position") == 0) {
          attribute_locations[KRMesh::KRENGINE_ATTRIB_VERTEX] = input_var.location + 1;
        } else if (strcmp(input_var.name, "vertex_normal") == 0) {
          attribute_locations[KRMesh::KRENGINE_ATTRIB_NORMAL] = input_var.location + 1;
        } else if (strcmp(input_var.name, "vertex_tangent") == 0) {
          attribute_locations[KRMesh::KRENGINE_ATTRIB_TANGENT] = input_var.location + 1;
        } else if (strcmp(input_var.name, "vertex_uv") == 0) {
          attribute_locations[KRMesh::KRENGINE_ATTRIB_TEXUVA] = input_var.location + 1;
        } else if (strcmp(input_var.name, "vertex_lightmap_uv") == 0) {
          attribute_locations[KRMesh::KRENGINE_ATTRIB_TEXUVB] = input_var.location + 1;
        } else if (strcmp(input_var.name, "bone_indexes") == 0) {
          attribute_locations[KRMesh::KRENGINE_ATTRIB_BONEINDEXES] = input_var.location + 1;
        } else if (strcmp(input_var.name, "bone_weights") == 0) {
          attribute_locations[KRMesh::KRENGINE_ATTRIB_BONEWEIGHTS] = input_var.location + 1;
        }
      }
    }

    initPushConstantStage(shader->getShaderStage(), reflection);
    initDescriptorSetStage(shader->getShaderStage(), reflection);
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
  viewport.width = static_cast<float>(viewport_size.x);
  viewport.height = static_cast<float>(viewport_size.y);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.offset = { 0, 0 };
  scissor.extent.width = scissor_size.x;
  scissor.extent.height = scissor_size.y;

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

  if (uboLayoutBindings.size()) {
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = uboLayoutBindings.size();
    layoutInfo.pBindings = uboLayoutBindings.data();

    if (vkCreateDescriptorSetLayout(device->m_logicalDevice, &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
      // failed! TODO - Error handling
    }
  }

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  if (uboLayoutBindings.size()) {
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
  } else {
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
  }
  pipelineLayoutInfo.pushConstantRangeCount = 0;
  pipelineLayoutInfo.pPushConstantRanges = nullptr;

  if (vkCreatePipelineLayout(device->m_logicalDevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
    // failed! TODO - Error handling
  }

  int iStage = 0;
  for (StageInfo& stageInfo : m_stages) {
    PushConstantInfo& pushConstants = stageInfo.pushConstants;
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
      push_constant.stageFlags = getShaderStageFlagBitsFromShaderStage(static_cast<ShaderStage>(iStage));
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
  pipelineInfo.renderPass = renderPass->m_renderPass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineInfo.basePipelineIndex = -1;

  if (vkCreateGraphicsPipelines(device->m_logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS) {
    // Failed! TODO - Error handling
  }
}

KRPipeline::~KRPipeline()
{
  if (m_graphicsPipeline) {
    // TODO: vkDestroyPipeline(device, m_graphicsPipeline, nullptr);
  }
  if (m_pipelineLayout) {
    // TODO: vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
  }
  if (m_descriptorSetLayout) {
    // TODO: vkDestroyDescriptorSetLayout(device, m_descriptorSetLayout, nullptr);
  }

  if (getContext().getPipelineManager()->m_active_pipeline == this) {
    getContext().getPipelineManager()->m_active_pipeline = NULL;
  }
  for (StageInfo& stageInfo : m_stages) {
    PushConstantInfo& pushConstants = stageInfo.pushConstants;
    if (pushConstants.layout) {
      // TODO: vkDestroyPipelineLayout(device, pushConstants.layout, nullptr);
    }
    if (pushConstants.buffer) {
      delete pushConstants.buffer;
      pushConstants.buffer = nullptr;
    }
  }
}

void KRPipeline::initPushConstantStage(ShaderStage stage, const SpvReflectShaderModule* reflection)
{
  PushConstantInfo& pushConstants = m_stages[static_cast<int>(stage)].pushConstants;
  for (int i = 0; i < reflection->push_constant_block_count; i++) {
    const SpvReflectBlockVariable& block = reflection->push_constant_blocks[i];
    if (stricmp(block.name, "PushConstants") == 0) {
      if (block.size > 0) {
        pushConstants.buffer = (__uint8_t*)malloc(block.size);
        memset(pushConstants.buffer, 0, block.size);
        pushConstants.bufferSize = block.size;

        // Get push constant offsets
        for (int iUniform = 0; iUniform < kPushConstantCount; iUniform++) {
          for (int iMember = 0; iMember < block.member_count; iMember++) {
            const SpvReflectBlockVariable& member = block.members[iMember];
            if (stricmp(SHADER_VALUE_NAMES[iUniform], member.name) == 0) {
              pushConstants.offset[iUniform] = member.offset;
              pushConstants.size[iUniform] = member.size;
              if (member.type_description->op == SpvOpTypeVector && member.numeric.scalar.width == 32) {
                switch (member.numeric.vector.component_count) {
                case 2:
                  pushConstants.type[iUniform] = ShaderValueType::type_vector2;
                  break;
                case 3:
                  pushConstants.type[iUniform] = ShaderValueType::type_vector3;
                  break;
                case 4:
                  pushConstants.type[iUniform] = ShaderValueType::type_vector4;
                  break;
                }
              } else if (member.type_description->op == SpvOpTypeFloat && member.numeric.scalar.width == 32) {
                pushConstants.type[iUniform] = ShaderValueType::type_float32;
              } else if (member.type_description->op == SpvOpTypeFloat && member.numeric.scalar.width == 64) {
                pushConstants.type[iUniform] = ShaderValueType::type_float64;
              } else if (member.type_description->op == SpvOpTypeInt && member.numeric.scalar.width == 32) {
                pushConstants.type[iUniform] = ShaderValueType::type_int32;
              } else if (member.type_description->op == SpvOpTypeInt && member.numeric.scalar.width == 64) {
                pushConstants.type[iUniform] = ShaderValueType::type_int64;
              } else if (member.type_description->op == SpvOpTypeMatrix && member.numeric.scalar.width == 32) {
                if (member.numeric.matrix.column_count == 2 && member.numeric.matrix.row_count == 2) {
                  pushConstants.type[iUniform] = ShaderValueType::type_matrix2;
                } else if (member.numeric.matrix.column_count == 2 && member.numeric.matrix.row_count == 3) {
                  pushConstants.type[iUniform] = ShaderValueType::type_matrix2x3;
                } else if (member.numeric.matrix.column_count == 4 && member.numeric.matrix.row_count == 4) {
                  pushConstants.type[iUniform] = ShaderValueType::type_matrix4;
                }
              }
              // TODO: Support unsigned integer binding?
            }
          }
        }
      }
    }
  }
}

void KRPipeline::initDescriptorSetStage(ShaderStage stage, const SpvReflectShaderModule* reflection)
{
  std::vector<DescriptorSetInfo>& descriptorSets = m_stages[static_cast<int>(stage)].descriptorSets;
  descriptorSets.reserve(reflection->descriptor_set_count);
  for (int i = 0; i < reflection->descriptor_set_count; i++) {
    SpvReflectDescriptorSet descriptorSet = reflection->descriptor_sets[i];
    DescriptorSetInfo& descriptorSetInfo = descriptorSets.emplace_back();
    descriptorSetInfo.bindings.reserve(descriptorSet.binding_count);
    for (int j = 0; j < descriptorSet.binding_count; j++) {
      SpvReflectDescriptorBinding& binding = *descriptorSet.bindings[j];
      DescriptorBinding& descriptorQuery = descriptorSetInfo.bindings.emplace_back();
      
      switch (binding.descriptor_type) {
      case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        {
          ImageDescriptorInfo& imageInfo = descriptorQuery.emplace<ImageDescriptorInfo>();
          imageInfo.name = binding.name;
          imageInfo.texture = nullptr;
          imageInfo.sampler = nullptr;
        }
        break;
      case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        {
          UniformBufferDescriptorInfo& bufferInfo = descriptorQuery.emplace<UniformBufferDescriptorInfo>();
          bufferInfo.name = binding.name;
          bufferInfo.buffer = nullptr;
        }
        break;
      default:
        // Not supported
        // TODO - Error handling
        break;
      }
    }
  }
}

bool KRPipeline::hasPushConstant(ShaderValue location) const
{
  for (const StageInfo& stageInfo : m_stages) {
    const PushConstantInfo& pushConstants = stageInfo.pushConstants;
    if (pushConstants.size[static_cast<size_t>(location)]) {
      return true;
    }
  }
  return false;
}


void KRPipeline::setPushConstants(const std::vector<const KRReflectedObject*> objects)
{
  for (StageInfo& stageInfo : m_stages) {
    PushConstantInfo& pushConstants = stageInfo.pushConstants;
    for (int i = 0; i < kPushConstantCount; i++) {
      size_t size = pushConstants.size[static_cast<size_t>(i)];
      if (size != 0) {
        void* constant = (pushConstants.buffer + pushConstants.offset[static_cast<size_t>(i)]);
        for (const KRReflectedObject* object : objects) {
          if (object->getShaderValue(static_cast<ShaderValue>(i), pushConstants.type[static_cast<size_t>(i)], constant)) {
            break;
          }
        }
      }
    }
  }
}

void KRPipeline::setPushConstant(ShaderValue location, float value)
{
  for (StageInfo& stageInfo : m_stages) {
    PushConstantInfo& pushConstants = stageInfo.pushConstants;
    if (pushConstants.size[static_cast<size_t>(location)] == sizeof(value)) {
      float* constant = (float*)(pushConstants.buffer + pushConstants.offset[static_cast<size_t>(location)]);
      *constant = value;
    }
  }
}


void KRPipeline::setPushConstant(ShaderValue location, int value)
{
  for (StageInfo& stageInfo : m_stages) {
    PushConstantInfo& pushConstants = stageInfo.pushConstants;
    if (pushConstants.size[static_cast<size_t>(location)] == sizeof(value)) {
      int* constant = (int*)(pushConstants.buffer + pushConstants.offset[static_cast<size_t>(location)]);
      *constant = value;
    }
  }
}

void KRPipeline::setPushConstant(ShaderValue location, const Vector2& value)
{
  for (StageInfo& stageInfo : m_stages) {
    PushConstantInfo& pushConstants = stageInfo.pushConstants;
    if (pushConstants.size[static_cast<size_t>(location)] == sizeof(value)) {
      Vector2* constant = (Vector2*)(pushConstants.buffer + pushConstants.offset[static_cast<size_t>(location)]);
      *constant = value;
    }
  }
}
void KRPipeline::setPushConstant(ShaderValue location, const Vector3& value)
{
  for (StageInfo& stageInfo : m_stages) {
    PushConstantInfo& pushConstants = stageInfo.pushConstants;
    if (pushConstants.size[static_cast<size_t>(location)] == sizeof(value)) {
      Vector3* constant = (Vector3*)(pushConstants.buffer + pushConstants.offset[static_cast<size_t>(location)]);
      *constant = value;
    }
  }
}

void KRPipeline::setPushConstant(ShaderValue location, const Vector4& value)
{
  for (StageInfo& stageInfo : m_stages) {
    PushConstantInfo& pushConstants = stageInfo.pushConstants;
    if (pushConstants.size[static_cast<size_t>(location)] == sizeof(value)) {
      Vector4* constant = (Vector4*)(pushConstants.buffer + pushConstants.offset[static_cast<size_t>(location)]);
      *constant = value;
    }
  }
}

void KRPipeline::setPushConstant(ShaderValue location, const Matrix4& value)
{
  for (StageInfo& stageInfo : m_stages) {
    PushConstantInfo& pushConstants = stageInfo.pushConstants;
    if (pushConstants.size[static_cast<size_t>(location)] == sizeof(value)) {
      Matrix4* constant = (Matrix4*)(pushConstants.buffer + pushConstants.offset[static_cast<size_t>(location)]);
      *constant = value;
    }
  }
}

void KRPipeline::setPushConstant(ShaderValue location, const Matrix4* value, const size_t count)
{
  for (StageInfo& stageInfo : m_stages) {
    PushConstantInfo& pushConstants = stageInfo.pushConstants;
    // TODO - Vulkan refactoring
    // GLDEBUG(glUniformMatrix4fv(pushConstants.offset[KRPipeline::PushConstant::bone_transforms], (int)bones.size(), GL_FALSE, bone_mats));
  }
}

void KRPipeline::updateDescriptorBinding()
{
  // TODO - Implement
  // Vulkan Refactoring
}

void KRPipeline::updatePushConstants(KRNode::RenderInfo& ri, const Matrix4& matModel)
{
  KRModelView modelView(ri.viewport, matModel);

  std::vector<const KRReflectedObject*> objects = { &modelView, ri.viewport };
  setPushConstants(objects);

  setPushConstant(ShaderValue::absolute_time, getContext().getAbsoluteTime());

  int light_directional_count = 0;
  //int light_point_count = 0;
  //int light_spot_count = 0;
  // TODO - Need to support multiple lights and more light types in forward rendering
  if (ri.renderPass->getType() != RenderPassType::RENDER_PASS_DEFERRED_LIGHTS && ri.renderPass->getType() != RenderPassType::RENDER_PASS_DEFERRED_GBUFFER && ri.renderPass->getType() != RenderPassType::RENDER_PASS_DEFERRED_OPAQUE && ri.renderPass->getType() != RenderPassType::RENDER_PASS_SHADOWMAP) {

    for (std::vector<KRDirectionalLight*>::const_iterator light_itr = ri.directional_lights.begin(); light_itr != ri.directional_lights.end(); light_itr++) {
      KRDirectionalLight* directional_light = (*light_itr);
      if (light_directional_count == 0) {
        int cShadowBuffers = directional_light->getShadowBufferCount();
        if (hasPushConstant(ShaderValue::shadowtexture1) && cShadowBuffers > 0) {
          // TODO - Vulkan Refactoring.  Note: Sampler needs clamp-to-edge and linear filtering
          if (m_pContext->getTextureManager()->selectTexture(0 /*GL_TEXTURE_2D*/, 3, directional_light->getShadowTextures()[0])) {
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
          }
        }

        if (hasPushConstant(ShaderValue::shadowtexture2) && cShadowBuffers > 1 && ri.camera->settings.m_cShadowBuffers > 1) {
          // TODO - Vulkan Refactoring.  Note: Sampler needs clamp-to-edge and linear filtering
          if (m_pContext->getTextureManager()->selectTexture(0 /*GL_TEXTURE_2D*/, 4, directional_light->getShadowTextures()[1])) {
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
          }
        }

        if (hasPushConstant(ShaderValue::shadowtexture3) && cShadowBuffers > 2 && ri.camera->settings.m_cShadowBuffers > 2) {
          // TODO - Vulkan Refactoring.  Note: Sampler needs clamp-to-edge and linear filtering
          if (m_pContext->getTextureManager()->selectTexture(0 /*GL_TEXTURE_2D*/, 5, directional_light->getShadowTextures()[2])) {
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
          }
        }

        Matrix4 matBias;
        matBias.translate(1.0, 1.0, 1.0);
        matBias.scale(0.5);
        for (int iShadow = 0; iShadow < cShadowBuffers; iShadow++) {
          setPushConstant(static_cast<ShaderValue>(static_cast<int>(ShaderValue::shadow_mvp1) + iShadow), matModel * directional_light->getShadowViewports()[iShadow].getViewProjectionMatrix() * matBias);
        }

        if (hasPushConstant(ShaderValue::light_direction_model_space)) {
          Matrix4 inverseModelMatrix = matModel;
          inverseModelMatrix.invert();

          // Bind the light direction vector
          Vector3 lightDirObject = Matrix4::Dot(inverseModelMatrix, directional_light->getWorldLightDirection());
          lightDirObject.normalize();
          setPushConstant(ShaderValue::light_direction_model_space, lightDirObject);
        }
      }

      light_directional_count++;
    }

    //light_point_count = point_lights.size();
    //light_spot_count = spot_lights.size();
  }

  // Fog parameters
  setPushConstant(ShaderValue::fog_near, ri.camera->settings.fog_near);
  setPushConstant(ShaderValue::fog_far, ri.camera->settings.fog_far);
  setPushConstant(ShaderValue::fog_density, ri.camera->settings.fog_density);
  setPushConstant(ShaderValue::fog_color, ri.camera->settings.fog_color);

  if (hasPushConstant(ShaderValue::fog_scale)) {
    setPushConstant(ShaderValue::fog_scale, 1.0f / (ri.camera->settings.fog_far - ri.camera->settings.fog_near));
  }
  if (hasPushConstant(ShaderValue::density_premultiplied_exponential)) {
    setPushConstant(ShaderValue::density_premultiplied_exponential, -ri.camera->settings.fog_density * 1.442695f); // -fog_density / log(2)
  }
  if (hasPushConstant(ShaderValue::density_premultiplied_squared)) {
    setPushConstant(ShaderValue::density_premultiplied_squared, (float)(-ri.camera->settings.fog_density * ri.camera->settings.fog_density * 1.442695)); // -fog_density * fog_density / log(2)
  }

  // Sets the diffuseTexture variable to the first texture unit
  setPushConstant(ShaderValue::diffusetexture, 0);

  // Sets the specularTexture variable to the second texture unit
  setPushConstant(ShaderValue::speculartexture, 1);

  // Sets the normalTexture variable to the third texture unit
  setPushConstant(ShaderValue::normaltexture, 2);

  // Sets the shadowTexture variable to the fourth texture unit
  setPushConstant(ShaderValue::shadowtexture1, 3);
  setPushConstant(ShaderValue::shadowtexture2, 4);
  setPushConstant(ShaderValue::shadowtexture3, 5);
  setPushConstant(ShaderValue::reflectioncubetexture, 4);
  setPushConstant(ShaderValue::lightmaptexture, 5);
  setPushConstant(ShaderValue::gbuffer_frame, 6);
  setPushConstant(ShaderValue::gbuffer_depth, 7); // Texture unit 7 is used for reading the depth buffer in gBuffer pass #2 and in post-processing pass
  setPushConstant(ShaderValue::reflectiontexture, 7); // Texture unit 7 is used for the reflection map textures in gBuffer pass #3 and when using forward rendering
  setPushConstant(ShaderValue::depth_frame, 0);
  setPushConstant(ShaderValue::render_frame, 1);
  setPushConstant(ShaderValue::volumetric_environment_frame, 2);

  for (StageInfo& stageInfo : m_stages) {
    PushConstantInfo& pushConstants = stageInfo.pushConstants;
    if (pushConstants.buffer) {
      vkCmdPushConstants(ri.commandBuffer, pushConstants.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, pushConstants.bufferSize, pushConstants.buffer);
    }
  }
}

bool KRPipeline::bind(KRNode::RenderInfo& ri, const Matrix4& matModel)
{
  updateDescriptorBinding();
  updateDescriptorSets();
  bindDescriptorSets(ri.commandBuffer);
  updatePushConstants(ri, matModel);

  if (ri.pipeline != this) {
    vkCmdBindPipeline(ri.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
    ri.pipeline = this;
  }

  return true;
}

void KRPipeline::updateDescriptorSets()
{
  if (m_descriptorSetLayout == VK_NULL_HANDLE) {
    // There are no descriptors
    return;
  }

  std::unique_ptr<KRDevice>& device = getContext().getDeviceManager()->getDevice(m_deviceHandle);

  // If the descriptor sets are not yet allocted, create them
  if (m_descriptorSets.size() == 0) {
    int descriptorSetCount = 0;
    for (int stage = 0; stage < static_cast<size_t>(ShaderStage::ShaderStageCount); stage++) {
      const StageInfo& stageInfo = m_stages[stage];
      descriptorSetCount += stageInfo.descriptorSets.size();
    }
    m_descriptorSets.resize(KRENGINE_MAX_FRAMES_IN_FLIGHT * descriptorSetCount, VK_NULL_HANDLE);
    std::vector<VkDescriptorSetLayout> layouts(KRENGINE_MAX_FRAMES_IN_FLIGHT, m_descriptorSetLayout);
    device->createDescriptorSets(layouts, m_descriptorSets);
  }

  // Update the descriptor sets
  // TODO - We should only do this when the descriptors have changed

  int descriptorSetCount = m_descriptorSets.size() / KRENGINE_MAX_FRAMES_IN_FLIGHT;
  int descriptorSetStart = (getContext().getCurrentFrame() % KRENGINE_MAX_FRAMES_IN_FLIGHT) * descriptorSetCount;
  int descriptorSetIndex = descriptorSetStart;

  std::vector<VkWriteDescriptorSet> descriptorWrites;
  std::vector<VkDescriptorBufferInfo> buffers;
  std::vector<VkDescriptorImageInfo> images;

  for (int stage = 0; stage < static_cast<size_t>(ShaderStage::ShaderStageCount); stage++) {
    StageInfo& stageInfo = m_stages[stage];
    for (DescriptorSetInfo& descriptorSetInfo : stageInfo.descriptorSets) {
      VkDescriptorSet descriptorSet = m_descriptorSets[descriptorSetIndex++];

      int bindingIndex = 0;
      for (DescriptorBinding& binding : descriptorSetInfo.bindings) {
        UniformBufferDescriptorInfo* buffer = std::get_if<UniformBufferDescriptorInfo>(&binding);
        ImageDescriptorInfo* image = std::get_if<ImageDescriptorInfo>(&binding);
        if (buffer) {
          VkDescriptorBufferInfo& bufferInfo = buffers.emplace_back(VkDescriptorBufferInfo{});
          bufferInfo.buffer = buffer->buffer->getBuffer();
          bufferInfo.offset = 0;
          bufferInfo.range = VK_WHOLE_SIZE;

          VkWriteDescriptorSet& descriptorWrite = descriptorWrites.emplace_back(VkWriteDescriptorSet{});
          descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
          descriptorWrite.dstSet = descriptorSet;
          descriptorWrite.dstBinding = bindingIndex;
          descriptorWrite.dstArrayElement = 0;
          descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
          descriptorWrite.descriptorCount = 1;
          descriptorWrite.pBufferInfo = &bufferInfo;
        } else if (image) {
          VkDescriptorImageInfo& imageInfo = images.emplace_back(VkDescriptorImageInfo{});
          imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
          imageInfo.imageView = image->texture->getFullImageView(m_deviceHandle);
          imageInfo.sampler = image->sampler->getSampler(m_deviceHandle);

          VkWriteDescriptorSet& descriptorWrite = descriptorWrites.emplace_back(VkWriteDescriptorSet{});
          descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
          descriptorWrite.dstSet = descriptorSet;
          descriptorWrite.dstBinding = bindingIndex++;
          descriptorWrite.dstArrayElement = 0;
          descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
          descriptorWrite.descriptorCount = 1;
          descriptorWrite.pImageInfo = &imageInfo;
        } else {
          // TODO - Error Handling
          assert(false);
        }
        bindingIndex++;
      }
    }
  }

  if (!descriptorWrites.empty()) {
    vkUpdateDescriptorSets(device->m_logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
  }
}

void KRPipeline::bindDescriptorSets(VkCommandBuffer& commandBuffer)
{
  if (m_descriptorSets.empty()) {
    return;
  }
  int descriptorSetCount = m_descriptorSets.size() / KRENGINE_MAX_FRAMES_IN_FLIGHT;
  int startDescriptorSet = (getContext().getCurrentFrame() % KRENGINE_MAX_FRAMES_IN_FLIGHT) * descriptorSetCount;
  VkDescriptorSet descriptorSet = m_descriptorSets[startDescriptorSet];
  if (descriptorSet == VK_NULL_HANDLE) {
    return;
  }
  // TODO - Vulkan Refactoring - Support multiple descriptor set binding
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, descriptorSetCount, &descriptorSet, 0, nullptr);
}

const char* KRPipeline::getKey() const
{
  return m_szKey;
}

VkPipeline& KRPipeline::getPipeline()
{
  return m_graphicsPipeline;
}

void KRPipeline::setImageBinding(const std::string& name, KRTexture* texture, KRSampler* sampler)
{
  for (int stage = 0; stage < static_cast<size_t>(ShaderStage::ShaderStageCount); stage++) {
    StageInfo& stageInfo = m_stages[stage];
    for (DescriptorSetInfo& descriptorSetInfo : stageInfo.descriptorSets) {
      for (DescriptorBinding& binding : descriptorSetInfo.bindings) {
        ImageDescriptorInfo* image = std::get_if<ImageDescriptorInfo>(&binding);
        if (image) {
          if (image->name == name) {
            image->texture = texture;
            image->sampler = sampler;
          }
        }
      }
    }
  }
}
