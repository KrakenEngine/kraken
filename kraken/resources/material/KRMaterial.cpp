//
//  KRMaterial.cpp
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

#include "KREngine-common.h"

#include "KRMaterial.h"
#include "resources/texture/KRTextureManager.h"
#include "KRRenderPass.h"

#include "KRContext.h"

using namespace mimir;
using namespace hydra;

KRMaterial::KRMaterial(KRContext& context, const char* szName) : KRResource(context, szName)
{
  m_name = szName;
  m_ambientColor = Vector3::Zero();
  m_diffuseColor = Vector3::One();
  m_specularColor = Vector3::One();
  m_reflectionColor = Vector3::Zero();
  m_tr = 1.0f;
  m_ns = 0.0f;
  m_ambientMapOffset = Vector2::Create(0.0f, 0.0f);
  m_specularMapOffset = Vector2::Create(0.0f, 0.0f);
  m_diffuseMapOffset = Vector2::Create(0.0f, 0.0f);
  m_ambientMapScale = Vector2::Create(1.0f, 1.0f);
  m_specularMapScale = Vector2::Create(1.0f, 1.0f);
  m_diffuseMapScale = Vector2::Create(1.0f, 1.0f);
  m_reflectionMapOffset = Vector2::Create(0.0f, 0.0f);
  m_reflectionMapScale = Vector2::Create(1.0f, 1.0f);
  m_alpha_mode = KRMATERIAL_ALPHA_MODE_OPAQUE;
}

KRMaterial::~KRMaterial()
{

}

std::string KRMaterial::getExtension()
{
  return "mtl";
}

bool KRMaterial::needsVertexTangents()
{
  return m_normalMap.isSet();
}

bool KRMaterial::save(Block& data)
{
  std::stringstream stream;
  stream.precision(std::numeric_limits<long double>::digits10);
  stream.setf(std::ios::fixed, std::ios::floatfield);

  stream << "newmtl " << m_name;
  stream << "\nka " << m_ambientColor.x << " " << m_ambientColor.y << " " << m_ambientColor.z;
  stream << "\nkd " << m_diffuseColor.x << " " << m_diffuseColor.y << " " << m_diffuseColor.z;
  stream << "\nks " << m_specularColor.x << " " << m_specularColor.y << " " << m_specularColor.z;
  stream << "\nkr " << m_reflectionColor.x << " " << m_reflectionColor.y << " " << m_reflectionColor.z;
  stream << "\nTr " << m_tr;
  stream << "\nNs " << m_ns;
  if (m_ambientMap.isSet()) {
    stream << "\nmap_Ka " << m_ambientMap.getName() << ".pvr -s " << m_ambientMapScale.x << " " << m_ambientMapScale.y << " -o " << m_ambientMapOffset.x << " " << m_ambientMapOffset.y;
  } else {
    stream << "\n# map_Ka filename.pvr -s 1.0 1.0 -o 0.0 0.0";
  }
  if (m_diffuseMap.isSet()) {
    stream << "\nmap_Kd " << m_diffuseMap.getName() << ".pvr -s " << m_diffuseMapScale.x << " " << m_diffuseMapScale.y << " -o " << m_diffuseMapOffset.x << " " << m_diffuseMapOffset.y;
  } else {
    stream << "\n# map_Kd filename.pvr -s 1.0 1.0 -o 0.0 0.0";
  }
  if (m_specularMap.isSet()) {
    stream << "\nmap_Ks " << m_specularMap.getName() << ".pvr -s " << m_specularMapScale.x << " " << m_specularMapScale.y << " -o " << m_specularMapOffset.x << " " << m_specularMapOffset.y << "\n";
  } else {
    stream << "\n# map_Ks filename.pvr -s 1.0 1.0 -o 0.0 0.0";
  }
  if (m_normalMap.isSet()) {
    stream << "\nmap_Normal " << m_normalMap.getName() << ".pvr -s " << m_normalMapScale.x << " " << m_normalMapScale.y << " -o " << m_normalMapOffset.x << " " << m_normalMapOffset.y;
  } else {
    stream << "\n# map_Normal filename.pvr -s 1.0 1.0 -o 0.0 0.0";
  }
  if (m_reflectionMap.isSet()) {
    stream << "\nmap_Reflection " << m_reflectionMap.getName() << ".pvr -s " << m_reflectionMapScale.x << " " << m_reflectionMapScale.y << " -o " << m_reflectionMapOffset.x << " " << m_reflectionMapOffset.y;
  } else {
    stream << "\n# map_Reflection filename.pvr -s 1.0 1.0 -o 0.0 0.0";
  }
  if (m_reflectionCube.isSet()) {
    stream << "\nmap_ReflectionCube " << m_reflectionCube.getName() << ".pvr";
  } else {
    stream << "\n# map_ReflectionCube cubemapname";
  }
  switch (m_alpha_mode) {
  case KRMATERIAL_ALPHA_MODE_OPAQUE:
    stream << "\nalpha_mode opaque";
    break;
  case KRMATERIAL_ALPHA_MODE_TEST:
    stream << "\nalpha_mode test";
    break;
  case KRMATERIAL_ALPHA_MODE_BLENDONESIDE:
    stream << "\nalpha_mode blendoneside";
    break;
  case KRMATERIAL_ALPHA_MODE_BLENDTWOSIDE:
    stream << "\nalpha_mode blendtwoside";
    break;
  }
  stream << "\n# alpha_mode opaque, test, blendoneside, or blendtwoside";

  stream << "\n";
  data.append(stream.str());

  return true;
}

void KRMaterial::setAmbientMap(std::string texture_name, Vector2 texture_scale, Vector2 texture_offset)
{
  m_ambientMap.set(texture_name);
  m_ambientMapScale = texture_scale;
  m_ambientMapOffset = texture_offset;
}

void KRMaterial::setDiffuseMap(std::string texture_name, Vector2 texture_scale, Vector2 texture_offset)
{
  m_diffuseMap.set(texture_name);
  m_diffuseMapScale = texture_scale;
  m_diffuseMapOffset = texture_offset;
}

void KRMaterial::setSpecularMap(std::string texture_name, Vector2 texture_scale, Vector2 texture_offset)
{
  m_specularMap.set(texture_name);
  m_specularMapScale = texture_scale;
  m_specularMapOffset = texture_offset;
}

void KRMaterial::setNormalMap(std::string texture_name, Vector2 texture_scale, Vector2 texture_offset)
{
  m_normalMap.set(texture_name);
  m_normalMapScale = texture_scale;
  m_normalMapOffset = texture_offset;
}

void KRMaterial::setReflectionMap(std::string texture_name, Vector2 texture_scale, Vector2 texture_offset)
{
  m_reflectionMap.set(texture_name);
  m_reflectionMapScale = texture_scale;
  m_reflectionMapOffset = texture_offset;
}

void KRMaterial::setReflectionCube(std::string texture_name)
{
  m_reflectionCube.set(texture_name);
}

void KRMaterial::setAlphaMode(KRMaterial::alpha_mode_type alpha_mode)
{
  m_alpha_mode = alpha_mode;
}

KRMaterial::alpha_mode_type KRMaterial::getAlphaMode()
{
  return m_alpha_mode;
}

void KRMaterial::setAmbient(const Vector3& c)
{
  m_ambientColor = c;
}

void KRMaterial::setDiffuse(const Vector3& c)
{
  m_diffuseColor = c;
}

void KRMaterial::setSpecular(const Vector3& c)
{
  m_specularColor = c;
}

void KRMaterial::setReflection(const Vector3& c)
{
  m_reflectionColor = c;
}

void KRMaterial::setTransparency(float a)
{
  if (a < 1.0f && m_alpha_mode == KRMaterial::KRMATERIAL_ALPHA_MODE_OPAQUE) {
    setAlphaMode(KRMaterial::KRMATERIAL_ALPHA_MODE_BLENDONESIDE);
  }
  m_tr = a;
}

void KRMaterial::setShininess(float s)
{
  m_ns = s;
}

bool KRMaterial::isTransparent()
{
  return m_tr < 1.0 || m_alpha_mode == KRMATERIAL_ALPHA_MODE_BLENDONESIDE || m_alpha_mode == KRMATERIAL_ALPHA_MODE_BLENDTWOSIDE;
}

void KRMaterial::preStream(float lodCoverage)
{
  getTextures();

  if (m_ambientMap.isBound()) {
    m_ambientMap.get()->resetPoolExpiry(lodCoverage, KRTexture::TEXTURE_USAGE_AMBIENT_MAP);
  }

  if (m_diffuseMap.isBound()) {
    m_diffuseMap.get()->resetPoolExpiry(lodCoverage, KRTexture::TEXTURE_USAGE_DIFFUSE_MAP);
  }

  if (m_normalMap.isBound()) {
    m_normalMap.get()->resetPoolExpiry(lodCoverage, KRTexture::TEXTURE_USAGE_NORMAL_MAP);
  }

  if (m_specularMap.isBound()) {
    m_specularMap.get()->resetPoolExpiry(lodCoverage, KRTexture::TEXTURE_USAGE_SPECULAR_MAP);
  }

  if (m_reflectionMap.isBound()) {
    m_reflectionMap.get()->resetPoolExpiry(lodCoverage, KRTexture::TEXTURE_USAGE_REFLECTION_MAP);
  }

  if (m_reflectionCube.isBound()) {
    m_reflectionCube.get()->resetPoolExpiry(lodCoverage, KRTexture::TEXTURE_USAGE_REFECTION_CUBE);
  }
}


kraken_stream_level KRMaterial::getStreamLevel()
{
  kraken_stream_level stream_level = kraken_stream_level::STREAM_LEVEL_IN_HQ;

  getTextures();

  if (m_ambientMap.isBound()) {
    stream_level = KRMIN(stream_level, m_ambientMap.get()->getStreamLevel(KRTexture::TEXTURE_USAGE_AMBIENT_MAP));
  }

  if (m_diffuseMap.isBound()) {
    stream_level = KRMIN(stream_level, m_diffuseMap.get()->getStreamLevel(KRTexture::TEXTURE_USAGE_DIFFUSE_MAP));
  }

  if (m_normalMap.isBound()) {
    stream_level = KRMIN(stream_level, m_normalMap.get()->getStreamLevel(KRTexture::TEXTURE_USAGE_NORMAL_MAP));
  }

  if (m_specularMap.isBound()) {
    stream_level = KRMIN(stream_level, m_specularMap.get()->getStreamLevel(KRTexture::TEXTURE_USAGE_SPECULAR_MAP));
  }

  if (m_reflectionMap.isBound()) {
    stream_level = KRMIN(stream_level, m_reflectionMap.get()->getStreamLevel(KRTexture::TEXTURE_USAGE_REFLECTION_MAP));
  }

  if (m_reflectionCube.isBound()) {
    stream_level = KRMIN(stream_level, m_reflectionCube.get()->getStreamLevel(KRTexture::TEXTURE_USAGE_REFECTION_CUBE));
  }

  return stream_level;
}

void KRMaterial::getTextures()
{
  m_ambientMap.bind(&getContext());
  m_diffuseMap.bind(&getContext());
  m_normalMap.bind(&getContext());
  m_specularMap.bind(&getContext());
  m_reflectionMap.bind(&getContext());
  m_reflectionCube.bind(&getContext());
}

void KRMaterial::bind(KRNode::RenderInfo& ri, ModelFormat modelFormat, __uint32_t vertexAttributes, CullMode cullMode, const std::vector<KRBone*>& bones, const std::vector<Matrix4>& bind_poses, const Matrix4& matModel, KRTexture* pLightMap, float lod_coverage)
{
  bool bLightMap = pLightMap && ri.camera->settings.bEnableLightMap;

  getTextures();

  Vector2 default_scale = Vector2::One();
  Vector2 default_offset = Vector2::Zero();

  bool bHasReflection = m_reflectionColor != Vector3::Zero();
  bool bDiffuseMap = m_diffuseMap.isBound() && ri.camera->settings.bEnableDiffuseMap;
  bool bNormalMap = m_normalMap.isBound() && ri.camera->settings.bEnableNormalMap;
  bool bSpecMap = m_specularMap.isBound() && ri.camera->settings.bEnableSpecMap;
  bool bReflectionMap = m_reflectionMap.isBound() && ri.camera->settings.bEnableReflectionMap && ri.camera->settings.bEnableReflection && bHasReflection;
  bool bReflectionCubeMap = m_reflectionCube.isBound() && ri.camera->settings.bEnableReflection && bHasReflection;
  bool bAlphaTest = (m_alpha_mode == KRMATERIAL_ALPHA_MODE_TEST) && bDiffuseMap;
  bool bAlphaBlend = (m_alpha_mode == KRMATERIAL_ALPHA_MODE_BLENDONESIDE) || (m_alpha_mode == KRMATERIAL_ALPHA_MODE_BLENDTWOSIDE);

  PipelineInfo info{};
  std::string shader_name("object");
  info.shader_name = &shader_name;
  info.pCamera = ri.camera;
  info.point_lights = &ri.point_lights;
  info.directional_lights = &ri.directional_lights;
  info.spot_lights = &ri.spot_lights;
  info.bone_count = (int)bones.size();
  info.renderPass = ri.renderPass;
  info.bDiffuseMap = bDiffuseMap;
  info.bNormalMap = bNormalMap;
  info.bSpecMap = bSpecMap;
  info.bReflectionMap = bReflectionMap;
  info.bReflectionCubeMap = bReflectionCubeMap;
  info.bLightMap = bLightMap;
  info.bDiffuseMapScale = m_diffuseMapScale != default_scale && bDiffuseMap;
  info.bSpecMapScale = m_specularMapScale != default_scale && bSpecMap;
  info.bNormalMapScale = m_normalMapScale != default_scale && bNormalMap;
  info.bReflectionMapScale = m_reflectionMapScale != default_scale && bReflectionMap;
  info.bDiffuseMapOffset = m_diffuseMapOffset != default_offset && bDiffuseMap;
  info.bSpecMapOffset = m_specularMapOffset != default_offset && bSpecMap;
  info.bNormalMapOffset = m_normalMapOffset != default_offset && bNormalMap;
  info.bReflectionMapOffset = m_reflectionMapOffset != default_offset && bReflectionMap;
  info.bAlphaTest = bAlphaTest;
  info.rasterMode = bAlphaBlend ? RasterMode::kAlphaBlend : RasterMode::kOpaque;
  info.renderPass = ri.renderPass;
  info.modelFormat = modelFormat;
  info.vertexAttributes = vertexAttributes;
  info.cullMode = cullMode;
  KRPipeline* pShader = getContext().getPipelineManager()->getPipeline(*ri.surface, info);

  // Bind bones
  if (pShader->hasPushConstant(ShaderValue::bone_transforms)) {
    float bone_mats[256 * 16];
    float* bone_mat_component = bone_mats;
    for (int bone_index = 0; bone_index < bones.size(); bone_index++) {
      KRBone* bone = bones[bone_index];

      //                Vector3 initialRotation = bone->getInitialLocalRotation();
      //                Vector3 rotation = bone->getLocalRotation();
      //                Vector3 initialTranslation = bone->getInitialLocalTranslation();
      //                Vector3 translation = bone->getLocalTranslation();
      //                Vector3 initialScale = bone->getInitialLocalScale();
      //                Vector3 scale = bone->getLocalScale();
      //                
                  //printf("%s - delta rotation: %.4f %.4f %.4f\n", bone->getName().c_str(), (rotation.x - initialRotation.x) * 180.0 / M_PI, (rotation.y - initialRotation.y) * 180.0 / M_PI, (rotation.z - initialRotation.z) * 180.0 / M_PI);
                  //printf("%s - delta translation: %.4f %.4f %.4f\n", bone->getName().c_str(), translation.x - initialTranslation.x, translation.y - initialTranslation.y, translation.z - initialTranslation.z);
      //                printf("%s - delta scale: %.4f %.4f %.4f\n", bone->getName().c_str(), scale.x - initialScale.x, scale.y - initialScale.y, scale.z - initialScale.z);

      Matrix4 skin_bone_bind_pose = bind_poses[bone_index];
      Matrix4 active_mat = bone->getActivePoseMatrix();
      Matrix4 inv_bind_mat = bone->getInverseBindPoseMatrix();
      Matrix4 inv_bind_mat2 = Matrix4::Invert(bind_poses[bone_index]);
      Matrix4 t = (inv_bind_mat * active_mat);
      Matrix4 t2 = inv_bind_mat2 * bone->getModelMatrix();
      for (int i = 0; i < 16; i++) {
        *bone_mat_component++ = t[i];
      }
    }
    if (pShader->hasPushConstant(ShaderValue::bone_transforms)) {
      pShader->setPushConstant(ShaderValue::bone_transforms, (Matrix4*)bone_mats, bones.size());
    }
  }

  if (bDiffuseMap) {
    m_diffuseMap.get()->resetPoolExpiry(lod_coverage, KRTexture::TEXTURE_USAGE_DIFFUSE_MAP);
    pShader->setImageBinding("diffuseTexture", m_diffuseMap.get(), getContext().getSamplerManager()->DEFAULT_WRAPPING_SAMPLER);
  }

  if (bSpecMap) {
    m_specularMap.get()->resetPoolExpiry(lod_coverage, KRTexture::TEXTURE_USAGE_SPECULAR_MAP);
    pShader->setImageBinding("specularTexture", m_specularMap.get(), getContext().getSamplerManager()->DEFAULT_WRAPPING_SAMPLER);
  }

  if (bNormalMap) {
    m_normalMap.get()->resetPoolExpiry(lod_coverage, KRTexture::TEXTURE_USAGE_NORMAL_MAP);
    pShader->setImageBinding("normalTexture", m_normalMap.get(), getContext().getSamplerManager()->DEFAULT_WRAPPING_SAMPLER);
  }

  if (bReflectionCubeMap) {
    m_reflectionCube.get()->resetPoolExpiry(lod_coverage, KRTexture::TEXTURE_USAGE_REFECTION_CUBE);
    pShader->setImageBinding("reflectionCubeTexture", m_reflectionCube.get(), getContext().getSamplerManager()->DEFAULT_CLAMPED_SAMPLER);
  }

  if (bReflectionMap) {
    m_reflectionMap.get()->resetPoolExpiry(lod_coverage, KRTexture::TEXTURE_USAGE_REFLECTION_MAP);
    pShader->setImageBinding("reflectionTexture", m_reflectionMap.get(), getContext().getSamplerManager()->DEFAULT_CLAMPED_SAMPLER);
  }

  ri.reflectedObjects.push_back(this);
  pShader->bind(ri, matModel);
  ri.reflectedObjects.pop_back();
}

const std::string& KRMaterial::getName() const
{
  return m_name;
}


bool KRMaterial::getShaderValue(ShaderValue value, float* output) const
{
  switch (value) {
  case ShaderValue::material_alpha:
    *output = m_tr;
    return true;
  case ShaderValue::material_shininess:
    *output = m_ns;
    return true;
  }
  return false;
}

bool KRMaterial::getShaderValue(ShaderValue value, hydra::Vector2* output) const
{
  switch (value) {
  case ShaderValue::diffusetexture_scale:
    *output = m_diffuseMapScale;
    return true;
  case ShaderValue::speculartexture_scale:
    *output = m_specularMapScale;
    return true;
  case ShaderValue::reflectiontexture_scale:
    *output = m_reflectionMapScale;
    return true;
  case ShaderValue::normaltexture_scale:
    *output = m_normalMapScale;
    return true;
  case ShaderValue::diffusetexture_offset:
    *output = m_diffuseMapOffset;
    return true;
  case ShaderValue::speculartexture_offset:
    *output = m_specularMapOffset;
    return true;
  case ShaderValue::reflectiontexture_offset:
    *output = m_reflectionMapOffset;
    return true;
  case ShaderValue::normaltexture_offset:
    *output = m_normalMapOffset;
    return true;
  }
  return false;
}

bool KRMaterial::getShaderValue(ShaderValue value, hydra::Vector3* output) const
{
  switch (value) {
  case ShaderValue::material_reflection:
    *output = m_reflectionColor;
    return true;
  case ShaderValue::material_ambient:
    *output = m_ambientColor;
    return true;
  case ShaderValue::material_diffuse:
    *output = m_diffuseColor;
    return true;
  case ShaderValue::material_specular:
    *output = m_specularColor;
    return true;
  }
  return false;
}
