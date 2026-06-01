//
//  KRResource+gltf.cpp
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

#include "KRResource.h"
#include "bundle/KRBundle.h"
#include "scene/KRScene.h"

#include "mimir.h"

#include <format>

using namespace mimir;
using namespace hydra;

#include "simdjson.h"
using namespace simdjson;

KRBundle* LoadGltf(KRContext& context, simdjson::ondemand::object& jsonRoot, std::vector<Block>& buffers, const std::string& baseName)
{
  std::string_view version;
  if (!tryJsonRequired(jsonRoot["asset"]["version"].get(version))) {
    return nullptr;
  }
  
  std::string_view minVersion;
  if(tryJson(jsonRoot["asset"]["minVersion"].get(minVersion))) {
    // We have a minVersion field.
    // We currently support only version 2.0
    if (minVersion != "2.0") {
      // TODO - Report and handle error
      return nullptr;
    }
  } else {
    // We don't have a minversion field.
    // We will support any version 2.x.
    if (!version.starts_with("2.")) {
      // TODO - Report and handle error
      return nullptr;
    }
  }
  
  
  std::vector<KRTexture*> images;
  simdjson::ondemand::array jsonImages;
  if(tryJson(jsonRoot["images"].get_array().get(jsonImages))) {
    for (auto jsonImage : jsonImages) {
      KRTexture*& image = images.emplace_back();
      image = nullptr;
      std::string_view uri;
      int bufferView = -1;
      if (tryJson(jsonImage["uri"].get(uri))) {
        if (uri.starts_with("data:")) {
          KRContext::Log(KRContext::LOG_LEVEL_ERROR, "Kraken - GLTF: Data URI's not supported for images.");
          continue;
        }
        std::string uriStr;
        uriStr = uri;
        image = context.getTextureManager()->getTexture(util::GetFileBase(uriStr));
        if (image == nullptr) {
          KRContext::Log(KRContext::LOG_LEVEL_ERROR, "Kraken - GLTF: Image with URI not found: %s", uriStr.c_str());
          continue;
        }
      } else if (tryJson(jsonImage["bufferView"].get(bufferView))) {
        std::string_view mimeType;
        if(!tryJsonRequired(jsonImage["mimeType"].get(mimeType))) {
          continue;
        }
        if (mimeType == "image/png") {
          // TODO - Implement png loading from buffer view
          KRContext::Log(KRContext::LOG_LEVEL_ERROR, "Kraken - GLTF: Loading PNG Images from a buffer view is not yet supported.");
          continue;
        } else if (mimeType == "image/jpeg") {
          // TODO - Implement jpeg loading from buffer view
          KRContext::Log(KRContext::LOG_LEVEL_ERROR, "Kraken - GLTF: Loading JPEG Images from a buffer view is not yet supported.");
          continue;
        } else {
          std::string mimeTypeStr;
          mimeTypeStr = mimeType;
          KRContext::Log(KRContext::LOG_LEVEL_ERROR, "Kraken - GLTF: Image with mime type not supported: %s", mimeTypeStr.c_str());
          continue;
        }
      } else {
        KRContext::Log(KRContext::LOG_LEVEL_ERROR, "Kraken - GLTF: Image without uri or bufferView could not be imported.");
        continue;
      }
    }
  }
  
  struct SamplerInfo
  {
    KRMaterial::texture_wrap_type wrapS = KRMaterial::texture_wrap_type::KRMATERIAL_TEXTURE_REPEAT;
    KRMaterial::texture_wrap_type wrapT = KRMaterial::texture_wrap_type::KRMATERIAL_TEXTURE_REPEAT;
    KRMaterial::texture_mag_filter_type magFilter = KRMaterial::texture_mag_filter_type::KRMATERIAL_TEXTURE_MAG_LINEAR;
    KRMaterial::texture_min_filter_type minFilter = KRMaterial::texture_min_filter_type::KRMATERIAL_TEXTURE_MIN_LINEAR_MIPMAP_LINEAR;
  };
  std::vector<SamplerInfo> samplers;
  simdjson::ondemand::array jsonSamplers;
  if(tryJson(jsonRoot["samplers"].get_array().get(jsonSamplers))) {
    for (auto jsonSampler : jsonSamplers) {
      SamplerInfo& sampler = samplers.emplace_back();
      int jsonWrapS = 10497; // REPEAT
      int jsonWrapT = 10497; // REPEAT
      int jsonMinFilter = 9987; // LINEAR_MIPMAP_LINEAR
      int jsonMagFilter = 9729; // LINEAR
      tryJson(jsonSampler["wrapS"].get(jsonWrapS));
      tryJson(jsonSampler["wrapT"].get(jsonWrapT));
      tryJson(jsonSampler["minFilter"].get(jsonMinFilter));
      tryJson(jsonSampler["magFilter"].get(jsonMagFilter));
      
      switch(jsonWrapS)
      {
        case 33071: // CLAMP_TO_EDGE
          sampler.wrapS = KRMaterial::texture_wrap_type::KRMATERIAL_TEXTURE_CLAMP;
          break;
        case 10497: // REPEAT
          sampler.wrapS = KRMaterial::texture_wrap_type::KRMATERIAL_TEXTURE_REPEAT;
          break;
        case 33648: // MIRRORED_REPEAT
          sampler.wrapS = KRMaterial::texture_wrap_type::KRMATERIAL_TEXTURE_MIRROR_REPEAT;
          break;
        default:
          KRContext::Log(KRContext::LOG_LEVEL_ERROR, "Kraken - GLTF: Sampler with unknown sampler wrap: %i", jsonWrapS);
          break;
      }
      
      switch(jsonWrapT)
      {
        case 33071: // CLAMP_TO_EDGE
          sampler.wrapT = KRMaterial::texture_wrap_type::KRMATERIAL_TEXTURE_CLAMP;
          break;
        case 10497: // REPEAT
          sampler.wrapT = KRMaterial::texture_wrap_type::KRMATERIAL_TEXTURE_REPEAT;
          break;
        case 33648: // MIRRORED_REPEAT
          sampler.wrapT = KRMaterial::texture_wrap_type::KRMATERIAL_TEXTURE_MIRROR_REPEAT;
          break;
        default:
          KRContext::Log(KRContext::LOG_LEVEL_ERROR, "Kraken - GLTF: Sampler with unknown sampler wrap: %i", jsonWrapT);
          break;
      }
      
      switch(jsonMinFilter)
      {
        case 9728: // NEAREST
          sampler.minFilter = KRMaterial::texture_min_filter_type::KRMATERIAL_TEXTURE_MIN_NEAREST;
          break;
        case 9729: // LINEAR
          sampler.minFilter = KRMaterial::texture_min_filter_type::KRMATERIAL_TEXTURE_MIN_LINEAR;
          break;
        case 9984: // NEAREST_MIPMAP_NEAREST
          sampler.minFilter = KRMaterial::texture_min_filter_type::KRMATERIAL_TEXTURE_MIN_NEAREST_MIPMAP_NEAREST;
          break;
        case 9985: // LINEAR_MIPMAP_NEAREST
          sampler.minFilter = KRMaterial::texture_min_filter_type::KRMATERIAL_TEXTURE_MIN_LINEAR_MIPMAP_NEAREST;
          break;
        case 9986: // NEAREST_MIPMAP_LINEAR
          sampler.minFilter = KRMaterial::texture_min_filter_type::KRMATERIAL_TEXTURE_MIN_NEAREST_MIPMAP_LINEAR;
          break;
        case 9987: // LINEAR_MIPMAP_LINEAR
          sampler.minFilter = KRMaterial::texture_min_filter_type::KRMATERIAL_TEXTURE_MIN_LINEAR_MIPMAP_LINEAR;
          break;
        default:
          KRContext::Log(KRContext::LOG_LEVEL_ERROR, "Kraken - GLTF: Sampler with unknown minFilter: %i", jsonMinFilter);
          break;
      }
      
      switch(jsonMagFilter)
      {
        case 9728: // NEAREST
          sampler.magFilter = KRMaterial::texture_mag_filter_type::KRMATERIAL_TEXTURE_MAG_NEAREST;
          break;
        case 9729: // LINEAR
          sampler.magFilter = KRMaterial::texture_mag_filter_type::KRMATERIAL_TEXTURE_MAG_LINEAR;
          break;
        default:
          KRContext::Log(KRContext::LOG_LEVEL_ERROR, "Kraken - GLTF: Sampler with unknown magFilter: %i", jsonMagFilter);
          break;
      }
    }
  }
  
  struct TextureInfo
  {
    std::string name;
    KRTexture* texture = nullptr;
    int imageIndex = -1;
    hydra::Vector2 scale{ 1.f, 1.f };
    hydra::Vector2 offset{ 0.f, 0.f };
    float rotation{ 0.f };
    
    SamplerInfo sampler;
  };
  std::vector<TextureInfo> textures;
  simdjson::ondemand::array jsonTextures;
  if(tryJson(jsonRoot["textures"].get_array().get(jsonTextures)))
  {
    int textureIndex = 0;
    for (auto jsonTexture : jsonTextures) {
      TextureInfo& texture = textures.emplace_back();
      std::string textureName;
      std::string_view textureNameVal;
      if (tryJson(jsonTexture["name"].get(textureNameVal))) {
        texture.name = textureNameVal;
      } else {
        // Name not found in JSON. Generate a fall-back name.
        texture.name = std::format("{}_texture_{}", baseName, textureIndex);
      }
      
      int samplerIndex = -1;
      if (tryJson(jsonTexture["sampler"].get(samplerIndex))) {
        KRContext::Log(KRContext::LOG_LEVEL_WARNING, "Kraken - GLTF: Sampler options not supported for texture: %s", texture.name.c_str());
        if (samplerIndex < 0 || samplerIndex >= samplers.size()) {
          KRContext::Log(KRContext::LOG_LEVEL_ERROR, "Kraken - GLTF: Could not load texture with a sampler index that is out of range: %s", texture.name.c_str());
          continue;
        }
        texture.sampler = samplers[samplerIndex];
      }
      int imageIndex = -1;
      if (!tryJson(jsonTexture["source"].get(imageIndex))) {
        KRContext::Log(KRContext::LOG_LEVEL_ERROR, "Kraken - GLTF: Could not load texture without a source attribute: %s", texture.name.c_str());
        continue;
      }
      if (imageIndex < 0 || imageIndex >= images.size()) {
        KRContext::Log(KRContext::LOG_LEVEL_ERROR, "Kraken - GLTF: Could not load texture with a source index that is out of range: %s", texture.name.c_str());
        continue;
      }
      texture.texture = images[imageIndex];
      simdjson::ondemand::object extensions;
      if(tryJson(jsonTexture["extensions"].get(extensions))) {
        simdjson::ondemand::object textureTransform;
        if(tryJson(extensions["KHR_texture_transform"].get(textureTransform))) {
          tryJson(textureTransform["offset"].get(texture.offset));
          tryJson(textureTransform["rotation"].get(texture.rotation));
          tryJson(textureTransform["scale"].get(texture.scale));
        }
      }
      textureIndex++;
    }
  }

  KRBundle* bundle = new KRBundle(context, baseName);

  std::vector<KRMaterial*> materials;
  simdjson::ondemand::array jsonMaterials;
  if (tryJson(jsonRoot["materials"].get_array().get(jsonMaterials))) {
    int materialIndex = 0;
    for (auto jsonMaybeMaterial : jsonMaterials) {
      KRMaterial*& new_material = materials.emplace_back();
      new_material = nullptr;
      simdjson::ondemand::object jsonMaterial;
      if(!tryJsonRequired(jsonMaybeMaterial.get(jsonMaterial))) {
        continue;
      }
      std::string materialName;
      std::string_view materialNameVal;
      if (tryJson(jsonMaterial["name"].get(materialNameVal))) {
        materialName = materialNameVal;
      } else {
        // Name not found in JSON. Generate a fall-back name.
        materialName = std::format("{}_material_{}", baseName, materialIndex);
      }
      
      auto parseTextureInfoAttributes = [&materialName, &textures](simdjson::ondemand::object jsonTextureInfo, KRMaterial::TextureMap& textureMap)
      {
        tryJson(jsonTextureInfo["texCoord"].get(textureMap.texCoord));
        int textureIndex = -1;
        if(tryJson(jsonTextureInfo["index"].get(textureIndex))) {
          if (textureIndex < 0 || textureIndex >= textures.size()) {
            KRContext::Log(KRContext::LOG_LEVEL_ERROR, "Kraken - GLTF: Material textureInfo with index out of bounds: %s", materialName.c_str());
          } else {
            const TextureInfo& texture = textures[textureIndex];
            textureMap.scale = texture.scale;
            textureMap.offset = texture.offset;
            textureMap.rotation = texture.rotation;
            textureMap.wrapS = texture.sampler.wrapS;
            textureMap.wrapT = texture.sampler.wrapT;
            textureMap.minFilter = texture.sampler.minFilter;
            textureMap.magFilter = texture.sampler.magFilter;
            textureMap.texture.set(texture.texture);
          }
        }
      };
      
      auto parseTextureInfo = [parseTextureInfoAttributes, &materialName, &textures](simdjson::ondemand::object jsonObj, const char* nodeName, KRMaterial::TextureMap& textureMap)
      {
        simdjson::ondemand::object jsonTextureInfo;
        if(tryJson(jsonObj[nodeName].get(jsonTextureInfo))) {
          parseTextureInfoAttributes(jsonTextureInfo, textureMap);
        }
      };
      
      auto parseNormalTextureInfo = [parseTextureInfoAttributes, &materialName, &textures](simdjson::ondemand::object jsonObj, const char* nodeName, KRMaterial::TextureMap& textureMap, float* normalScale)
      {
        simdjson::ondemand::object jsonTextureInfo;
        if(tryJson(jsonObj[nodeName].get(jsonTextureInfo))) {
          parseTextureInfoAttributes(jsonTextureInfo, textureMap);
          tryJson(jsonTextureInfo["scale"].get(*normalScale));
        }
      };
      
      new_material = new KRMaterial(context, std::string(materialName).c_str());
      simdjson::ondemand::object pbrMetallicRoughnessObj;
      if(tryJson(jsonMaterial["pbrMetallicRoughness"].get(pbrMetallicRoughnessObj))) {
        tryJson(pbrMetallicRoughnessObj["baseColorFactor"].get(new_material->m_baseColorFactor));
        parseTextureInfo(pbrMetallicRoughnessObj, "baseColorTexture", new_material->m_baseColorMap);
        tryJson(pbrMetallicRoughnessObj["metallicFactor"].get(new_material->m_metalicFactor));
        tryJson(pbrMetallicRoughnessObj["roughnessFactor"].get(new_material->m_roughnessFactor));
        parseTextureInfo(pbrMetallicRoughnessObj, "metallicRoughnessTexture", new_material->m_metalicRoughnessMap);
      }
      parseNormalTextureInfo(jsonMaterial, "normalTexture", new_material->m_normalMap, &new_material->m_normalScale);
      simdjson::ondemand::object occlusionTextureInfo;
      if(tryJson(jsonMaterial["occlusionTexture"].get(occlusionTextureInfo))) {
        parseTextureInfoAttributes(occlusionTextureInfo, new_material->m_occlusionMap);
        tryJson(occlusionTextureInfo["strength"].get(new_material->m_occlusionStrength));
      }
      parseTextureInfo(jsonMaterial, "emissiveTexture", new_material->m_emissiveMap);
      tryJson(jsonMaterial["emissiveFactor"].get(new_material->m_emissiveFactor));
      new_material->moveToBundle(bundle);
      
      std::string_view alphaMode;
      if(tryJson(jsonMaterial["alphaMode"].get(alphaMode))) {
        if (alphaMode.compare("OPAQUE") == 0) {
          new_material->m_alphaMode = KRMaterial::KRMATERIAL_ALPHA_MODE_OPAQUE;
        } else if (alphaMode.compare("BLEND") == 0) {
          new_material->m_alphaMode = KRMaterial::KRMATERIAL_ALPHA_MODE_BLEND;
        } else if (alphaMode.compare("MASK") == 0) {
          new_material->m_alphaMode = KRMaterial::KRMATERIAL_ALPHA_MODE_TEST;
        } else {
          KRContext::Log(KRContext::LOG_LEVEL_ERROR, "Kraken - GLTF: Material with unknown alphaMode: %s", materialName.c_str());
        }
      }
      tryJson(jsonMaterial["alphaCutoff"].get(new_material->m_alphaCutoff));
      tryJson(jsonMaterial["doubleSided"].get(new_material->m_doubleSided));
      
      simdjson::ondemand::object extensions;
      if(tryJson(jsonMaterial["extensions"].get(extensions))) {
        tryJson(extensions["KHR_materials_ior"]["ior"].get(new_material->m_ior));
        tryJson(extensions["KHR_materials_dispersion"]["dispersino"].get(new_material->m_dispersion));
        simdjson::ondemand::object extensions_KHR_materials_unlit;
        if(tryJson(extensions["KHR_materials_unlit"].get(extensions_KHR_materials_unlit))) {
          // The presence of the KHR_materials_unlit node is all that is needed to enable the unlit shading model
          new_material->m_shadingModel = KRMaterial::KRMATERIAL_SHADING_MODEL_UNLIT;
        } else {
          new_material->m_shadingModel = KRMaterial::KRMATERIAL_SHADING_MODEL_PBR;
        }
        simdjson::ondemand::object extensions_KHR_materials_anisotropy;
        if(tryJson(extensions["KHR_materials_anisotropy"].get(extensions_KHR_materials_anisotropy))) {
          parseTextureInfo(extensions_KHR_materials_anisotropy, "anisotropyTexture", new_material->m_anisotropyMap);
          tryJson(extensions_KHR_materials_anisotropy["anisotropyStrength"].get(new_material->m_anisotropyStrength));
          tryJson(extensions_KHR_materials_anisotropy["anisotropyRotation"].get(new_material->m_anisotropyRotation));
        }
        
        simdjson::ondemand::object extensions_KHR_materials_clearcoat;
        if(tryJson(extensions["KHR_materials_clearcoat"].get(extensions_KHR_materials_clearcoat))) {
          tryJson(extensions_KHR_materials_clearcoat["clearcoatFactor"].get(new_material->m_clearcoatFactor));
          parseTextureInfo(extensions_KHR_materials_clearcoat, "clearcoatTexture", new_material->m_clearcoatMap);
          tryJson(extensions_KHR_materials_clearcoat["clearcoatRoughnessFactor"].get(new_material->m_clearcoatRoughnessFactor));
          parseTextureInfo(extensions_KHR_materials_clearcoat, "clearcoatRoughnessTexture", new_material->m_clearcoatRoughnessMap);
          parseNormalTextureInfo(extensions_KHR_materials_clearcoat, "clearcoatNormalTexture", new_material->m_clearcoatNormalMap, &new_material->m_clearcoatNormalScale);
        }
        
        simdjson::ondemand::object extensions_KHR_materials_specular;
        if(tryJson(extensions["KHR_materials_specular"].get(extensions_KHR_materials_specular))) {
          tryJson(extensions_KHR_materials_specular["specularFactor"].get(new_material->m_specularFactor));
          parseTextureInfo(extensions_KHR_materials_specular, "specularTexture", new_material->m_specularMap);
          tryJson(extensions_KHR_materials_specular["specularColorFactor"].get(new_material->m_specularColorFactor));
          parseTextureInfo(extensions_KHR_materials_specular, "specularColorTexture", new_material->m_specularColorMap);
        }
        
        simdjson::ondemand::object extensions_KHR_materials_volume;
        if(tryJson(extensions["KHR_materials_volume"].get(extensions_KHR_materials_volume))) {
          tryJson(extensions_KHR_materials_volume["thicknessFactor"].get(new_material->m_thicknessFactor));
          parseTextureInfo(extensions_KHR_materials_volume, "thicknessTexture", new_material->m_thicknessMap);
          tryJson(extensions_KHR_materials_volume["attenuationDistance"].get(new_material->m_attenuationDistance));
          tryJson(extensions_KHR_materials_volume["attenuationColor"].get(new_material->m_attenuationColor));
        }
        
        simdjson::ondemand::object extensions_KHR_materials_transmission;
        if(tryJson(extensions["KHR_materials_transmission"].get(extensions_KHR_materials_transmission))) {
          tryJson(extensions_KHR_materials_transmission["transmissionFactor"].get(new_material->m_transmissionFactor));
          parseTextureInfo(extensions_KHR_materials_transmission, "transmissionTexture", new_material->m_transmissionMap);
        }
        
        simdjson::ondemand::object extensions_KHR_materials_emissive_strength;
        if(tryJson(extensions["KHR_materials_emissive_strength"].get(extensions_KHR_materials_emissive_strength))) {
          float strength = 1.f;
          tryJson(extensions_KHR_materials_emissive_strength["emissiveStrength"].get(strength));
          new_material->m_emissiveFactor *= strength;
        }
      }
      materialIndex++;
    }
  }

  KRScene* pScene = new KRScene(context, baseName + "_scene");

  context.getSceneManager()->add(pScene);
  KrResult result = pScene->moveToBundle(bundle);
  // TODO - Validate result
  bundle->append(*pScene);

  
  return bundle;
}

KRBundle* KRResource::LoadGltf(KRContext& context, const std::string& path)
{
  std::string filePath = util::GetFilePath(path);
  std::string fileBase = util::GetFileBase(path);
  std::string binFilePath = filePath + fileBase + ".bin";

  Block jsonData;
  Block binData;
  if (!jsonData.load(path)) {
    return nullptr;
  }

  simdjson::ondemand::parser parser;
  simdjson::ondemand::document doc;
  
  jsonData.expand(SIMDJSON_PADDING);
  jsonData.lock();
  auto error = parser.iterate((const char*)jsonData.getStart(), jsonData.getSize()).get(doc);
  jsonData.unlock();

  if (error) {
    // TODO - Report and handle error
    return nullptr;
  }
  
  ondemand::object jsonRoot;
  error = doc.get_object().get(jsonRoot);
  if (error) {
    // TODO - Report and handle error
    return nullptr;
  }

  simdjson::ondemand::array jsonBuffers;
  error = jsonRoot["buffers"].get_array().get(jsonBuffers);
  if (error) {
    // TODO - Report and handle error
    return nullptr;
  }

  std::vector<Block> buffers;
  for (auto jsonBuffer : jsonBuffers) {
    std::string_view bufferUri;
    error = jsonBuffer["uri"].get_string().get(bufferUri);
    if (error) {
      // TODO - Report and handle error
      return nullptr;
    }
    Block& block = buffers.emplace_back();
    if (!block.load(std::string(bufferUri))) {
      // TODO - Report and handle error
      return nullptr;
    }
  }

  return ::LoadGltf(context, jsonRoot, buffers, fileBase);
}
