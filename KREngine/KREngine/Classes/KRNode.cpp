//
//  KRNode.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-11.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include <iostream>

#import "KRNode.h"


KRNode::KRNode(std::string name)
{
    m_name = name;
    m_localScale = KRVector3(1.0f, 1.0f, 1.0f);
    m_localRotation = KRVector3(0.0f, 0.0f, 0.0f);
    m_localTranslation = KRVector3(0.0f, 0.0f, 0.0f);
}

KRNode::~KRNode() {
    for(std::vector<KRNode *>::iterator itr=m_childNodes.begin(); itr < m_childNodes.end(); ++itr) {
        delete *itr;
    }
    m_childNodes.clear();
}

void KRNode::addChild(KRNode *child) {
    m_childNodes.push_back(child);
}

tinyxml2::XMLElement *KRNode::saveXML(tinyxml2::XMLNode *parent) {
    tinyxml2::XMLDocument *doc = parent->GetDocument();
    tinyxml2::XMLElement *e = doc->NewElement(getElementName().c_str());
    tinyxml2::XMLNode *n = parent->InsertEndChild(e);
    e->SetAttribute("name", m_name.c_str());
    e->SetAttribute("translate_x", m_localTranslation.x);
    e->SetAttribute("translate_y", m_localTranslation.y);
    e->SetAttribute("translate_z", m_localTranslation.z);
    e->SetAttribute("scale_x", m_localScale.x);
    e->SetAttribute("scale_y", m_localScale.y);
    e->SetAttribute("scale_z", m_localScale.z);
    e->SetAttribute("rotate_x", m_localRotation.x);
    e->SetAttribute("rotate_y", m_localRotation.y);
    e->SetAttribute("rotate_z", m_localRotation.z);
    for(std::vector<KRNode *>::iterator itr=m_childNodes.begin(); itr < m_childNodes.end(); ++itr) {
        KRNode *child = (*itr);
        child->saveXML(n);
    }
    return e;
}

void KRNode::setLocalTranslation(const KRVector3 &v) {
    m_localTranslation = v;
}
void KRNode::setLocalScale(const KRVector3 &v) {
    m_localScale = v;
}
void KRNode::setLocalRotation(const KRVector3 &v) {
    m_localRotation = v;
}

const KRVector3 &KRNode::getLocalTranslation() {
    return m_localTranslation;
}
const KRVector3 &KRNode::getLocalScale() {
    return m_localScale;
}
const KRVector3 &KRNode::getLocalRotation() {
    return m_localRotation;
}

std::string KRNode::getElementName() {
    return "node";
}

