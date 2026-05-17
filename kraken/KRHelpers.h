//
//  KRHelpers.h
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

#include "hydra.h"
#include <string>

#include "../3rdparty/tinyxml2/tinyxml2.h"

#include "simdjson.h"

#define KRALIGN(x) ((x + 3) & ~0x03)

float const PI = 3.141592653589793f;
float const D2R = PI * 2 / 360;

namespace kraken {
// XML Helpers
void setXMLAttribute(const std::string& base_name, ::tinyxml2::XMLElement* e, const hydra::Vector3& value, const hydra::Vector3& default_value);
void setXMLAttribute(const std::string& base_name, ::tinyxml2::XMLElement* e, const hydra::AABB& value, const hydra::AABB& default_value);
const hydra::Vector3 getXMLAttribute(const std::string& base_name, ::tinyxml2::XMLElement* e, const hydra::Vector3& default_value);
const hydra::AABB getXMLAttribute(const std::string& base_name, ::tinyxml2::XMLElement* e, const hydra::AABB& default_value);

// JSON Helpers
bool tryJsonRequired(simdjson::error_code error);
bool tryJson(simdjson::error_code error);

} // namespace kraken

namespace simdjson {

// 32-bit float simdjson deserialization helper
template <typename simdjson_value>
auto tag_invoke(deserialize_tag, simdjson_value &val, float& ret)
{
  double doubleRet = ret;
  auto error = val.get(doubleRet);
  ret = static_cast<float>(doubleRet);
  return error;
}

template <typename builder_type>
void tag_invoke(serialize_tag, builder_type& builder, const hydra::Vector2& vec)
{
  builder.start_array();
  builder.append(vec.x);
  builder.append_comma();
  builder.append(vec.y);
  builder.end_array();
}

template <typename simdjson_value>
auto tag_invoke(deserialize_tag, simdjson_value &val, hydra::Vector2& vec)
{
  ondemand::array components;
  auto error = val.get_array().get(components);
  if (error) {
    return error;
  }
  
  ondemand::array_iterator itr;
  if ((error = components.begin().get(itr)))
  {
    return error;
  }
  double v[2] = {0.f};
  for(int i=0; i < 2; i++) {
    if ((error = (*itr).get(v[i]))) {
      return error;
    }
    ++itr;
  }

  vec[0] = v[0];
  vec[1] = v[1];
  return simdjson::SUCCESS;
}

template <typename builder_type>
void tag_invoke(serialize_tag, builder_type& builder, const hydra::Vector3& vec)
{
  builder.start_array();
  builder.append(vec.x);
  builder.append_comma();
  builder.append(vec.y);
  builder.append_comma();
  builder.append(vec.z);
  builder.end_array();
}


template <typename simdjson_value>
auto tag_invoke(deserialize_tag, simdjson_value &val, hydra::Vector3& vec) {
  ondemand::array components;
  auto error = val.get_array().get(components);
  if (error) {
    return error;
  }
  
  ondemand::array_iterator itr;
  if ((error = components.begin().get(itr)))
  {
    return error;
  }
  double v[3] = {0.f};
  for(int i=0; i < 3; i++) {
    if ((error = (*itr).get(v[i]))) {
      return error;
    }
    ++itr;
  }

  vec[0] = v[0];
  vec[1] = v[1];
  vec[2] = v[2];
  return simdjson::SUCCESS;
}

template <typename builder_type>
void tag_invoke(serialize_tag, builder_type& builder, const hydra::Vector4& vec)
{
  builder.start_array();
  builder.append(vec.x);
  builder.append_comma();
  builder.append(vec.y);
  builder.append_comma();
  builder.append(vec.z);
  builder.append_comma();
  builder.append(vec.w);
  builder.end_array();
}

template <typename simdjson_value>
auto tag_invoke(deserialize_tag, simdjson_value &val, hydra::Vector4& vec)
{
  ondemand::array components;
  auto error = val.get_array().get(components);
  if (error) {
    return error;
  }
  
  ondemand::array_iterator itr;
  if ((error = components.begin().get(itr)))
  {
    return error;
  }
  double v[4] = {0.f};
  for(int i=0; i < 4; i++) {
    if ((error = (*itr).get(v[i]))) {
      return error;
    }
    ++itr;
  }

  vec[0] = v[0];
  vec[1] = v[1];
  vec[2] = v[2];
  vec[3] = v[3];
  return simdjson::SUCCESS;
}

} // namespace simdjson
