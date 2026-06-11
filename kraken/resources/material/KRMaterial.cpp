//
//  KRMaterial.cpp
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

#include "KREngine-common.h"

#include "KRMaterial.h"
#include "resources/texture/KRTextureManager.h"
#include "KRRenderPass.h"

#include "KRContext.h"
#include "simdjson.h"

using namespace mimir;
using namespace hydra;
using namespace simdjson;

namespace simdjson
{
template <typename builder_type>
void tag_invoke(serialize_tag, builder_type& builder, const KRMaterial::TextureMap& texture)
{
  builder.start_object();
  builder.template append_key_value<"texture">(texture.texture.getName());
  builder.append_comma();
  builder.template append_key_value<"offset">(texture.offset);
  builder.append_comma();
  builder.template append_key_value<"scale">(texture.scale);
  builder.append_comma();
  builder.template append_key_value<"rotation">(texture.rotation);
  builder.append_comma();
  switch(texture.wrapS) {
    case KRMaterial::texture_wrap_type::KRMATERIAL_TEXTURE_CLAMP:
      builder.template append_key_value<"wrapS">("clamp");
      break;
    case KRMaterial::texture_wrap_type::KRMATERIAL_TEXTURE_REPEAT:
      builder.template append_key_value<"wrapS">("repeat");
      break;
    case KRMaterial::texture_wrap_type::KRMATERIAL_TEXTURE_MIRROR_REPEAT:
      builder.template append_key_value<"wrapS">("repeat_mirror");
      break;
  }
  builder.append_comma();
  switch(texture.wrapT) {
    case KRMaterial::texture_wrap_type::KRMATERIAL_TEXTURE_CLAMP:
      builder.template append_key_value<"wrapT">("clamp");
      break;
    case KRMaterial::texture_wrap_type::KRMATERIAL_TEXTURE_REPEAT:
      builder.template append_key_value<"wrapT">("repeat");
      break;
    case KRMaterial::texture_wrap_type::KRMATERIAL_TEXTURE_MIRROR_REPEAT:
      builder.template append_key_value<"wrapT">("repeat_mirror");
      break;
  }
  builder.append_comma();
  switch(texture.magFilter) {
    case KRMaterial::texture_mag_filter_type::KRMATERIAL_TEXTURE_MAG_NEAREST:
      builder.template append_key_value<"magFilter">("nearest");
      break;
    case KRMaterial::texture_mag_filter_type::KRMATERIAL_TEXTURE_MAG_LINEAR:
      builder.template append_key_value<"magFilter">("linear");
      break;
  }
  builder.append_comma();
  switch(texture.minFilter) {
    case KRMaterial::texture_min_filter_type::KRMATERIAL_TEXTURE_MIN_NEAREST:
      builder.template append_key_value<"minFilter">("nearest");
      break;
    case KRMaterial::texture_min_filter_type::KRMATERIAL_TEXTURE_MIN_LINEAR:
      builder.template append_key_value<"minFilter">("linear");
      break;
    case KRMaterial::texture_min_filter_type::KRMATERIAL_TEXTURE_MIN_NEAREST_MIPMAP_NEAREST:
      builder.template append_key_value<"minFilter">("nearest_mipmap_nearest");
      break;
    case KRMaterial::texture_min_filter_type::KRMATERIAL_TEXTURE_MIN_LINEAR_MIPMAP_NEAREST:
      builder.template append_key_value<"minFilter">("linear_mipmap_nearest");
      break;
    case KRMaterial::texture_min_filter_type::KRMATERIAL_TEXTURE_MIN_NEAREST_MIPMAP_LINEAR:
      builder.template append_key_value<"minFilter">("nearest_mipmap_linear");
      break;
    case KRMaterial::texture_min_filter_type::KRMATERIAL_TEXTURE_MIN_LINEAR_MIPMAP_LINEAR:
      builder.template append_key_value<"minFilter">("linear_mipmap_linear");
      break;
  }
  builder.end_object();
}

} // namespace simdjson

simdjson::error_code KRMaterial::TextureMap::parse(simdjson::ondemand::value &val)
{
  ondemand::object obj;
  auto error = val.get_object().get(obj);
  if (error) {
    return error;
  }
  
  std::string textureName;
  if ((error = obj["texture"].get_string().get(textureName))) {
    return error;
  }
  texture.set(textureName);
  
  if ((error = obj["offset"].get(offset))) {
    return error;
  }
  
  if ((error = obj["scale"].get(scale))) {
    return error;
  }
  
  if ((error = obj["rotation"].get(rotation))) {
    return error;
  }
  
  std::string strWrapS;
  std::string strWrapT;
  std::string strMinFilter;
  std::string strMagFilter;
  
  if ((error = obj["wrapS"].get(strWrapS))) {
    return error;
  }
  
  if (strWrapS == "clamp") {
    wrapS = KRMATERIAL_TEXTURE_CLAMP;
  } else if(strWrapS == "repeat") {
    wrapS = KRMATERIAL_TEXTURE_REPEAT;
  } else if(strWrapS == "repeat_mirror") {
    wrapS = KRMATERIAL_TEXTURE_MIRROR_REPEAT;
  } else {
    KRContext::Log(KRContext::LOG_LEVEL_ERROR, "Kraken - Unknown material texture wrap: %s.", strWrapS.c_str());
    wrapS = KRMATERIAL_TEXTURE_REPEAT;
  }
  
  if ((error = obj["wrapT"].get(strWrapT))) {
    return error;
  }
  
  if (strWrapT == "clamp") {
    wrapT = KRMATERIAL_TEXTURE_CLAMP;
  } else if(strWrapT == "repeat") {
    wrapT = KRMATERIAL_TEXTURE_REPEAT;
  } else if(strWrapT == "repeat_mirror") {
    wrapT = KRMATERIAL_TEXTURE_MIRROR_REPEAT;
  } else {
    KRContext::Log(KRContext::LOG_LEVEL_ERROR, "Kraken - Unknown material texture wrap: %s.", strWrapT.c_str());
    wrapS = KRMATERIAL_TEXTURE_REPEAT;
  }
  
  if ((error = obj["minFilter"].get(strMinFilter))) {
    return error;
  }
  
  if (strMinFilter == "nearest") {
    minFilter = KRMATERIAL_TEXTURE_MIN_NEAREST;
  } else if (strMinFilter =="linear") {
    minFilter = KRMATERIAL_TEXTURE_MIN_LINEAR;
  } else if (strMinFilter == "nearest_mipmap_nearest") {
    minFilter = KRMATERIAL_TEXTURE_MIN_NEAREST_MIPMAP_NEAREST;
  } else if (strMinFilter == "linear_mipmap_nearest") {
    minFilter = KRMATERIAL_TEXTURE_MIN_LINEAR_MIPMAP_NEAREST;
  } else if (strMinFilter == "nearest_mipmap_linear") {
    minFilter = KRMATERIAL_TEXTURE_MIN_NEAREST_MIPMAP_LINEAR;
  } else if (strMinFilter == "linear_mipmap_linear") {
    minFilter = KRMATERIAL_TEXTURE_MIN_LINEAR_MIPMAP_LINEAR;
  } else {
    KRContext::Log(KRContext::LOG_LEVEL_ERROR, "Kraken - Unknown material texture min filter: %s.", strMinFilter.c_str());
    minFilter = KRMATERIAL_TEXTURE_MIN_LINEAR;
  }
  
  if ((error = obj["magFilter"].get(strMagFilter))) {
    return error;
  }
  
  if (strMagFilter == "nearest") {
    magFilter = KRMATERIAL_TEXTURE_MAG_NEAREST;
  } else if (strMagFilter == "linear") {
    magFilter = KRMATERIAL_TEXTURE_MAG_LINEAR;
  } else {
    KRContext::Log(KRContext::LOG_LEVEL_ERROR, "Kraken - Unknown material texture mag filter: %s.", strMagFilter.c_str());
    magFilter = KRMATERIAL_TEXTURE_MAG_LINEAR;
  }
  
  return simdjson::SUCCESS;
}


KRMaterial::KRMaterial(KRContext& context, const char* name)
  : KRResource(context, name)
{
}

KRMaterial::KRMaterial(KRContext& context, std::string name, mimir::Block* data)
  : KRResource(context, name)
{
  simdjson::ondemand::parser parser;
  simdjson::ondemand::document doc;
  
  /*
  char* str = (char*)data->getStart();
  OutputDebugStringA("\n\n----====----\n");
  OutputDebugStringA(str);
  OutputDebugStringA("\n----====----\n\n");
  */

  mimir::Block paddedData;
  paddedData.append(*data);
  paddedData.expand(SIMDJSON_PADDING);
  paddedData.lock();
  auto error = parser.iterate((const char*)paddedData.getStart(), paddedData.getSize()).get(doc);
  paddedData.unlock();

  if (error) {
    // TODO - Report and handle error
    return;
  }
  
  ondemand::object jsonRoot;
  if(!tryJsonRequired(doc.get_object().get(jsonRoot))) {
    // TODO - Report and handle error
    return;
  }

  std::string_view alphaModeText;
  if (tryJson(jsonRoot["alphaMode"].get_string().get(alphaModeText))) {
    if (alphaModeText.compare("opaque") == 0) {
      m_alphaMode = KRMATERIAL_ALPHA_MODE_OPAQUE;
    } else if (alphaModeText.compare("blend") == 0) {
      m_alphaMode = KRMATERIAL_ALPHA_MODE_BLEND;
    } else if (alphaModeText.compare("test") == 0) {
      m_alphaMode = KRMATERIAL_ALPHA_MODE_TEST;
    } else {
      KRContext::Log(KRContext::LOG_LEVEL_ERROR, "Kraken - Unknown material alphaMode: %s for %s.", alphaModeText, name.c_str());
    }
  }
  
  tryJson(jsonRoot["alphaCutoff"].get(m_alphaCutoff));
  tryJson(jsonRoot["ior"].get(m_ior));
  tryJson(jsonRoot["dispersion"].get(m_dispersion));

  m_shadingModel = KRMATERIAL_SHADING_MODEL_PBR;
  std::string_view shadingModelText;
  if (tryJson(jsonRoot["shadingModel"].get_string().get(shadingModelText))) {
    if (shadingModelText.compare("unlit") == 0) {
      m_shadingModel = KRMATERIAL_SHADING_MODEL_UNLIT;
    } else if (shadingModelText.compare("pbr") == 0) {
      m_shadingModel = KRMATERIAL_SHADING_MODEL_PBR;
    } else {
      KRContext::Log(KRContext::LOG_LEVEL_ERROR, "Kraken - Unknown material shadingModel: %s for %s.", shadingModelText, name.c_str());
    }
  }
  
  {
    simdjson::ondemand::object baseColorObj;
    if (tryJson(jsonRoot["baseColor"].get_object().get(baseColorObj))) {
      simdjson::ondemand::value mapVal;
      if (tryJson(baseColorObj["map"].get(mapVal))) {
        tryJson(m_baseColorMap.parse(mapVal));
      }
      tryJson(baseColorObj["factor"].get(m_baseColorFactor));
    }
  }
  
  {
    simdjson::ondemand::object normalObj;
    if (tryJson(jsonRoot["normal"].get_object().get(normalObj))) {
      simdjson::ondemand::value mapVal;
      if (tryJson(normalObj["map"].get(mapVal))) {
        tryJson(m_normalMap.parse(mapVal));
      }
      tryJson(normalObj["scale"].get(m_normalScale));
    }
  }
  
  {
    simdjson::ondemand::object emissiveObj;
    if (tryJson(jsonRoot["emissive"].get_object().get(emissiveObj))) {
      simdjson::ondemand::value mapVal;
      if (tryJson(emissiveObj["map"].get(mapVal))) {
        tryJson(m_emissiveMap.parse(mapVal));
      }
      tryJson(emissiveObj["factor"].get(m_emissiveFactor));
    }
  }
  
  {
    simdjson::ondemand::object occlusionObj;
    if (tryJson(jsonRoot["occlusion"].get_object().get(occlusionObj))) {
      simdjson::ondemand::value mapVal;
      if (tryJson(occlusionObj["map"].get(mapVal))) {
        tryJson(m_occlusionMap.parse(mapVal));
      }
      tryJson(occlusionObj["strength"].get(m_occlusionStrength));
    }
  }
  
  {
    simdjson::ondemand::object metalicRoughnessObj;
    if (tryJson(jsonRoot["metalicRoughness"].get_object().get(metalicRoughnessObj))) {
      simdjson::ondemand::value mapVal;
      if (tryJson(metalicRoughnessObj["map"].get(mapVal))) {
        tryJson(m_metalicRoughnessMap.parse(mapVal));
      }
      tryJson(metalicRoughnessObj["metalicFactor"].get(m_metalicFactor));
      tryJson(metalicRoughnessObj["roughnessFactor"].get(m_roughnessFactor));
    }
  }

  {
    simdjson::ondemand::object anisotropyObj;
    if (tryJson(jsonRoot["anisotropy"].get_object().get(anisotropyObj))) {
      simdjson::ondemand::value mapVal;
      if (tryJson(anisotropyObj["map"].get(mapVal))) {
        tryJson(m_anisotropyMap.parse(mapVal));
      }
      tryJson(anisotropyObj["strength"].get(m_anisotropyStrength));
      tryJson(anisotropyObj["rotation"].get(m_anisotropyRotation));
    }
  }
  
  {
    simdjson::ondemand::object clearcoatObj;
    if (tryJson(jsonRoot["clearcoat"].get_object().get(clearcoatObj))) {
      simdjson::ondemand::value mapVal;
      error = clearcoatObj["map"].get(mapVal);
      if (error == simdjson::SUCCESS) {
        tryJson(m_clearcoatMap.parse(mapVal));
      }
      tryJson(clearcoatObj["factor"].get(m_clearcoatFactor));
      if (tryJson(clearcoatObj["roughnessMap"].get(mapVal))) {
        tryJson(m_clearcoatRoughnessMap.parse(mapVal));
      }
      tryJson(clearcoatObj["roughnessFactor"].get(m_clearcoatRoughnessFactor));
      if (tryJson(clearcoatObj["normalMap"].get(mapVal))) {
        tryJson(m_clearcoatNormalMap.parse(mapVal));
      }
      tryJson(clearcoatObj["clearcoatNormalScale"].get(m_clearcoatNormalScale));
    }
  }
  
  {
    simdjson::ondemand::object specularObj;
    if (tryJson(jsonRoot["specular"].get_object().get(specularObj))) {
      simdjson::ondemand::value mapVal;
      if (tryJson(specularObj["map"].get(mapVal))) {
        tryJson(m_specularMap.parse(mapVal));
      }
      tryJson(specularObj["factor"].get(m_specularFactor));
      if (tryJson(specularObj["colorMap"].get(mapVal))) {
        tryJson(m_specularColorMap.parse(mapVal));
      }
      tryJson(specularObj["colorFactor"].get(m_specularColorFactor));
    }
  }
  
  {
    simdjson::ondemand::object thicknessObj;
    if (tryJson(jsonRoot["thickness"].get_object().get(thicknessObj))) {
      simdjson::ondemand::value mapVal;
      if (tryJson(thicknessObj["map"].get(mapVal))) {
        tryJson(m_thicknessMap.parse(mapVal));
      }
      tryJson(thicknessObj["factor"].get(m_thicknessFactor));
      tryJson(thicknessObj["attenuiationDistance"].get(m_attenuationDistance));
      tryJson(thicknessObj["attenuationColor"].get(m_attenuationColor));
    }
  }
  
  {
    simdjson::ondemand::object transmissionObj;
    if (tryJson(jsonRoot["transmission"].get_object().get(transmissionObj))) {
      simdjson::ondemand::value mapVal;
      if (tryJson(transmissionObj["map"].get(mapVal))) {
        tryJson(m_transmissionMap.parse(mapVal));
      }
      tryJson(transmissionObj["factor"].get(m_transmissionFactor));
    }
  }
  
}

KRMaterial::~KRMaterial()
{

}

std::string KRMaterial::getExtension()
{
  return "krmaterial";
}

bool KRMaterial::needsVertexTangents()
{
  return m_normalMap.texture.isSet();
}

bool KRMaterial::save(Block& data)
{
  simdjson::builder::string_builder sb;
  sb.start_object();
  sb.append_key_value<"name">(getName());
  sb.append_comma();
  switch (m_alphaMode) {
  case KRMATERIAL_ALPHA_MODE_OPAQUE:
    sb.append_key_value<"alphaMode">("opaque");
    break;
  case KRMATERIAL_ALPHA_MODE_BLEND:
    sb.append_key_value<"alphaMode">("blend");
    break;
  case KRMATERIAL_ALPHA_MODE_TEST:
    sb.append_key_value<"alphaMode">("test");
    break;
  }
  sb.append_comma();
  sb.append_key_value<"alphaCutoff">(m_alphaCutoff);
  sb.append_comma();
  sb.append_key_value<"ior">(m_ior);
  sb.append_comma();
  sb.append_key_value<"dispersion">(m_dispersion);
  sb.append_comma();
  switch (m_shadingModel) {
  case KRMATERIAL_SHADING_MODEL_UNLIT:
    sb.append_key_value<"shadingModel">("unlit");
    break;
  case KRMATERIAL_SHADING_MODEL_PBR:
    sb.append_key_value<"shadingModel">("pbr");
    break;
  }

  sb.append_comma();
  sb.escape_and_append_with_quotes("baseColor");
  sb.append_colon();

  sb.start_object();
  sb.append_key_value<"map">(m_baseColorMap);
  sb.append_comma();
  sb.append_key_value<"factor">(m_baseColorFactor);
  sb.end_object();

  sb.append_comma();

  sb.escape_and_append_with_quotes("normal");
  sb.append_colon();

  sb.start_object();
  sb.append_key_value<"map">(m_normalMap);
  sb.append_comma();
  sb.append_key_value<"scale">(m_normalScale);
  sb.end_object();

  sb.append_comma();

  sb.escape_and_append_with_quotes("emissive");
  sb.append_colon();

  sb.start_object();
  sb.append_key_value<"map">(m_emissiveMap);
  sb.append_comma();
  sb.append_key_value<"factor">(m_emissiveFactor);
  sb.end_object();


  sb.append_comma();


  sb.escape_and_append_with_quotes("occlusion");
  sb.append_colon();

  sb.start_object();
  sb.append_key_value<"map">(m_occlusionMap);
  sb.append_comma();
  sb.append_key_value<"strength">(m_occlusionStrength);
  sb.end_object();

  sb.append_comma();

  sb.escape_and_append_with_quotes("metalicRoughness");
  sb.append_colon();

  sb.start_object();
  sb.append_key_value<"map">(m_metalicRoughnessMap);
  sb.append_comma();
  sb.append_key_value<"metalicFactor">(m_metalicFactor);
  sb.append_comma();
  sb.append_key_value<"roughnessFactor">(m_roughnessFactor);
  sb.end_object();

  sb.append_comma();

  sb.escape_and_append_with_quotes("anisotropy");
  sb.append_colon();

  sb.start_object();
  sb.append_key_value<"map">(m_anisotropyMap);
  sb.append_comma();
  sb.append_key_value<"strength">(m_anisotropyStrength);
  sb.append_comma();
  sb.append_key_value<"rotation">(m_anisotropyRotation);
  sb.end_object();

  sb.append_comma();

  sb.escape_and_append_with_quotes("clearcoat");
  sb.append_colon();

  sb.start_object();
  sb.append_key_value<"map">(m_clearcoatMap);
  sb.append_comma();
  sb.append_key_value<"factor">(m_clearcoatFactor);
  sb.append_comma();
  sb.append_key_value<"roughnessMap">(m_clearcoatRoughnessMap);
  sb.append_comma();
  sb.append_key_value<"roughnessFactor">(m_clearcoatRoughnessFactor);
  sb.append_comma();
  sb.append_key_value<"normalMap">(m_clearcoatNormalMap);
  sb.append_comma();
  sb.append_key_value<"normalScale">(m_clearcoatNormalScale);
  sb.end_object();

  sb.append_comma();

  sb.escape_and_append_with_quotes("specular");
  sb.append_colon();

  sb.start_object();
  sb.append_key_value<"map">(m_specularMap);
  sb.append_comma();
  sb.append_key_value<"factor">(m_specularFactor);
  sb.append_comma();
  sb.append_key_value<"colorMap">(m_specularColorMap);
  sb.append_comma();
  sb.append_key_value<"colorFactor">(m_specularColorFactor);
  sb.end_object();

  sb.append_comma();

  sb.escape_and_append_with_quotes("thickness");
  sb.append_colon();

  sb.start_object();
  sb.append_key_value<"map">(m_thicknessMap);
  sb.append_comma();
  sb.append_key_value<"factor">(m_thicknessFactor);
  sb.append_comma();
  sb.append_key_value<"attenuiationDistance">(m_attenuationDistance);
  sb.append_comma();
  sb.append_key_value<"attenuationColor">(m_attenuationColor);
  sb.end_object();


  sb.append_comma();

  sb.escape_and_append_with_quotes("transmission");
  sb.append_colon();

  sb.start_object();
  sb.append_key_value<"map">(m_transmissionMap);
  sb.append_comma();
  sb.append_key_value<"factor">(m_transmissionFactor);
  sb.end_object();


  sb.end_object();


  std::string_view view;
  const char* str = nullptr;
  auto error = sb.view().get(view);
  if (error) {
    return false;
    // TODO - Report error
  }
  data.append((void*) & view.front(), view.length());

  return true;
}

/*
void KRMaterial::setAmbientMap(std::string texture_name, Vector2 texture_scale, Vector2 texture_offset)
{
  m_ambient.texture.set(texture_name);
  m_ambient.scale = texture_scale;
  m_ambient.offset = texture_offset;
}

void KRMaterial::setDiffuseMap(std::string texture_name, Vector2 texture_scale, Vector2 texture_offset)
{
  m_diffuse.texture.set(texture_name);
  m_diffuse.scale = texture_scale;
  m_diffuse.offset = texture_offset;
}

void KRMaterial::setSpecularMap(std::string texture_name, Vector2 texture_scale, Vector2 texture_offset)
{
  m_specular.texture.set(texture_name);
  m_specular.scale = texture_scale;
  m_specular.offset = texture_offset;
}

void KRMaterial::setNormalMap(std::string texture_name, Vector2 texture_scale, Vector2 texture_offset)
{
  m_normal.texture.set(texture_name);
  m_normal.scale = texture_scale;
  m_normal.offset = texture_offset;
}

void KRMaterial::setReflectionMap(std::string texture_name, Vector2 texture_scale, Vector2 texture_offset)
{
  m_reflection.texture.set(texture_name);
  m_reflection.scale = texture_scale;
  m_reflection.offset = texture_offset;
}

void KRMaterial::setReflectionCube(std::string texture_name)
{
  m_reflectionCube.set(texture_name);
}
*/

void KRMaterial::setAlphaMode(KRMaterial::alpha_mode_type alpha_mode)
{
  m_alphaMode = alpha_mode;
}

KRMaterial::alpha_mode_type KRMaterial::getAlphaMode()
{
  return m_alphaMode;
}

void KRMaterial::setTransparency(float a)
{
  if (a < 1.0f && m_alphaMode == KRMaterial::KRMATERIAL_ALPHA_MODE_OPAQUE) {
    setAlphaMode(KRMaterial::KRMATERIAL_ALPHA_MODE_BLEND);
  }
  m_baseColorFactor[3] = a;
}

void KRMaterial::setShininess(float s)
{
  m_roughnessFactor = 1.0f - s;
}

bool KRMaterial::isTransparent()
{
  return m_baseColorFactor[3] < 1.0 || m_alphaMode == KRMATERIAL_ALPHA_MODE_BLEND;
}

void KRMaterial::getResourceBindings(std::list<KRResourceBinding*>& bindings)
{
  KRResource::getResourceBindings(bindings);

  bindings.push_back(&m_baseColorMap.texture);
  bindings.push_back(&m_normalMap.texture);
  bindings.push_back(&m_emissiveMap.texture);
  bindings.push_back(&m_occlusionMap.texture);
  bindings.push_back(&m_metalicRoughnessMap.texture);
  bindings.push_back(&m_anisotropyMap.texture);
  bindings.push_back(&m_clearcoatMap.texture);
  bindings.push_back(&m_clearcoatRoughnessMap.texture);
  bindings.push_back(&m_clearcoatNormalMap.texture);
  bindings.push_back(&m_specularMap.texture);
  bindings.push_back(&m_specularColorMap.texture);
  bindings.push_back(&m_thicknessMap.texture);
  bindings.push_back(&m_transmissionMap.texture);
}

kraken_stream_level KRMaterial::getStreamLevel()
{
  kraken_stream_level stream_level = kraken_stream_level::STREAM_LEVEL_IN_HQ;

  if (m_baseColorMap.texture.isBound()) {
    stream_level = std::min(stream_level, m_baseColorMap.texture.get()->getStreamLevel());
  }

  if (m_normalMap.texture.isBound()) {
    stream_level = std::min(stream_level, m_normalMap.texture.get()->getStreamLevel());
  }

  if (m_occlusionMap.texture.isBound()) {
    stream_level = std::min(stream_level, m_occlusionMap.texture.get()->getStreamLevel());
  }

  if (m_metalicRoughnessMap.texture.isBound()) {
    stream_level = std::min(stream_level, m_metalicRoughnessMap.texture.get()->getStreamLevel());
  }

  if (m_anisotropyMap.texture.isBound()) {
    stream_level = std::min(stream_level, m_anisotropyMap.texture.get()->getStreamLevel());
  }

  if (m_clearcoatMap.texture.isBound()) {
    stream_level = std::min(stream_level, m_clearcoatMap.texture.get()->getStreamLevel());
  }

  if (m_clearcoatRoughnessMap.texture.isBound()) {
    stream_level = std::min(stream_level, m_clearcoatRoughnessMap.texture.get()->getStreamLevel());
  }

  if (m_clearcoatNormalMap.texture.isBound()) {
    stream_level = std::min(stream_level, m_clearcoatNormalMap.texture.get()->getStreamLevel());
  }

  if (m_specularMap.texture.isBound()) {
    stream_level = std::min(stream_level, m_specularMap.texture.get()->getStreamLevel());
  }

  if (m_specularColorMap.texture.isBound()) {
    stream_level = std::min(stream_level, m_specularColorMap.texture.get()->getStreamLevel());
  }

  if (m_thicknessMap.texture.isBound()) {
    stream_level = std::min(stream_level, m_thicknessMap.texture.get()->getStreamLevel());
  }

  return stream_level;
}

bool KRMaterial::bind(KRNode::RenderInfo& ri, const VertexBufferLayout* layout, CullMode cullMode, const std::vector<KRBone*>& bones, const std::vector<Matrix4>& bind_poses, const Matrix4& matModel, KRTexture* pLightMap, float lod_coverage)
{
  bool bLightMap = pLightMap && ri.camera->settings.bEnableLightMap;

  Vector2 default_scale = Vector2::One();
  Vector2 default_offset = Vector2::Zero();

  bool bDiffuseMap = m_baseColorMap.texture.isBound() && ri.camera->settings.bEnableDiffuseMap;
  bool bNormalMap = m_normalMap.texture.isBound() && ri.camera->settings.bEnableNormalMap;
  bool bSpecMap = false;
  bool bReflectionMap = false;
  bool bReflectionCubeMap = false;
  bool bAlphaTest = m_alphaMode == KRMATERIAL_ALPHA_MODE_TEST;
  bool bAlphaBlend = m_alphaMode == KRMATERIAL_ALPHA_MODE_BLEND;

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
  info.bDiffuseMapScale = m_baseColorMap.scale != default_scale && bDiffuseMap;
  info.bSpecMapScale = false;
  info.bNormalMapScale = m_normalMap.scale != default_scale && bNormalMap;
  info.bReflectionMapScale = false;
  info.bDiffuseMapOffset = false;
  info.bSpecMapOffset = false;
  info.bNormalMapOffset = m_normalMap.offset != default_offset && bNormalMap;
  info.bReflectionMapOffset = false;
  info.bAlphaTest = bAlphaTest;
  info.rasterMode = bAlphaBlend ? RasterMode::kAlphaBlend : RasterMode::kOpaque;
  info.renderPass = ri.renderPass;
  info.layout = layout;
  info.cullMode = cullMode;
  KRPipeline* pShader = getContext().getPipelineManager()->getPipeline(*ri.surface, info);
  if (pShader == nullptr) {
    return false;
  }

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
    /*
    * TODO: Implement Skinned Mesh bone transform buffer upload
    if (pShader->hasPushConstant(ShaderValue::bone_transforms)) {
      pShader->setPushConstant(ShaderValue::bone_transforms, (Matrix4*)bone_mats, bones.size());
    }
    */
  }

  bool success = true;
  ri.reflectedObjects.push_back(this);
  if (!pShader->bind(ri, matModel)) {
    success = false;
  }
  ri.reflectedObjects.pop_back();
  return success;
}

bool KRMaterial::getShaderValue(const KRCamera* camera, ShaderValue value, float* output) const
{
  switch (value) {
    case ShaderValue::material_baseColor_map_rotation:
      *output = m_baseColorMap.rotation;
      return true;
    case ShaderValue::material_normal_map_rotation:
      *output = m_normalMap.rotation;
      return true;
    case ShaderValue::material_normal_scale:
      *output = m_normalScale;
      return true;
    case ShaderValue::material_emissive_map_rotation:
      *output = m_emissiveMap.rotation;
      return true;
    case ShaderValue::material_occlusion_map_rotation:
      *output = m_occlusionMap.rotation;
      return true;
    case ShaderValue::material_occlusion_strength:
      *output = m_occlusionStrength;
      return true;
    case ShaderValue::material_metalicRoughness_map_rotation:
      *output = m_metalicRoughnessMap.rotation;
      return true;
    case ShaderValue::material_metalic_factor:
      *output = m_metalicFactor;
      return true;
    case ShaderValue::material_roughness_factor:
      *output = m_roughnessFactor;
      return true;
    case ShaderValue::material_alphaMode:
      *output = m_alphaMode;
      return true;
    case ShaderValue::material_alphaCutoff:
      *output = m_alphaCutoff;
      return true;
    case ShaderValue::material_ior:
      *output = m_ior;
      return true;
    case ShaderValue::material_anisotropy_map_rotation:
      *output = m_anisotropyMap.rotation;
      return true;
    case ShaderValue::material_anisotropy_strength:
      *output = m_anisotropyStrength;
      return true;
    case ShaderValue::material_anisotropy_rotation:
      *output = m_anisotropyRotation;
      return true;
    case ShaderValue::material_clearcoat_map_rotation:
      *output = m_clearcoatMap.rotation;
      return true;
    case ShaderValue::material_clearcoat_factor:
      *output = m_clearcoatFactor;
      return true;
    case ShaderValue::material_clearcoatRoughness_map_rotation:
      *output = m_clearcoatRoughnessMap.rotation;
      return true;
    case ShaderValue::material_clearcoatRoughness_factor:
      *output = m_clearcoatRoughnessFactor;
      return true;
    case ShaderValue::material_clearcoatNormal_map_rotation:
      *output = m_clearcoatNormalMap.rotation;
      return true;
    case ShaderValue::material_clearcoatNormal_scale:
      *output = m_clearcoatNormalScale;
      return true;
    case ShaderValue::material_dispersion:
      *output = m_dispersion;
      return true;
    case ShaderValue::material_specular_map_rotation:
      *output = m_specularMap.rotation;
      return true;
    case ShaderValue::material_specular_factor:
      *output = m_specularFactor;
      return true;
    case ShaderValue::material_specularColor_map_rotation:
      *output = m_specularColorMap.rotation;
      return true;
    case ShaderValue::material_thickness_map_rotation:
      *output = m_thicknessMap.rotation;
      return true;
    case ShaderValue::material_thickness_factor:
      *output = m_thicknessFactor;
      return true;
    case ShaderValue::material_attenuationDistance:
      *output = m_attenuationDistance;
      return true;
    case ShaderValue::material_transmission_map_rotation:
      *output = m_transmissionMap.rotation;
      return true;
    case ShaderValue::material_transmission_factor:
      *output = m_transmissionFactor;
      return true;
    default:
      return false;
  }
  return false;
}

bool KRMaterial::getShaderValue(const KRCamera* camera, ShaderValue value, hydra::Vector2* output) const
{
  switch (value) {
    case ShaderValue::material_baseColor_map_scale:
      *output = m_baseColorMap.scale;
      return true;
    case ShaderValue::material_baseColor_map_offset:
      *output = m_baseColorMap.offset;
      return true;
    case ShaderValue::material_normal_map_scale:
      *output = m_normalMap.scale;
      return true;
    case ShaderValue::material_normal_map_offset:
      *output = m_normalMap.offset;
      return true;
    case ShaderValue::material_emissive_map_scale:
      *output = m_emissiveMap.scale;
      return true;
    case ShaderValue::material_emissive_map_offset:
      *output = m_emissiveMap.offset;
      return true;
    case ShaderValue::material_occlusion_map_scale:
      *output = m_occlusionMap.scale;
      return true;
    case ShaderValue::material_occlusion_map_offset:
      *output = m_occlusionMap.offset;
      return true;
    case ShaderValue::material_metalicRoughness_map_scale:
      *output = m_metalicRoughnessMap.scale;
      return true;
    case ShaderValue::material_metalicRoughness_map_offset:
      *output = m_metalicRoughnessMap.offset;
      return true;
    case ShaderValue::material_anisotropy_map_scale:
      *output = m_anisotropyMap.scale;
      return true;
    case ShaderValue::material_anisotropy_map_offset:
      *output = m_anisotropyMap.offset;
      return true;
    case ShaderValue::material_clearcoat_map_scale:
      *output = m_clearcoatMap.scale;
      return true;
    case ShaderValue::material_clearcoat_map_offset:
      *output = m_clearcoatMap.offset;
      return true;
    case ShaderValue::material_clearcoatRoughness_map_scale:
      *output = m_clearcoatRoughnessMap.scale;
      return true;
    case ShaderValue::material_clearcoatRoughness_map_offset:
      *output = m_clearcoatRoughnessMap.offset;
      return true;
    case ShaderValue::material_clearcoatNormal_map_scale:
      *output = m_clearcoatNormalMap.scale;
      return true;
    case ShaderValue::material_clearcoatNormal_map_offset:
      *output = m_clearcoatNormalMap.offset;
      return true;
    case ShaderValue::material_specular_map_scale:
      *output = m_specularMap.scale;
      return true;
    case ShaderValue::material_specular_map_offset:
      *output = m_specularMap.offset;
      return true;
    case ShaderValue::material_specularColor_map_scale:
      *output = m_specularColorMap.scale;
      return true;
    case ShaderValue::material_specularColor_map_offset:
      *output = m_specularColorMap.offset;
      return true;
    case ShaderValue::material_thickness_map_scale:
      *output = m_thicknessMap.scale;
      return true;
    case ShaderValue::material_thickness_map_offset:
      *output = m_thicknessMap.offset;
      return true;
    case ShaderValue::material_transmission_map_scale:
      *output = m_transmissionMap.scale;
      return true;
    case ShaderValue::material_transmission_map_offset:
      *output = m_transmissionMap.offset;
      return true;
    default:
      return false;
  }
  return false;
}

bool KRMaterial::getShaderValue(const KRCamera* camera, ShaderValue value, hydra::Vector3* output) const
{
  switch (value) {
    case ShaderValue::material_emissive_factor:
      *output = m_emissiveFactor;
      return true;
    case ShaderValue::material_specularColor_factor:
      *output = m_specularColorFactor;
      return true;
    case ShaderValue::material_attenuationColor:
      *output = m_attenuationColor;
      return true;
    default:
      return false;
  }
  return false;
}

bool KRMaterial::getShaderValue(const KRCamera* camera, ShaderValue value, hydra::Vector4* output) const
{
  switch (value) {
    case ShaderValue::material_baseColor_factor:
      *output = m_baseColorFactor;
      return true;
    default:
      return false;
  }
}

bool KRMaterial::getShaderValue(const KRCamera* camera, ShaderValue value, KRResourceBinding* output) const
{
  switch (value) {
    case ShaderValue::material_baseColor_map_texture:
      *output = m_baseColorMap.texture;
      return true;
    case ShaderValue::material_normal_map_texture:
      *output = m_normalMap.texture;
      return true;
    case ShaderValue::material_emissive_map_texture:
      *output = m_emissiveMap.texture;
      return true;
    case ShaderValue::material_anisotropy_map_texture:
      *output = m_anisotropyMap.texture;
      return true;
    case ShaderValue::material_occlusion_map_texture:
      *output = m_occlusionMap.texture;
      return true;
    case ShaderValue::material_metalicRoughness_map_texture:
      *output = m_metalicRoughnessMap.texture;
      return true;
    case ShaderValue::material_specular_map_texture:
      *output = m_specularMap.texture;
      return true;
    case ShaderValue::material_specularColor_map_texture:
      *output = m_specularColorMap.texture;
      return true;
    case ShaderValue::material_clearcoat_map_texture:
      *output = m_clearcoatMap.texture;
      return true;
    case ShaderValue::material_clearcoatRoughness_map_texture:
      *output = m_clearcoatRoughnessMap.texture;
      return true;
    case ShaderValue::material_clearcoatNormal_map_texture:
      *output = m_clearcoatNormalMap.texture;
      return true;
    case ShaderValue::material_thickness_map_texture:
      *output = m_thicknessMap.texture;
      return true;
    case ShaderValue::material_transmission_map_texture:
      *output = m_transmissionMap.texture;
      return true;
    default:
      return false;
  }
}

bool KRMaterial::getShaderValue(const KRCamera* camera, ShaderValue value, int64_t* output) const
{
  switch (value) {
    case ShaderValue::material_shadingModel:
      *output = m_shadingModel;
      return true;
    case ShaderValue::material_baseColor_map_texCoord:
      *output = m_baseColorMap.texCoord;
      return true;
    case ShaderValue::material_normal_map_texCoord:
      *output = m_normalMap.texCoord;
      return true;
    case ShaderValue::material_emissive_map_texCoord:
      *output = m_emissiveMap.texCoord;
      return true;
    case ShaderValue::material_occlusion_map_texCoord:
      *output = m_occlusionMap.texCoord;
      return true;
    case ShaderValue::material_metalicRoughness_map_texCoord:
      *output = m_metalicRoughnessMap.texCoord;
      return true;
    case ShaderValue::material_anisotropy_map_texCoord:
      *output = m_anisotropyMap.texCoord;
      return true;
    case ShaderValue::material_clearcoat_map_texCoord:
      *output = m_clearcoatMap.texCoord;
      return true;
    case ShaderValue::material_clearcoatRoughness_map_texCoord:
      *output = m_clearcoatRoughnessMap.texCoord;
      return true;
    case ShaderValue::material_clearcoatNormal_map_texCoord:
      *output = m_clearcoatNormalMap.texCoord;
      return true;
    case ShaderValue::material_specular_map_texCoord:
      *output = m_specularMap.texCoord;
      return true;
    case ShaderValue::material_specularColor_map_texCoord:
      *output = m_specularColorMap.texCoord;
      return true;
    case ShaderValue::material_thickness_map_texCoord:
      *output = m_thicknessMap.texCoord;
      return true;
    case ShaderValue::material_transmission_map_texCoord:
      *output = m_transmissionMap.texCoord;
      return true;
    default:
      return false;
  }
}

bool KRMaterial::getShaderValue(const KRCamera* camera, ShaderValue value, bool* output) const
{
  switch (value) {
    case ShaderValue::material_doubleSided:
      *output = m_doubleSided;
      return true;
    default:
      return false;
  }
}

bool KRMaterial::getImageBinding(const std::string& name, const KRTextureBinding** binding, KRSampler** sample) const
{
  // TODO - Need to implement sampler selection / generation system
  if (name == "baseColorTexture") {
    *binding = &m_baseColorMap.texture;
    *sample = getContext().getSamplerManager()->DEFAULT_WRAPPING_SAMPLER;
    return true;
  } else if (name == "normalTexture") {
    *binding = &m_normalMap.texture;
    *sample = getContext().getSamplerManager()->DEFAULT_WRAPPING_SAMPLER;
    return true;
  } else if (name == "emissiveTexture") {
    *binding = &m_emissiveMap.texture;
    *sample = getContext().getSamplerManager()->DEFAULT_WRAPPING_SAMPLER;
    return true;
  } else if (name == "occlusionTexture") {
    *binding = &m_occlusionMap.texture;
    *sample = getContext().getSamplerManager()->DEFAULT_WRAPPING_SAMPLER;
    return true;
  } else if (name == "metalicRoughnessTexture") {
    *binding = &m_metalicRoughnessMap.texture;
    *sample = getContext().getSamplerManager()->DEFAULT_WRAPPING_SAMPLER;
    return true;
  } else if (name == "anisotropyTexture") {
    *binding = &m_anisotropyMap.texture;
    *sample = getContext().getSamplerManager()->DEFAULT_WRAPPING_SAMPLER;
    return true;
  } else if (name == "clearcoatTexture") {
    *binding = &m_clearcoatMap.texture;
    *sample = getContext().getSamplerManager()->DEFAULT_WRAPPING_SAMPLER;
    return true;
  } else if (name == "clearcoatNormalTexture") {
    *binding = &m_clearcoatNormalMap.texture;
    *sample = getContext().getSamplerManager()->DEFAULT_WRAPPING_SAMPLER;
    return true;
  } else if (name == "specularTexture") {
    *binding = &m_specularMap.texture;
    *sample = getContext().getSamplerManager()->DEFAULT_WRAPPING_SAMPLER;
    return true;
  } else if (name == "specularColorTexture") {
    *binding = &m_specularColorMap.texture;
    *sample = getContext().getSamplerManager()->DEFAULT_WRAPPING_SAMPLER;
    return true;
  } else if (name == "thicknessTexture") {
    *binding = &m_thicknessMap.texture;
    *sample = getContext().getSamplerManager()->DEFAULT_WRAPPING_SAMPLER;
    return true;
  } else if (name == "transmissionTexture") {
    *binding = &m_transmissionMap.texture;
    *sample = getContext().getSamplerManager()->DEFAULT_WRAPPING_SAMPLER;
    return true;
  } else {
    return KRReflectedObject::getImageBinding(name, binding, sample);
  }
}