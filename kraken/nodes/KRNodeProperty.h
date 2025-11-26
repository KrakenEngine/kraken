//
//  KRNodeProperty.h
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

#pragma once

#include "resources/KRResourceBinding.h"

template <typename T, class config>
class KRNodeProperty
{
public:
  static constexpr decltype(config::defaultVal) defaultVal = config::defaultVal;
  static constexpr const char* name = config::name;
  KRNodeProperty()
    : val(config::defaultVal)
  {
  }

  KRNodeProperty(T& val)
    : val(val)
  {
  }

  KRNodeProperty& operator=(const T& v)
  {
    val = v;
    return *this;
  }

  operator const T& () const
  {
    return val;
  }

  void save(tinyxml2::XMLElement* element) const
  {
    save(element, config::name);
  }

  void save(tinyxml2::XMLElement* element, const char* attributeName) const
  {
    if constexpr (std::is_same<T, bool>::value) {
      element->SetAttribute(attributeName, val ? "true" : "false");
    } else if constexpr (std::is_same<T, hydra::Vector3>::value) {
      kraken::setXMLAttribute(attributeName, element, val, config::defaultVal);
    } else if constexpr (std::is_same<T, hydra::AABB>::value) {
      kraken::setXMLAttribute(attributeName, element, val, config::defaultVal);
    } else if constexpr (std::is_same<T, std::string>::value) {
      element->SetAttribute(attributeName, val.c_str());
    } else if constexpr (std::is_base_of<KRResourceBinding, T>::value) {
      element->SetAttribute(attributeName, val.getName().c_str());
    } else {
      element->SetAttribute(attributeName, val);
    }
  }

  void load(tinyxml2::XMLElement* element)
  {
    load(element, config::name);
  }

  void load(tinyxml2::XMLElement* element, const char* attributeName)
  {
    if constexpr (std::is_same<T, int>::value) {
      if (element->QueryIntAttribute(attributeName, &val) != tinyxml2::XML_SUCCESS) {
        val = config::defaultVal;
      }
    } else if constexpr (std::is_same<T, unsigned int>::value) {
      if (element->QueryUnsignedAttribute(attributeName, &val) != tinyxml2::XML_SUCCESS) {
        val = config::defaultVal;
      }
    } else if constexpr (std::is_same<T, float>::value) {
      if (element->QueryFloatAttribute(attributeName, &val) != tinyxml2::XML_SUCCESS) {
        val = config::defaultVal;
      }
    } else if constexpr (std::is_same<T, bool>::value) {
      if (element->QueryBoolAttribute(attributeName, &val) != tinyxml2::XML_SUCCESS) {
        val = config::defaultVal;
      }
    } else if constexpr (std::is_same<T, hydra::Vector3>::value) {
      val = kraken::getXMLAttribute(attributeName, element, config::defaultVal);
    } else if constexpr (std::is_same<T, hydra::AABB>::value) {
      val = kraken::getXMLAttribute(attributeName, element, config::defaultVal);
    } else if constexpr (std::is_same<T, std::string>::value) {
      const char* name = element->Attribute(attributeName);
      if (name) {
        val = name;
      } else {
        val = config::defaultVal;
      }
    } else if constexpr (std::is_base_of<KRResourceBinding, T>::value) {
      const char* name = element->Attribute(attributeName);
      if (name) {
        val.set(name);
      } else {
        val.clear();
      }
    } else {
      static_assert(false, "Typename not implemented.");
    }
  }

  T val;
};
