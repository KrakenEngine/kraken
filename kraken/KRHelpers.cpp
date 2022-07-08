//
//  KRHelpers.cpp
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

#include "KREngine-common.h"

#include "KRHelpers.h"

namespace kraken {

void setXMLAttribute(const std::string &base_name, tinyxml2::XMLElement *e, const Vector3 &value, const Vector3 &default_value)
{
  // TODO - Increase number of digits after the decimal in floating point format (6 -> 12?)
  // FINDME, TODO - This needs optimization...
  if (value != default_value) {
    e->SetAttribute((base_name + "_x").c_str(), value.x);
    e->SetAttribute((base_name + "_y").c_str(), value.y);
    e->SetAttribute((base_name + "_z").c_str(), value.z);
  }
}

const Vector3 getXMLAttribute(const std::string &base_name, tinyxml2::XMLElement *e, const Vector3 &default_value)
{
  Vector3 value;
  if (e->QueryFloatAttribute((base_name + "_x").c_str(), &value.x) == tinyxml2::XML_SUCCESS
    && e->QueryFloatAttribute((base_name + "_y").c_str(), &value.y) == tinyxml2::XML_SUCCESS
    && e->QueryFloatAttribute((base_name + "_z").c_str(), &value.z) == tinyxml2::XML_SUCCESS) {
    return value;
  } else {
    return default_value;
  }
}

} // namespace kraken
