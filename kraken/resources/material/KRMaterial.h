//
//  KRMaterial.h
//  Kraken Engine
//
//  Copyright 2026 Kearwood Gilbert. All rights reserved.
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

#include "resources/texture/KRTexture.h"
#include "resources/texture/KRTextureBinding.h"
#include "KRPipelineManager.h"
#include "KRPipeline.h"
#include "nodes/KRCamera.h"
#include "resources/KRResource.h"
#include "resources/scene/KRScene.h"
#include "nodes/KRBone.h"
#include "KRShaderReflection.h"

enum class CullMode : __uint32_t;
enum class ModelFormat : __uint8_t;

class KRTextureManager;
class KRContext;
class KRSurface;

class KRMaterial
  : public KRResource
  , public KRReflectedObject
{
public:
  typedef enum
  {
    KRMATERIAL_ALPHA_MODE_OPAQUE, // Non-transparent materials
    KRMATERIAL_ALPHA_MODE_TEST, // Alpha in diffuse texture is interpreted as punch-through when < 0.5
    KRMATERIAL_ALPHA_MODE_BLEND // Blended alpha with backface culling
  } alpha_mode_type;

  typedef enum
  {
    KRMATERIAL_SHADING_MODEL_UNLIT,
    KRMATERIAL_SHADING_MODEL_PBR
  } shading_model_type;
  
  typedef enum
  {
    KRMATERIAL_TEXTURE_CLAMP,
    KRMATERIAL_TEXTURE_REPEAT,
    KRMATERIAL_TEXTURE_MIRROR_REPEAT
  } texture_wrap_type;
  
  typedef enum
  {
    KRMATERIAL_TEXTURE_MAG_NEAREST,
    KRMATERIAL_TEXTURE_MAG_LINEAR
  } texture_mag_filter_type;
  
  typedef enum
  {
    KRMATERIAL_TEXTURE_MIN_NEAREST,
    KRMATERIAL_TEXTURE_MIN_LINEAR,
    KRMATERIAL_TEXTURE_MIN_NEAREST_MIPMAP_NEAREST,
    KRMATERIAL_TEXTURE_MIN_LINEAR_MIPMAP_NEAREST,
    KRMATERIAL_TEXTURE_MIN_NEAREST_MIPMAP_LINEAR,
    KRMATERIAL_TEXTURE_MIN_LINEAR_MIPMAP_LINEAR
  } texture_min_filter_type;

  struct TextureMap
  {
    KRTextureBinding texture;
    int texCoord{ 0 };
    hydra::Vector2 scale{ 1.f, 1.f };
    hydra::Vector2 offset{ 0.f, 0.f };
    float rotation{ 0.f };
    
    texture_wrap_type wrapS = KRMATERIAL_TEXTURE_REPEAT;
    texture_wrap_type wrapT = KRMATERIAL_TEXTURE_REPEAT;
    texture_mag_filter_type magFilter = KRMATERIAL_TEXTURE_MAG_LINEAR;
    texture_min_filter_type minFilter = KRMATERIAL_TEXTURE_MIN_LINEAR_MIPMAP_LINEAR;

    TextureMap(KRTexture::texture_usage_t usage)
      : texture{ usage }
    {
    }
    
    simdjson::error_code parse(simdjson::ondemand::value &val);
  };

  KRMaterial(KRContext& context, const char* szName);
  KRMaterial(KRContext& context, std::string name, mimir::Block* data);
  virtual ~KRMaterial();

  virtual std::string getExtension() override;
  virtual bool save(mimir::Block& data) override;

  void setTransparency(float a);
  void setShininess(float s);
  void setAlphaMode(alpha_mode_type blend_mode);
  alpha_mode_type getAlphaMode();



  bool isTransparent();
  
  bool bind(KRNode::RenderInfo& ri, ModelFormat modelFormat, __uint32_t vertexAttributes, CullMode cullMode, const std::vector<KRBone*>& bones, const std::vector<hydra::Matrix4>& bind_poses, const hydra::Matrix4& matModel, KRTexture* pLightMap, float lod_coverage = 0.0f);

  bool needsVertexTangents();

  kraken_stream_level getStreamLevel();

  virtual void getResourceBindings(std::list<KRResourceBinding*>& bindings) override;

  // --- Serialized Material Attributes ---
  TextureMap m_baseColorMap{ KRTexture::TEXTURE_USAGE_MATERIAL_BASE_COLOR };
  hydra::Vector4 m_baseColorFactor{ 1.f, 1.f, 1.f, 1.f };
  TextureMap m_normalMap{ KRTexture::TEXTURE_USAGE_MATERIAL_NORMAL };
  float m_normalScale{ 1.f };
  TextureMap m_emissiveMap{ KRTexture::TEXTURE_USAGE_MATERIAL_EMISSIVE };
  hydra::Vector3 m_emissiveFactor{ 0.f, 0.f, 0.f };
  TextureMap m_occlusionMap{ KRTexture::TEXTURE_USAGE_MATERIAL_OCCLUSION };
  float m_occlusionStrength{ 1.f };
  TextureMap m_metalicRoughnessMap{ KRTexture::TEXTURE_USAGE_MATERIAL_METALIC_ROUGHNESS };
  float m_metalicFactor{ 1.f };
  float m_roughnessFactor{ 1.f };
  alpha_mode_type m_alphaMode{ KRMATERIAL_ALPHA_MODE_OPAQUE };
  float m_alphaCutoff{ 0.5f };
  bool m_doubleSided{ false };
  float m_ior{ 1.5f };
  shading_model_type m_shadingModel = { KRMATERIAL_SHADING_MODEL_PBR };

  TextureMap m_anisotropyMap{ KRTexture::TEXTURE_USAGE_MATERIAL_ANISOTROPY };
  float m_anisotropyStrength{ 0.f };
  float m_anisotropyRotation{ 0.f };

  TextureMap m_clearcoatMap{ KRTexture::TEXTURE_USAGE_MATERIAL_CLEARCOAT };
  float m_clearcoatFactor{ 0.f };
  TextureMap m_clearcoatRoughnessMap{ KRTexture::TEXTURE_USAGE_MATERIAL_CLEARCOAT_ROUGHNESS };
  float m_clearcoatRoughnessFactor{ 0.f };
  TextureMap m_clearcoatNormalMap{ KRTexture::TEXTURE_USAGE_MATERIAL_CLEARCOAT_NORMAL };
  float m_clearcoatNormalScale{ 1.f };

  float m_dispersion{ 0.f };

  TextureMap m_specularMap{ KRTexture::TEXTURE_USAGE_MATERIAL_SPECULAR };
  float m_specularFactor{ 1.f };
  TextureMap m_specularColorMap{ KRTexture::TEXTURE_USAGE_MATERIAL_SPECULAR_COLOR };
  hydra::Vector3 m_specularColorFactor{ 1.f, 1.f, 1.f };

  TextureMap m_thicknessMap{ KRTexture::TEXTURE_USAGE_MATERIAL_THICKNESS };
  float m_thicknessFactor{ 0.f };
  float m_attenuationDistance{ std::numeric_limits<float>::max() };
  hydra::Vector3 m_attenuationColor{ 1.f, 1.f, 1.f };

  TextureMap m_transmissionMap{ KRTexture::TEXTURE_USAGE_MATERIAL_TRANSMISSION };
  float m_transmissionFactor = 0.f;

private:
  bool getShaderValue(const KRCamera* camera, ShaderValue value, float* output) const final;
  bool getShaderValue(const KRCamera* camera, ShaderValue value, hydra::Vector2* output) const final;
  bool getShaderValue(const KRCamera* camera, ShaderValue value, hydra::Vector3* output) const final;
  bool getShaderValue(const KRCamera* camera, ShaderValue value, hydra::Vector4* output) const final;
  bool getShaderValue(const KRCamera* camera, ShaderValue value, KRResourceBinding* output) const final;
  bool getShaderValue(const KRCamera* camera, ShaderValue value, int64_t* output) const final;
  bool getShaderValue(const KRCamera* camera, ShaderValue value, bool* output) const final;
  bool getImageBinding(const std::string& name, const KRTextureBinding** binding, KRSampler** sample) const final;
};
