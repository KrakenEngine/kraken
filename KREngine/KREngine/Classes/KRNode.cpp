//
//  KRNode.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-11.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include <iostream>

#import "KRNode.h"
#import "KRPointLight.h"
#import "KRSpotLight.h"
#import "KRDirectionalLight.h"


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

void KRNode::loadXML(tinyxml2::XMLElement *e) {
    m_name = e->Attribute("name");
    float x,y,z;
    e->QueryFloatAttribute("translate_x", &x);
    e->QueryFloatAttribute("translate_y", &y);
    e->QueryFloatAttribute("translate_z", &z);
    m_localTranslation = KRVector3(x,y,z);
    
    e->QueryFloatAttribute("scale_x", &x);
    e->QueryFloatAttribute("scale_y", &y);
    e->QueryFloatAttribute("scale_z", &z);
    m_localScale = KRVector3(x,y,z);
    
    e->QueryFloatAttribute("rotate_x", &x);
    e->QueryFloatAttribute("rotate_y", &y);
    e->QueryFloatAttribute("rotate_z", &z);
    m_localRotation = KRVector3(x,y,z);
    
    for(tinyxml2::XMLElement *child_element=e->FirstChildElement(); child_element != NULL; child_element = child_element->NextSiblingElement()) {
        KRNode *child_node = KRNode::LoadXML(child_element);
        if(child_node) {
            addChild(child_node);
        }
    }
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

KRNode *KRNode::LoadXML(tinyxml2::XMLElement *e) {
    KRNode *new_node = NULL;
    const char *szElementName = e->Name();
    const char *szName = e->Attribute("name");
    if(strcmp(szElementName, "node") == 0) {
        new_node = new KRNode(szName);
    } else if(strcmp(szElementName, "point_light") == 0) {
        new_node = new KRPointLight(szName);
    } else if(strcmp(szElementName, "directional_light") == 0) {
        new_node = new KRDirectionalLight(szName);
    } else if(strcmp(szElementName, "spot_light") == 0) {
        new_node = new KRSpotLight(szName);
    } else if(strcmp(szElementName, "mesh") == 0) {
        
    }
    
    if(new_node) {
        new_node->loadXML(e);
    }
    
    return new_node;
}

