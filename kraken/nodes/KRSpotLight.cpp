//
//  KRSpotLight.cpp
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
#include "KRSpotLight.h"

using namespace hydra;

/* static */
void KRSpotLight::InitNodeInfo(KrNodeInfo* nodeInfo)
{
  KRLight::InitNodeInfo(nodeInfo);
  nodeInfo->light.spot.inner_angle = M_PI_4;
  nodeInfo->light.spot.outer_angle = M_PI_2;
}

KRSpotLight::KRSpotLight(KRScene& scene, std::string name) : KRLight(scene, name)
{
  m_innerAngle = M_PI_4;
  m_outerAngle = M_PI_2;
}

KRSpotLight::~KRSpotLight()
{

}

std::string KRSpotLight::getElementName()
{
  return "spot_light";
}

tinyxml2::XMLElement* KRSpotLight::saveXML(tinyxml2::XMLNode* parent)
{
  tinyxml2::XMLElement* e = KRLight::saveXML(parent);
  e->SetAttribute("inner_angle", m_innerAngle);
  e->SetAttribute("outer_angle", m_outerAngle);
  return e;
}

void KRSpotLight::loadXML(tinyxml2::XMLElement* e)
{
  KRLight::loadXML(e);

  e->QueryFloatAttribute("inner_angle", &m_innerAngle);
  e->QueryFloatAttribute("outer_angle", &m_outerAngle);
}

float KRSpotLight::getInnerAngle()
{
  return m_innerAngle;
}
float KRSpotLight::getOuterAngle()
{
  return m_outerAngle;
}
void KRSpotLight::setInnerAngle(float innerAngle)
{
  m_innerAngle = innerAngle;
}
void KRSpotLight::setOuterAngle(float outerAngle)
{
  m_outerAngle = outerAngle;
}

AABB KRSpotLight::getBounds()
{
  float influence_radius = m_decayStart - sqrt(m_intensity * 0.01f) / sqrt(KRLIGHT_MIN_INFLUENCE);
  if (influence_radius < m_flareOcclusionSize) {
    influence_radius = m_flareOcclusionSize;
  }
  return AABB::Create(Vector3::Create(-influence_radius), Vector3::Create(influence_radius), getModelMatrix());
}


