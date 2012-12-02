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


KRAnimationAttribute::KRAnimationAttribute(KRContext &context) : KRContextObject(context)
{
    
}

KRAnimationAttribute::~KRAnimationAttribute()
{
    
}

tinyxml2::XMLElement *KRAnimationAttribute::saveXML( tinyxml2::XMLNode *parent)
{
    tinyxml2::XMLDocument *doc = parent->GetDocument();
    tinyxml2::XMLElement *e = doc->NewElement("attribute");
    tinyxml2::XMLNode *n = parent->InsertEndChild(e);
    e->SetAttribute("curve", m_curve_name.c_str());
    e->SetAttribute("target", m_target_name.c_str());
    e->SetAttribute("attribute", m_target_attribute_name.c_str());
    return e;
}

void KRAnimationAttribute::loadXML(tinyxml2::XMLElement *e)
{
    m_curve_name = e->Attribute("curve");
    m_target_name = e->Attribute("target");
    m_target_attribute_name = e->Attribute("attribute");
}

std::string KRAnimationAttribute::getTargetName() const
{
    return m_target_name;
}

void KRAnimationAttribute::setTargetName(const std::string &target_name)
{
    m_target_name = target_name;
}

std::string KRAnimationAttribute::getTargetAttributeName() const
{
    return m_target_attribute_name;
}

void KRAnimationAttribute::setTargetAttributeName(const std::string &target_attribute_name)
{
    m_target_attribute_name = target_attribute_name;
}


std::string KRAnimationAttribute::getCurveName() const
{
    return m_curve_name;
}

void KRAnimationAttribute::setCurveName(const std::string &curve_name)
{
    m_curve_name = curve_name;
}


