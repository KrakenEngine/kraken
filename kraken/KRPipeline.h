//
//  KRPipeline.h
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


#pragma once

#include "KREngine-common.h"
#include "KRCamera.h"
#include "KRNode.h"
#include "KRViewport.h"
#include "KRMesh.h"

class KRShader;
class KRSurface;
class KRRenderPass;

enum class ModelFormat : __uint8_t;

enum class CullMode : uint32_t {
  kCullBack = 0,
  kCullFront,
  kCullNone
};

// Note: RasterMode is likely to be refactored later to a bitfield
enum class RasterMode : uint32_t {
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

class PipelineInfo {
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
  KRNode::RenderPass renderPass;
};

class KRPipeline : public KRContextObject {
public:

    KRPipeline(KRContext& context, KRSurface& surface, const PipelineInfo& info, const char* szKey, const std::vector<KRShader*>& shaders, uint32_t vertexAttributes, ModelFormat modelFormat);
    virtual ~KRPipeline();
    const char *getKey() const;
    
    bool bind(VkCommandBuffer& commandBuffer, KRCamera &camera, const KRViewport &viewport, const Matrix4 &matModel, const std::vector<KRPointLight *> *point_lights, const std::vector<KRDirectionalLight *> *directional_lights, const std::vector<KRSpotLight *>*spot_lights, const KRNode::RenderPass &renderPass);

    enum class Uniform : uint8_t {
        material_ambient = 0,
        material_diffuse,
        material_specular,
        material_reflection,
        material_alpha,
        material_shininess,
        light_position,
        light_direction_model_space,
        light_direction_view_space,
        light_color,
        light_decay_start,
        light_cutoff,
        light_intensity,
        flare_size,
        view_space_model_origin,
        mvp,
        invp,
        invmvp,
        invmvp_no_translate,
        model_view_inverse_transpose,
        model_inverse_transpose,
        model_view,
        model_matrix,
        projection_matrix,
        camerapos_model_space,
        viewport,
        viewport_downsample,
        diffusetexture,
        speculartexture,
        reflectioncubetexture,
        reflectiontexture,
        normaltexture,
        diffusetexture_scale,
        speculartexture_scale,
        reflectiontexture_scale,
        normaltexture_scale,
        ambienttexture_scale,
        diffusetexture_offset,
        speculartexture_offset,
        reflectiontexture_offset,
        normaltexture_offset,
        ambienttexture_offset,
        shadow_mvp1,
        shadow_mvp2,
        shadow_mvp3,
        shadowtexture1,
        shadowtexture2,
        shadowtexture3,
        lightmaptexture,
        gbuffer_frame,
        gbuffer_depth,
        depth_frame,
        volumetric_environment_frame,
        render_frame,
        absolute_time,
        fog_near,
        fog_far,
        fog_density,
        fog_color,
        fog_scale,
        density_premultiplied_exponential,
        density_premultiplied_squared,
        slice_depth_scale,
        particle_origin,
        bone_transforms,
        rim_color,
        rim_power,
        fade_color,
        NUM_UNIFORMS
    };

    static const size_t kUniformCount = static_cast<size_t>(Uniform::NUM_UNIFORMS);

    enum class ShaderStages : uint8_t
    {
      vertex = 0,
      fragment,
      geometry,
      compute,
      shaderStageCount
    };

    static const size_t kShaderStageCount = static_cast<size_t>(ShaderStages::shaderStageCount);
    
    bool hasUniform(Uniform location) const;
    void setUniform(Uniform location, float value);
    void setUniform(Uniform location, int value);
    void setUniform(Uniform location, const Vector2 &value);
    void setUniform(Uniform location, const Vector3 &value);
    void setUniform(Uniform location, const Vector4 &value);
    void setUniform(Uniform location, const Matrix4 &value);
    void setUniform(Uniform location, const Matrix4* value, const size_t count);

    VkPipeline& getPipeline();
    
private:
    static const char* KRENGINE_UNIFORM_NAMES[];

    struct PushConstantStageInfo
    {
      int offset[kUniformCount];
      __uint8_t size[kUniformCount];
    };
    PushConstantStageInfo m_pushConstants[static_cast<size_t>(ShaderStages::shaderStageCount)];

    uint8_t* m_pushConstantBuffer;
    int m_pushConstantBufferSize;

    char m_szKey[256];

    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_graphicsPipeline;
    VkPipelineLayout m_pushConstantsLayout;
    
};
