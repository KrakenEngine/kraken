//
//  KRAnimationLayer.h
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

#include "KRContextObject.h"
#include "KREngine-common.h"
#include "KRAnimationAttribute.h"

namespace tinyxml2 {
class XMLNode;
class XMLAttribute;
}

class KRAnimationLayer : public KRContextObject
{
public:
  KRAnimationLayer(KRContext& context);
  ~KRAnimationLayer();

  tinyxml2::XMLElement* saveXML(tinyxml2::XMLNode* parent);
  void loadXML(tinyxml2::XMLElement* e);

  std::string getName() const;
  void setName(const std::string& name);

  float getWeight() const;
  void setWeight(float weight);

  typedef enum
  {
    KRENGINE_ANIMATION_BLEND_MODE_ADDITIVE,
    KRENGINE_ANIMATION_BLEND_MODE_OVERRIDE,
    KRENGINE_ANIMATION_BLEND_MODE_OVERRIDE_PASSTHROUGH
  } blend_mode_t;

  blend_mode_t getBlendMode() const;
  void setBlendMode(const blend_mode_t& blend_mode);

  typedef enum
  {
    KRENGINE_ANIMATION_ROTATION_ACCUMULATION_BY_LAYER,
    KRENGINE_ANIMATION_ROTATION_ACCUMULATION_BY_CHANNEL
  } rotation_accumulation_mode_t;

  rotation_accumulation_mode_t getRotationAccumulationMode() const;
  void setRotationAccumulationMode(const rotation_accumulation_mode_t& rotation_accumulation_mode);

  typedef enum
  {
    KRENGINE_ANIMATION_SCALE_ACCUMULATION_MULTIPLY,
    KRENGINE_ANIMATION_SCALE_ACCUMULATION_ADDITIVE
  } scale_accumulation_mode_t;

  scale_accumulation_mode_t getScaleAccumulationMode() const;
  void setScaleAccumulationMode(const scale_accumulation_mode_t& scale_accumulation_mode);

  void addAttribute(KRAnimationAttribute* attribute);
  std::vector<KRAnimationAttribute*>& getAttributes();

private:
  std::string m_name;
  float m_weight;
  blend_mode_t m_blend_mode;
  rotation_accumulation_mode_t m_rotation_accumulation_mode;
  scale_accumulation_mode_t m_scale_accumulation_mode;

  std::vector<KRAnimationAttribute*> m_attributes;
};
