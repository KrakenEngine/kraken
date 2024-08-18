//
//  KRMaterial.cpp
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
  m_pAmbientMap = NULL;
  m_pDiffuseMap = NULL;
  m_pSpecularMap = NULL;
  m_pNormalMap = NULL;
  m_pReflectionMap = NULL;
  m_pReflectionCube = NULL;
  m_ambientColor = Vector3::Zero();
  m_diffuseColor = Vector3::One();
  m_specularColor = Vector3::One();
  m_reflectionColor = Vector3::Zero();
  m_tr = 1.0f;
  m_ns = 0.0f;
  m_ambientMap = "";
  m_diffuseMap = "";
  m_specularMap = "";
  m_normalMap = "";
  m_reflectionMap = "";
  m_reflectionCube = "";
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
  return m_normalMap.size() > 0;
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
  if (m_ambientMap.size()) {
    stream << "\nmap_Ka " << m_ambientMap << ".pvr -s " << m_ambientMapScale.x << " " << m_ambientMapScale.y << " -o " << m_ambientMapOffset.x << " " << m_ambientMapOffset.y;
  } else {
    stream << "\n# map_Ka filename.pvr -s 1.0 1.0 -o 0.0 0.0";
  }
  if (m_diffuseMap.size()) {
    stream << "\nmap_Kd " << m_diffuseMap << ".pvr -s " << m_diffuseMapScale.x << " " << m_diffuseMapScale.y << " -o " << m_diffuseMapOffset.x << " " << m_diffuseMapOffset.y;
  } else {
    stream << "\n# map_Kd filename.pvr -s 1.0 1.0 -o 0.0 0.0";
  }
  if (m_specularMap.size()) {
    stream << "\nmap_Ks " << m_specularMap << ".pvr -s " << m_specularMapScale.x << " " << m_specularMapScale.y << " -o " << m_specularMapOffset.x << " " << m_specularMapOffset.y << "\n";
  } else {
    stream << "\n# map_Ks filename.pvr -s 1.0 1.0 -o 0.0 0.0";
  }
  if (m_normalMap.size()) {
    stream << "\nmap_Normal " << m_normalMap << ".pvr -s " << m_normalMapScale.x << " " << m_normalMapScale.y << " -o " << m_normalMapOffset.x << " " << m_normalMapOffset.y;
  } else {
    stream << "\n# map_Normal filename.pvr -s 1.0 1.0 -o 0.0 0.0";
  }
  if (m_reflectionMap.size()) {
    stream << "\nmap_Reflection " << m_reflectionMap << ".pvr -s " << m_reflectionMapScale.x << " " << m_reflectionMapScale.y << " -o " << m_reflectionMapOffset.x << " " << m_reflectionMapOffset.y;
  } else {
    stream << "\n# map_Reflection filename.pvr -s 1.0 1.0 -o 0.0 0.0";
  }
  if (m_reflectionCube.size()) {
    stream << "\nmap_ReflectionCube " << m_reflectionCube << ".pvr";
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
  m_ambientMap = texture_name;
  m_ambientMapScale = texture_scale;
  m_ambientMapOffset = texture_offset;
}

void KRMaterial::setDiffuseMap(std::string texture_name, Vector2 texture_scale, Vector2 texture_offset)
{
  m_diffuseMap = texture_name;
  m_diffuseMapScale = texture_scale;
  m_diffuseMapOffset = texture_offset;
}

void KRMaterial::setSpecularMap(std::string texture_name, Vector2 texture_scale, Vector2 texture_offset)
{
  m_specularMap = texture_name;
  m_specularMapScale = texture_scale;
  m_specularMapOffset = texture_offset;
}

void KRMaterial::setNormalMap(std::string texture_name, Vector2 texture_scale, Vector2 texture_offset)
{
  m_normalMap = texture_name;
  m_normalMapScale = texture_scale;
  m_normalMapOffset = texture_offset;
}

void KRMaterial::setReflectionMap(std::string texture_name, Vector2 texture_scale, Vector2 texture_offset)
{
  m_reflectionMap = texture_name;
  m_reflectionMapScale = texture_scale;
  m_reflectionMapOffset = texture_offset;
}

void KRMaterial::setReflectionCube(std::string texture_name)
{
  m_reflectionCube = texture_name;
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

  if (m_pAmbientMap) {
    m_pAmbientMap->resetPoolExpiry(lodCoverage, KRTexture::TEXTURE_USAGE_AMBIENT_MAP);
  }

  if (m_pDiffuseMap) {
    m_pDiffuseMap->resetPoolExpiry(lodCoverage, KRTexture::TEXTURE_USAGE_DIFFUSE_MAP);
  }

  if (m_pNormalMap) {
    m_pNormalMap->resetPoolExpiry(lodCoverage, KRTexture::TEXTURE_USAGE_NORMAL_MAP);
  }

  if (m_pSpecularMap) {
    m_pSpecularMap->resetPoolExpiry(lodCoverage, KRTexture::TEXTURE_USAGE_SPECULAR_MAP);
  }

  if (m_pReflectionMap) {
    m_pReflectionMap->resetPoolExpiry(lodCoverage, KRTexture::TEXTURE_USAGE_REFLECTION_MAP);
  }

  if (m_pReflectionCube) {
    m_pReflectionCube->resetPoolExpiry(lodCoverage, KRTexture::TEXTURE_USAGE_REFECTION_CUBE);
  }
}


kraken_stream_level KRMaterial::getStreamLevel()
{
  kraken_stream_level stream_level = kraken_stream_level::STREAM_LEVEL_IN_HQ;

  getTextures();

  if (m_pAmbientMap) {
    stream_level = KRMIN(stream_level, m_pAmbientMap->getStreamLevel(KRTexture::TEXTURE_USAGE_AMBIENT_MAP));
  }

  if (m_pDiffuseMap) {
    stream_level = KRMIN(stream_level, m_pDiffuseMap->getStreamLevel(KRTexture::TEXTURE_USAGE_DIFFUSE_MAP));
  }

  if (m_pNormalMap) {
    stream_level = KRMIN(stream_level, m_pNormalMap->getStreamLevel(KRTexture::TEXTURE_USAGE_NORMAL_MAP));
  }

  if (m_pSpecularMap) {
    stream_level = KRMIN(stream_level, m_pSpecularMap->getStreamLevel(KRTexture::TEXTURE_USAGE_SPECULAR_MAP));
  }

  if (m_pReflectionMap) {
    stream_level = KRMIN(stream_level, m_pReflectionMap->getStreamLevel(KRTexture::TEXTURE_USAGE_REFLECTION_MAP));
  }

  if (m_pReflectionCube) {
    stream_level = KRMIN(stream_level, m_pReflectionCube->getStreamLevel(KRTexture::TEXTURE_USAGE_REFECTION_CUBE));
  }

  return stream_level;
}

void KRMaterial::getTextures()
{
  if (!m_pAmbientMap && m_ambientMap.size()) {
    m_pAmbientMap = getContext().getTextureManager()->getTexture(m_ambientMap);
  }
  if (!m_pDiffuseMap && m_diffuseMap.size()) {
    m_pDiffuseMap = getContext().getTextureManager()->getTexture(m_diffuseMap);
  }
  if (!m_pNormalMap && m_normalMap.size()) {
    m_pNormalMap = getContext().getTextureManager()->getTexture(m_normalMap);
  }
  if (!m_pSpecularMap && m_specularMap.size()) {
    m_pSpecularMap = getContext().getTextureManager()->getTexture(m_specularMap);
  }
  if (!m_pReflectionMap && m_reflectionMap.size()) {
    m_pReflectionMap = getContext().getTextureManager()->getTexture(m_reflectionMap);
  }
  if (!m_pReflectionCube && m_reflectionCube.size()) {
    m_pReflectionCube = getContext().getTextureManager()->getTextureCube(m_reflectionCube.c_str());
  }
}

void KRMaterial::bind(const KRNode::RenderInfo& ri, ModelFormat modelFormat, __uint32_t vertexAttributes, CullMode cullMode, const std::vector<KRBone*>& bones, const std::vector<Matrix4>& bind_poses, const Matrix4& matModel, KRTexture* pLightMap, const Vector3& rim_color, float rim_power, float lod_coverage)
{
  bool bLightMap = pLightMap && ri.camera->settings.bEnableLightMap;

  getTextures();

  Vector2 default_scale = Vector2::One();
  Vector2 default_offset = Vector2::Zero();

  bool bHasReflection = m_reflectionColor != Vector3::Zero();
  bool bDiffuseMap = m_pDiffuseMap != NULL && ri.camera->settings.bEnableDiffuseMap;
  bool bNormalMap = m_pNormalMap != NULL && ri.camera->settings.bEnableNormalMap;
  bool bSpecMap = m_pSpecularMap != NULL && ri.camera->settings.bEnableSpecMap;
  bool bReflectionMap = m_pReflectionMap != NULL && ri.camera->settings.bEnableReflectionMap && ri.camera->settings.bEnableReflection && bHasReflection;
  bool bReflectionCubeMap = m_pReflectionCube != NULL && ri.camera->settings.bEnableReflection && bHasReflection;
  bool bAlphaTest = (m_alpha_mode == KRMATERIAL_ALPHA_MODE_TEST) && bDiffuseMap;
  bool bAlphaBlend = (m_alpha_mode == KRMATERIAL_ALPHA_MODE_BLENDONESIDE) || (m_alpha_mode == KRMATERIAL_ALPHA_MODE_BLENDTWOSIDE);

  PipelineInfo info{};
  std::string shader_name("ObjectShader");
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
  info.bRimColor = rim_power != 0.0f;
  info.renderPass = ri.renderPass;
  info.modelFormat = modelFormat;
  info.vertexAttributes = vertexAttributes;
  info.cullMode = cullMode;
  KRPipeline* pShader = getContext().getPipelineManager()->getPipeline(*ri.surface, info);

  // Rim highlighting parameters
  pShader->setPushConstant(KRPipeline::PushConstant::rim_color, rim_color);
  pShader->setPushConstant(KRPipeline::PushConstant::rim_power, rim_power);

  // Bind bones
  if (pShader->hasPushConstant(KRPipeline::PushConstant::bone_transforms)) {
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
    if (pShader->hasPushConstant(KRPipeline::PushConstant::bone_transforms)) {
      pShader->setPushConstant(KRPipeline::PushConstant::bone_transforms, (Matrix4*)bone_mats, bones.size());
    }
  }


  pShader->setPushConstant(KRPipeline::PushConstant::material_ambient, m_ambientColor + ri.camera->settings.ambient_intensity);

  if (ri.renderPass->getType() == RenderPassType::RENDER_PASS_FORWARD_OPAQUE) {
    // We pre-multiply the light color with the material color in the forward renderer
    pShader->setPushConstant(KRPipeline::PushConstant::material_diffuse, Vector3::Create(m_diffuseColor.x * ri.camera->settings.light_intensity.x, m_diffuseColor.y * ri.camera->settings.light_intensity.y, m_diffuseColor.z * ri.camera->settings.light_intensity.z));
  } else {
    pShader->setPushConstant(KRPipeline::PushConstant::material_diffuse, m_diffuseColor);
  }

  if (ri.renderPass->getType() == RenderPassType::RENDER_PASS_FORWARD_OPAQUE) {
    // We pre-multiply the light color with the material color in the forward renderer
    pShader->setPushConstant(KRPipeline::PushConstant::material_specular, Vector3::Create(m_specularColor.x * ri.camera->settings.light_intensity.x, m_specularColor.y * ri.camera->settings.light_intensity.y, m_specularColor.z * ri.camera->settings.light_intensity.z));
  } else {
    pShader->setPushConstant(KRPipeline::PushConstant::material_specular, m_specularColor);
  }

  pShader->setPushConstant(KRPipeline::PushConstant::material_shininess, m_ns);
  pShader->setPushConstant(KRPipeline::PushConstant::material_reflection, m_reflectionColor);
  pShader->setPushConstant(KRPipeline::PushConstant::diffusetexture_scale, m_diffuseMapScale);
  pShader->setPushConstant(KRPipeline::PushConstant::speculartexture_scale, m_specularMapScale);
  pShader->setPushConstant(KRPipeline::PushConstant::reflectiontexture_scale, m_reflectionMapScale);
  pShader->setPushConstant(KRPipeline::PushConstant::normaltexture_scale, m_normalMapScale);
  pShader->setPushConstant(KRPipeline::PushConstant::diffusetexture_offset, m_diffuseMapOffset);
  pShader->setPushConstant(KRPipeline::PushConstant::speculartexture_offset, m_specularMapOffset);
  pShader->setPushConstant(KRPipeline::PushConstant::reflectiontexture_offset, m_reflectionMapOffset);
  pShader->setPushConstant(KRPipeline::PushConstant::normaltexture_offset, m_normalMapOffset);

  pShader->setPushConstant(KRPipeline::PushConstant::material_alpha, m_tr);

  if (bDiffuseMap) {
    m_pDiffuseMap->resetPoolExpiry(lod_coverage, KRTexture::TEXTURE_USAGE_DIFFUSE_MAP);
    pShader->setImageBinding("diffuseTexture", m_pDiffuseMap, getContext().getSamplerManager()->DEFAULT_WRAPPING_SAMPLER);
  }

  if (bSpecMap) {
    m_pSpecularMap->resetPoolExpiry(lod_coverage, KRTexture::TEXTURE_USAGE_SPECULAR_MAP);
    pShader->setImageBinding("specularTexture", m_pDiffuseMap, getContext().getSamplerManager()->DEFAULT_WRAPPING_SAMPLER);
  }

  if (bNormalMap) {
    m_pNormalMap->resetPoolExpiry(lod_coverage, KRTexture::TEXTURE_USAGE_NORMAL_MAP);
    pShader->setImageBinding("normalTexture", m_pNormalMap, getContext().getSamplerManager()->DEFAULT_WRAPPING_SAMPLER);
  }

  if (bReflectionCubeMap) {
    m_pReflectionCube->resetPoolExpiry(lod_coverage, KRTexture::TEXTURE_USAGE_REFECTION_CUBE);
    pShader->setImageBinding("reflectionCubeTexture", m_pReflectionCube, getContext().getSamplerManager()->DEFAULT_CLAMPED_SAMPLER);
  }

  if (bReflectionMap) {
    m_pReflectionMap->resetPoolExpiry(lod_coverage, KRTexture::TEXTURE_USAGE_REFLECTION_MAP);
    pShader->setImageBinding("reflectionTexture", m_pReflectionMap, getContext().getSamplerManager()->DEFAULT_CLAMPED_SAMPLER);
  }

  pShader->bind(ri.commandBuffer, *ri.camera, ri.viewport, matModel, &ri.point_lights, &ri.directional_lights, &ri.spot_lights, ri.renderPass);
}

const std::string& KRMaterial::getName() const
{
  return m_name;
}

