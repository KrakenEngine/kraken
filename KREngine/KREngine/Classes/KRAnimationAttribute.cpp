//
//  KRAnimationAttribute.cpp
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

#include "KRAnimationAttribute.h"
#include "KRContext.h"
#include "KRAnimationManager.h"
#include "KRAnimationCurveManager.h"


KRAnimationAttribute::KRAnimationAttribute(KRContext &context) : KRContextObject(context)
{
    m_node_attribute = KRNode::KRENGINE_NODE_ATTRIBUTE_NONE;
    m_target = NULL;
    m_curve = NULL;
}

KRAnimationAttribute::~KRAnimationAttribute()
{
    
}

tinyxml2::XMLElement *KRAnimationAttribute::saveXML( tinyxml2::XMLNode *parent)
{
    tinyxml2::XMLDocument *doc = parent->GetDocument();
    tinyxml2::XMLElement *e = doc->NewElement("attribute");
    parent->InsertEndChild(e);
    e->SetAttribute("curve", m_curve_name.c_str());
    e->SetAttribute("target", m_target_name.c_str());
    const char *szAttribute = "none";
    switch(m_node_attribute) {
        case KRNode::KRENGINE_NODE_ATTRIBUTE_NONE:
            szAttribute = "none";
            break;
        case KRNode::KRENGINE_NODE_ATTRIBUTE_TRANSLATE_X:
            szAttribute = "translate_x";
            break;
        case KRNode::KRENGINE_NODE_ATTRIBUTE_TRANSLATE_Y:
            szAttribute = "translate_y";
            break;
        case KRNode::KRENGINE_NODE_ATTRIBUTE_TRANSLATE_Z:
            szAttribute = "translate_z";
            break;
        case KRNode::KRENGINE_NODE_ATTRIBUTE_SCALE_X:
            szAttribute = "scale_x";
            break;
        case KRNode::KRENGINE_NODE_ATTRIBUTE_SCALE_Y:
            szAttribute = "scale_y";
            break;
        case KRNode::KRENGINE_NODE_ATTRIBUTE_SCALE_Z:
            szAttribute = "scale_z";
            break;
        case KRNode::KRENGINE_NODE_ATTRIBUTE_ROTATE_X:
            szAttribute = "rotate_x";
            break;
        case KRNode::KRENGINE_NODE_ATTRIBUTE_ROTATE_Y:
            szAttribute = "rotate_y";
            break;
        case KRNode::KRENGINE_NODE_ATTRIBUTE_ROTATE_Z:
            szAttribute = "rotate_z";
            break;
    }
    
    e->SetAttribute("attribute", szAttribute);
    return e;
}

void KRAnimationAttribute::loadXML(tinyxml2::XMLElement *e)
{
    m_target = NULL;
    m_curve = NULL;
    m_curve_name = e->Attribute("curve");
    m_target_name = e->Attribute("target");
    m_node_attribute = KRNode::KRENGINE_NODE_ATTRIBUTE_NONE;
    
    const char *szAttribute = e->Attribute("attribute");
    if(strcmp(szAttribute, "none") == 0) {
        m_node_attribute = KRNode::KRENGINE_NODE_ATTRIBUTE_NONE;
    } else if(strcmp(szAttribute, "translate_x") == 0) {
        m_node_attribute = KRNode::KRENGINE_NODE_ATTRIBUTE_TRANSLATE_X;
    } else if(strcmp(szAttribute, "translate_y") == 0) {
        m_node_attribute = KRNode::KRENGINE_NODE_ATTRIBUTE_TRANSLATE_Y;
    } else if(strcmp(szAttribute, "translate_z") == 0) {
        m_node_attribute = KRNode::KRENGINE_NODE_ATTRIBUTE_TRANSLATE_Z;
    } else if(strcmp(szAttribute, "rotate_x") == 0) {
        m_node_attribute = KRNode::KRENGINE_NODE_ATTRIBUTE_ROTATE_X;
    } else if(strcmp(szAttribute, "rotate_y") == 0) {
        m_node_attribute = KRNode::KRENGINE_NODE_ATTRIBUTE_ROTATE_Y;
    } else if(strcmp(szAttribute, "rotate_z") == 0) {
        m_node_attribute = KRNode::KRENGINE_NODE_ATTRIBUTE_ROTATE_Z;
    } else if(strcmp(szAttribute, "scale_x") == 0) {
        m_node_attribute = KRNode::KRENGINE_NODE_ATTRIBUTE_SCALE_X;
    } else if(strcmp(szAttribute, "scale_y") == 0) {
        m_node_attribute = KRNode::KRENGINE_NODE_ATTRIBUTE_SCALE_Y;
    } else if(strcmp(szAttribute, "scale_z") == 0) {
        m_node_attribute = KRNode::KRENGINE_NODE_ATTRIBUTE_SCALE_Z;
    } else {
        m_node_attribute = KRNode::KRENGINE_NODE_ATTRIBUTE_NONE;
    }
}

KRNode::node_attribute_type KRAnimationAttribute::getTargetAttribute() const
{
    return m_node_attribute;
}

void KRAnimationAttribute::setTargetAttribute(KRNode::node_attribute_type target_attribute)
{
    m_node_attribute = target_attribute;
}

std::string KRAnimationAttribute::getTargetName() const
{
    return m_target_name;
}

void KRAnimationAttribute::setTargetName(const std::string &target_name)
{
    m_target_name = target_name;
    m_target = NULL;
}

std::string KRAnimationAttribute::getCurveName() const
{
    return m_curve_name;
}

void KRAnimationAttribute::setCurveName(const std::string &curve_name)
{
    m_curve_name = curve_name;
    m_curve = NULL;
}

KRNode *KRAnimationAttribute::getTarget()
{
    if(m_target == NULL) {
        m_target = getContext().getSceneManager()->getFirstScene()->getRootNode()->findChild(m_target_name); // FINDME, HACK! - This won't work with multiple scenes in a context; we should move the animations out of KRAnimationManager and attach them to the parent nodes of the animated KRNode's
    }
    return m_target;
}

KRAnimationCurve *KRAnimationAttribute::getCurve()
{
    if(m_curve == NULL) {
        m_curve = getContext().getAnimationCurveManager()->getAnimationCurve(m_curve_name.c_str());
    }
    return m_curve;
}


