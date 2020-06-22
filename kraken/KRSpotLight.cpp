//
//  KRSpotLight.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-05.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KREngine-common.h"
#include "KRSpotLight.h"

/* static */
void KRSpotLight::InitNodeInfo(KrNodeInfo* nodeInfo)
{
  KRLight::InitNodeInfo(nodeInfo);
  nodeInfo->light.spot.inner_angle = M_PI_4;
  nodeInfo->light.spot.outer_angle = M_PI_2;
}

KRSpotLight::KRSpotLight(KRScene &scene, std::string name) : KRLight(scene, name)
{
  m_innerAngle = M_PI_4;
  m_outerAngle = M_PI_2;
}

KRSpotLight::~KRSpotLight()
{
    
}

std::string KRSpotLight::getElementName() {
    return "spot_light";
}

tinyxml2::XMLElement *KRSpotLight::saveXML( tinyxml2::XMLNode *parent)
{
    tinyxml2::XMLElement *e = KRLight::saveXML(parent);
    e->SetAttribute("inner_angle", m_innerAngle);
    e->SetAttribute("outer_angle", m_outerAngle);
    return e;
}

void KRSpotLight::loadXML(tinyxml2::XMLElement *e) {
    KRLight::loadXML(e);

    e->QueryFloatAttribute("inner_angle", &m_innerAngle);
    e->QueryFloatAttribute("outer_angle", &m_outerAngle);
}

float KRSpotLight::getInnerAngle() {
    return m_innerAngle;
}
float KRSpotLight::getOuterAngle() {
    return m_outerAngle;
}
void KRSpotLight::setInnerAngle(float innerAngle) {
    m_innerAngle = innerAngle;
}
void KRSpotLight::setOuterAngle(float outerAngle) {
    m_outerAngle = outerAngle;
}

AABB KRSpotLight::getBounds() {
    float influence_radius = m_decayStart - sqrt(m_intensity * 0.01f) / sqrt(KRLIGHT_MIN_INFLUENCE);
    if(influence_radius < m_flareOcclusionSize) {
        influence_radius = m_flareOcclusionSize;
    }
    return AABB::Create(Vector3::Create(-influence_radius), Vector3::Create(influence_radius), getModelMatrix());
}


