//
//  KRAnimationLayer.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-11-30.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KRAnimationLayer.h"

KRAnimationLayer::KRAnimationLayer(KRContext &context) : KRContextObject(context)
{
    m_name = "";
    m_blend_mode = KRENGINE_ANIMATION_BLEND_MODE_ADDITIVE;
    m_rotation_accumulation_mode = KRENGINE_ANIMATION_ROTATION_ACCUMULATION_BY_LAYER;
    m_scale_accumulation_mode = KRENGINE_ANIMATION_SCALE_ACCUMULATION_ADDITIVE;
}

KRAnimationLayer::~KRAnimationLayer()
{
    
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
    
    return e;
}

void KRAnimationLayer::loadXML(tinyxml2::XMLElement *e)
{
    m_name = e->Attribute("name");
    if(e->QueryFloatAttribute("weight", &m_weight) == tinyxml2::XML_SUCCESS) {
        m_weight /= 100.0f;
    } else {
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
        m_scale_accumulation_mode = KRENGINE_ANIMATION_SCALE_ACCUMULATION_ADDITIVE; // default
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

