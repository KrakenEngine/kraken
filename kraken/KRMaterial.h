//
//  KRMaterial.h
//  Kraken Engine
//
//  Copyright 2023 Kearwood Gilbert. All rights reserved.
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

#include "KRTexture.h"
#include "KRPipelineManager.h"
#include "KRPipeline.h"
#include "KRCamera.h"
#include "KRResource.h"
#include "KRScene.h"
#include "KRBone.h"

enum class CullMode : __uint32_t;
enum class ModelFormat : __uint8_t;

class KRTextureManager;
class KRContext;
class KRSurface;

class KRMaterial : public KRResource
{
public:
  typedef enum
  {
    KRMATERIAL_ALPHA_MODE_OPAQUE, // Non-transparent materials
    KRMATERIAL_ALPHA_MODE_TEST, // Alpha in diffuse texture is interpreted as punch-through when < 0.5
    KRMATERIAL_ALPHA_MODE_BLENDONESIDE, // Blended alpha with backface culling
    KRMATERIAL_ALPHA_MODE_BLENDTWOSIDE // Blended alpha rendered in two passes.  First pass renders backfaces; second pass renders frontfaces.
  } alpha_mode_type;

  KRMaterial(KRContext& context, const char* szName);
  virtual ~KRMaterial();

  virtual std::string getExtension();
  virtual bool save(mimir::Block& data);


  void setAmbientMap(std::string texture_name, Vector2 texture_scale, Vector2 texture_offset);
  void setDiffuseMap(std::string texture_name, Vector2 texture_scale, Vector2 texture_offset);
  void setSpecularMap(std::string texture_name, Vector2 texture_scale, Vector2 texture_offset);
  void setReflectionMap(std::string texture_name, Vector2 texture_scale, Vector2 texture_offset);
  void setReflectionCube(std::string texture_name);
  void setNormalMap(std::string texture_name, Vector2 texture_scale, Vector2 texture_offset);
  void setAmbient(const Vector3& c);
  void setDiffuse(const Vector3& c);
  void setSpecular(const Vector3& c);
  void setReflection(const Vector3& c);
  void setTransparency(float a);
  void setShininess(float s);
  void setAlphaMode(alpha_mode_type blend_mode);
  alpha_mode_type getAlphaMode();


  bool isTransparent();
  const std::string& getName() const;

  void bind(const KRNode::RenderInfo& ri, ModelFormat modelFormat, __uint32_t vertexAttributes, CullMode cullMode, const std::vector<KRBone*>& bones, const std::vector<Matrix4>& bind_poses, const Matrix4& matModel, KRTexture* pLightMap, const Vector3& rim_color, float rim_power, float lod_coverage = 0.0f);

  bool needsVertexTangents();

  kraken_stream_level getStreamLevel();
  void preStream(float lodCoverage);

private:
  std::string m_name;

  KRTexture* m_pAmbientMap; // mtl map_Ka value
  KRTexture* m_pDiffuseMap; // mtl map_Kd value
  KRTexture* m_pSpecularMap; // mtl map_Ks value
  KRTexture* m_pReflectionMap; // mtl refl value
  KRTexture* m_pReflectionCube;
  KRTexture* m_pNormalMap; // mtl map_Normal value
  std::string m_ambientMap;
  std::string m_diffuseMap;
  std::string m_specularMap;
  std::string m_reflectionMap;
  std::string m_reflectionCube;
  std::string m_normalMap;

  Vector2 m_ambientMapScale;
  Vector2 m_ambientMapOffset;
  Vector2 m_diffuseMapScale;
  Vector2 m_diffuseMapOffset;
  Vector2 m_specularMapScale;
  Vector2 m_specularMapOffset;
  Vector2 m_reflectionMapScale;
  Vector2 m_reflectionMapOffset;
  Vector2 m_normalMapScale;
  Vector2 m_normalMapOffset;

  Vector3 m_ambientColor; // Ambient rgb
  Vector3 m_diffuseColor; // Diffuse rgb
  Vector3 m_specularColor; // Specular rgb
  Vector3 m_reflectionColor; // Reflection rgb

  //float m_ka_r, m_ka_g, m_ka_b; // Ambient rgb
  //float m_kd_r, m_kd_g, m_kd_b; // Diffuse rgb
  //float m_ks_r, m_ks_g, m_ks_b; // Specular rgb
  //float m_kr_r, m_kr_g, m_kr_b; // Reflection rgb

  float m_tr; // Transparency
  float m_ns; // Shininess

  alpha_mode_type m_alpha_mode;

  void getTextures();
};
