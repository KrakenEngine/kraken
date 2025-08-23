//
//  KRPipeline.h
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


#pragma once

#include "KREngine-common.h"
#include "nodes/KRCamera.h"
#include "nodes/KRNode.h"
#include "KRViewport.h"
#include "resources/mesh/KRMesh.h"
#include "resources/shader/KRShader.h"

class KRSampler;
class KRShader;
class KRRenderPass;
class KRUniformBuffer;
class KRTexture;

enum class ModelFormat : __uint8_t;
struct SpvReflectShaderModule;

enum class CullMode : uint32_t
{
  kCullBack = 0,
  kCullFront,
  kCullNone
};

// Note: RasterMode is likely to be refactored later to a bitfield
enum class RasterMode : uint32_t
{
  kOpaque = 0,
  /*
      kOpaque is equivalent to:

      // Disable blending
      glDisable(GL_BLEND));

      // Enable z-buffer write
      glDepthMask(GL_TRUE);

      // Enable z-buffer test
      glEnable(GL_DEPTH_TEST))
      glDepthFunc(GL_LEQUAL);
      glDepthRangef(0.0, 1.0);
  */
  kOpaqueNoDepthWrite,
  /*
      kOpaque is equivalent to:

      // Disable blending
      glDisable(GL_BLEND));

      // Disable z-buffer write
      glDepthMask(GL_FALSE);

      // Enable z-buffer test
      glEnable(GL_DEPTH_TEST))
      glDepthFunc(GL_LEQUAL);
      glDepthRangef(0.0, 1.0);
  */
  kOpaqueLessTest,
  /*
      kOpaqueLessTest is equivalent to:

      // Disable blending
      glDisable(GL_BLEND));

      // Enable z-buffer write
      glDepthMask(GL_TRUE);

      // Enable z-buffer test
      glEnable(GL_DEPTH_TEST))
      glDepthFunc(GL_LESS);
      glDepthRangef(0.0, 1.0);
  */
  kOpaqueNoTest,
  /*
      kOpaqueNoTest is equivalent to:

      // Disable blending
      glDisable(GL_BLEND));

      // Enable z-buffer write
      glDepthMask(GL_TRUE);

      // Disable z-buffer test
      glDisable(GL_DEPTH_TEST)
  */
  kAlphaBlend,
  /*
      kAlphaBlend is equivalent to:

      // Enable alpha blending
      glEnable(GL_BLEND));
      glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));

      // Disable z-buffer write
      glDepthMask(GL_FALSE);

      // Enable z-buffer test
      glEnable(GL_DEPTH_TEST))
      glDepthFunc(GL_LEQUAL);
      glDepthRangef(0.0, 1.0);
  */
  kAlphaBlendNoTest,
  /*
      kAlphaBlendNoTest is equivalent to:

      // Enable alpha blending
      glEnable(GL_BLEND));
      glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));

      // Disable z-buffer write
      glDepthMask(GL_FALSE);

      // Disable z-buffer test
      glDisable(GL_DEPTH_TEST)
  */
  kAdditive,
  /*
      kAdditive is equivalent to:

      // Enable additive blending
      glEnable(GL_BLEND));
      glBlendFunc(GL_ONE, GL_ONE));

      // Disable z-buffer write
      glDepthMask(GL_FALSE);

      // Enable z-buffer test
      glEnable(GL_DEPTH_TEST))
      glDepthFunc(GL_LEQUAL);
      glDepthRangef(0.0, 1.0);
  */
  kAdditiveNoTest,
  /*
  kAdditive is equivalent to:

  // Enable additive blending
  glEnable(GL_BLEND));
  glBlendFunc(GL_ONE, GL_ONE));

  // Disable z-buffer write
  glDepthMask(GL_FALSE);

  // Disable z-buffer test
  glDisable(GL_DEPTH_TEST)
  */
};

class PipelineInfo
{
public:
  const std::string* shader_name;
  KRCamera* pCamera;
  const std::vector<KRPointLight*>* point_lights;
  const std::vector<KRDirectionalLight*>* directional_lights;
  const std::vector<KRSpotLight*>* spot_lights;
  int bone_count;
  bool bDiffuseMap : 1;
  bool bNormalMap : 1;
  bool bSpecMap : 1;
  bool bReflectionMap : 1;
  bool bReflectionCubeMap : 1;
  bool bLightMap : 1;
  bool bDiffuseMapScale : 1;
  bool bSpecMapScale : 1;
  bool bNormalMapScale : 1;
  bool bReflectionMapScale : 1;
  bool bDiffuseMapOffset : 1;
  bool bSpecMapOffset : 1;
  bool bNormalMapOffset : 1;
  bool bReflectionMapOffset : 1;
  bool bAlphaTest : 1;
  bool bRimColor : 1;
  RasterMode rasterMode;
  CullMode cullMode;
  uint32_t vertexAttributes;
  ModelFormat modelFormat;
  const KRRenderPass* renderPass;
};

class KRPipeline : public KRContextObject
{
public:

  KRPipeline(KRContext& context, KrDeviceHandle deviceHandle, const KRRenderPass* renderPass, hydra::Vector2i viewport_size, hydra::Vector2i scissor_size, const PipelineInfo& info, const char* szKey, const std::vector<KRShader*>& shaders, uint32_t vertexAttributes, ModelFormat modelFormat);
  virtual ~KRPipeline();
  const char* getKey() const;

  bool bind(KRNode::RenderInfo& ri, const hydra::Matrix4& matModel);

  static const size_t kPushConstantCount = static_cast<size_t>(ShaderValue::NUM_PUSH_CONSTANTS);

  bool hasPushConstant(ShaderValue location) const;
  void setPushConstant(ShaderValue location, float value);
  void setPushConstant(ShaderValue location, int value);
  void setPushConstant(ShaderValue location, const hydra::Vector2& value);
  void setPushConstant(ShaderValue location, const hydra::Vector3& value);
  void setPushConstant(ShaderValue location, const hydra::Vector4& value);
  void setPushConstant(ShaderValue location, const hydra::Matrix4& value);
  void setPushConstant(ShaderValue location, const hydra::Matrix4* value, const size_t count);

  void setImageBinding(const std::string& name, KRTexture* texture, KRSampler* sampler);

  VkPipeline& getPipeline();
  void bindDescriptorSets(VkCommandBuffer& commandBuffer);

private:
  void updateDescriptorBinding();
  void updateDescriptorSets();
  void updatePushConstants(KRNode::RenderInfo& ri, const hydra::Matrix4& matModel);

  struct PushConstantInfo
  {
    int offset[kPushConstantCount];
    __uint8_t size[kPushConstantCount];
    uint8_t* buffer;
    int bufferSize;
    VkPipelineLayout layout;
  };

  struct ImageDescriptorInfo
  {
    KRTexture* texture;
    KRSampler* sampler;
    std::string name;
  };

  struct UniformBufferDescriptorInfo
  {
    KRUniformBuffer* buffer;
    std::string name;
  };

  typedef std::variant<ImageDescriptorInfo, UniformBufferDescriptorInfo> DescriptorBinding;
  typedef std::vector<DescriptorBinding> DescriptorSetBinding;

  struct DescriptorSetInfo
  {
    DescriptorSetBinding bindings;
  };

  struct StageInfo
  {
    StageInfo()
      : pushConstants{}
    {}
    PushConstantInfo pushConstants;
    std::vector<DescriptorSetInfo> descriptorSets;
  };

  StageInfo m_stages[static_cast<size_t>(ShaderStage::ShaderStageCount)];

  char m_szKey[256];

  VkDescriptorSetLayout m_descriptorSetLayout;
  VkPipelineLayout m_pipelineLayout;
  VkPipeline m_graphicsPipeline;
  std::vector<VkDescriptorSet> m_descriptorSets;
  KrDeviceHandle m_deviceHandle;

  void initPushConstantStage(ShaderStage stage, const SpvReflectShaderModule* reflection);
  void initDescriptorSetStage(ShaderStage stage, const SpvReflectShaderModule* reflection);
};
