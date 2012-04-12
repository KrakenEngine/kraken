//
//  KRLight.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-05.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include <iostream>

#import "KRLight.h"

KRLight::KRLight(std::string name) : KRNode(name)
{
    m_intensity = 1.0f;
}

KRLight::~KRLight()
{

}

tinyxml2::XMLElement *KRLight::saveXML( tinyxml2::XMLNode *parent)
{
    tinyxml2::XMLElement *e = KRNode::saveXML(parent);
    e->SetAttribute("intensity", m_intensity);
    e->SetAttribute("color_r", m_color.x);
    e->SetAttribute("color_g", m_color.y);
    e->SetAttribute("color_b", m_color.z);
    e->SetAttribute("decay_start", m_decayStart);
    return e;
}

void KRLight::loadXML(tinyxml2::XMLElement *e) {
    KRNode::loadXML(e);
    float x,y,z;
    e->QueryFloatAttribute("color_r", &x);
    e->QueryFloatAttribute("color_g", &y);
    e->QueryFloatAttribute("color_b", &z);
    m_color = KRVector3(x,y,z);
    
    e->QueryFloatAttribute("intensity", &m_intensity);
    e->QueryFloatAttribute("decay_start", &m_decayStart);
}

void KRLight::setIntensity(float intensity) {
    m_intensity = intensity;
}
float KRLight::getIntensity() {
    return m_intensity;
}

const KRVector3 &KRLight::getColor() {
    return m_color;
}

void KRLight::setColor(const KRVector3 &color) {
    m_color = color;
}

void KRLight::setDecayStart(float decayStart) {
    m_decayStart = decayStart;
}

float KRLight::getDecayStart() {
    return m_decayStart;
}
