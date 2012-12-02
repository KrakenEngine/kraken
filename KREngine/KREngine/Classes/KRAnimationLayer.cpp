//
//  KRAnimationLayer.cpp
//  KREngine
//
//  Copyright 2012 Kearwood Gilbert. All rights reserved.
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

#include "KRAnimationLayer.h"

KRAnimationLayer::KRAnimationLayer(KRContext &context) : KRContextObject(context)
{
    m_name = "";
    m_blend_mode = KRENGINE_ANIMATION_BLEND_MODE_ADDITIVE;
    m_rotation_accumulation_mode = KRENGINE_ANIMATION_ROTATION_ACCUMULATION_BY_LAYER;
    m_scale_accumulation_mode = KRENGINE_ANIMATION_SCALE_ACCUMULATION_MULTIPLY;
}

KRAnimationLayer::~KRAnimationLayer()
{
    for(std::vector<KRAnimationAttribute *>::iterator itr = m_attributes.begin(); itr != m_attributes.end(); ++itr){
        delete (*itr);
    }
}

std::string KRAnimationLayer::getName() const
{
    return m_name;
}

void KRAnimationLayer::setName(const std::string &name)
{
    m_name = name;
}

tinyxml2::XMLElement *KRAnimationLayer::saveXML( tinyxml2::XMLNode *parent)
{
    tinyxml2::XMLDocument *doc = parent->GetDocument();
    tinyxml2::XMLElement *e = doc->NewElement("layer");
    tinyxml2::XMLNode *n = parent->InsertEndChild(e);
    e->SetAttribute("name", m_name.c_str());
    e->SetAttribute("weight", m_weight);
    
    switch(m_blend_mode) {
        case KRENGINE_ANIMATION_BLEND_MODE_ADDITIVE:
            e->SetAttribute("blend_mode", "additive");
            break;
        case KRENGINE_ANIMATION_BLEND_MODE_OVERRIDE:
            e->SetAttribute("blend_mode", "override");
            break;
        case KRENGINE_ANIMATION_BLEND_MODE_OVERRIDE_PASSTHROUGH:
            e->SetAttribute("blend_mode", "override_passthrough");
            break;
    }
    
    switch(m_rotation_accumulation_mode) {
        case KRENGINE_ANIMATION_ROTATION_ACCUMULATION_BY_CHANNEL:
            e->SetAttribute("rotation_accumulation_mode", "by_channel");
            break;
        case KRENGINE_ANIMATION_ROTATION_ACCUMULATION_BY_LAYER:
            e->SetAttribute("rotation_accumulation_mode", "by_layer");
            break;
    }
    
    switch(m_scale_accumulation_mode) {
        case KRENGINE_ANIMATION_SCALE_ACCUMULATION_ADDITIVE:
            e->SetAttribute("scale_accumulation_mode", "additive");
            break;
        case KRENGINE_ANIMATION_SCALE_ACCUMULATION_MULTIPLY:
            e->SetAttribute("scale_accumulation_mode", "multiply");
            break;
    }
    
    for(std::vector<KRAnimationAttribute *>::iterator itr = m_attributes.begin(); itr != m_attributes.end(); ++itr){
        (*itr)->saveXML(n);
    }
    
    return e;
}

void KRAnimationLayer::loadXML(tinyxml2::XMLElement *e)
{
    m_name = e->Attribute("name");
    if(e->QueryFloatAttribute("weight", &m_weight) != tinyxml2::XML_SUCCESS) {
        m_weight = 1.0f; // default
    }
    
    const char *szBlendMode = e->Attribute("blend_mode");
    if(strcmp(szBlendMode, "additive") == 0) {
        m_blend_mode = KRENGINE_ANIMATION_BLEND_MODE_ADDITIVE;
    } else if(strcmp(szBlendMode, "override") == 0) {
        m_blend_mode = KRENGINE_ANIMATION_BLEND_MODE_OVERRIDE;
    } else if(strcmp(szBlendMode, "override_passthrough") == 0) {
        m_blend_mode = KRENGINE_ANIMATION_BLEND_MODE_OVERRIDE_PASSTHROUGH;
    } else {
        m_blend_mode = KRENGINE_ANIMATION_BLEND_MODE_ADDITIVE; // default
    }
    
    const char *szRotationAccumulationMode = e->Attribute("rotation_accumulation_mode");
    if(strcmp(szRotationAccumulationMode, "by_channel") == 0) {
        m_rotation_accumulation_mode = KRENGINE_ANIMATION_ROTATION_ACCUMULATION_BY_CHANNEL;
    } else if(strcmp(szRotationAccumulationMode, "by_layer") == 0) {
        m_rotation_accumulation_mode = KRENGINE_ANIMATION_ROTATION_ACCUMULATION_BY_LAYER;
    } else {
        m_rotation_accumulation_mode = KRENGINE_ANIMATION_ROTATION_ACCUMULATION_BY_LAYER; // default
    }
    
    const char *szScaleAccumulationMode = e->Attribute("scale_accumulation_mode");
    if(strcmp(szScaleAccumulationMode, "additive") == 0) {
        m_scale_accumulation_mode = KRENGINE_ANIMATION_SCALE_ACCUMULATION_ADDITIVE;
    } else if(strcmp(szScaleAccumulationMode, "multiply") == 0) {
        m_scale_accumulation_mode = KRENGINE_ANIMATION_SCALE_ACCUMULATION_MULTIPLY;
    } else {
        m_scale_accumulation_mode = KRENGINE_ANIMATION_SCALE_ACCUMULATION_MULTIPLY; // default
    }
    
    for(tinyxml2::XMLElement *child_element=e->FirstChildElement(); child_element != NULL; child_element = child_element->NextSiblingElement()) {
        if(strcmp(child_element->Name(), "attribute") == 0) {
            KRAnimationAttribute *new_attribute = new KRAnimationAttribute(getContext());
            new_attribute->loadXML(child_element);
            m_attributes.push_back(new_attribute);
        }
    }
}

float KRAnimationLayer::getWeight() const
{
    return m_weight;
}

void KRAnimationLayer::setWeight(float weight)
{
    m_weight = weight;
}


KRAnimationLayer::blend_mode_t KRAnimationLayer::getBlendMode() const
{
    return m_blend_mode;
}

void KRAnimationLayer::setBlendMode(const KRAnimationLayer::blend_mode_t &blend_mode)
{
    m_blend_mode = blend_mode;
}

KRAnimationLayer::rotation_accumulation_mode_t KRAnimationLayer::getRotationAccumulationMode() const
{
    return m_rotation_accumulation_mode;
}

void KRAnimationLayer::setRotationAccumulationMode(const KRAnimationLayer::rotation_accumulation_mode_t &rotation_accumulation_mode)
{
    m_rotation_accumulation_mode = rotation_accumulation_mode;
}

KRAnimationLayer::scale_accumulation_mode_t KRAnimationLayer::getScaleAccumulationMode() const
{
    return m_scale_accumulation_mode;
}

void KRAnimationLayer::setScaleAccumulationMode(const KRAnimationLayer::scale_accumulation_mode_t &scale_accumulation_mode)
{
    m_scale_accumulation_mode = scale_accumulation_mode;
}

void KRAnimationLayer::addAttribute(KRAnimationAttribute *attribute)
{
    m_attributes.push_back(attribute);
}
