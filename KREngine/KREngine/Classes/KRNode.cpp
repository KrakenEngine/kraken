//
//  KRNode.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-11.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include <iostream>
#include <assert.h>
#include <limits>

#import "KRNode.h"
#import "KRPointLight.h"
#import "KRSpotLight.h"
#import "KRDirectionalLight.h"
#import "KRInstance.h"
#import "KRAABB.h"


KRNode::KRNode(KRScene &scene, std::string name) : KRContextObject(scene.getContext())
{
    m_name = name;
    m_localScale = KRVector3::One();
    m_localRotation = KRVector3::Zero();
    m_localTranslation = KRVector3::Zero();
    m_parentNode = NULL;
    m_pScene = &scene;
    getScene().notify_sceneGraphCreate(this);
}

KRNode::~KRNode() {
    getScene().notify_sceneGraphDelete(this);
    for(std::vector<KRNode *>::iterator itr=m_childNodes.begin(); itr < m_childNodes.end(); ++itr) {
        delete *itr;
    }
    m_childNodes.clear();
}

void KRNode::addChild(KRNode *child) {
    assert(child->m_parentNode == NULL);
    child->m_parentNode = this;
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
        KRNode *child_node = KRNode::LoadXML(getScene(), child_element);
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

const KRVector3 &KRNode::getWorldTranslation() {
    return m_localTranslation;
}
const KRVector3 &KRNode::getWorldScale() {
    return m_localScale;
}
const KRVector3 &KRNode::getWorldRotation() {
    return m_localRotation;
}

std::string KRNode::getElementName() {
    return "node";
}

KRNode *KRNode::LoadXML(KRScene &scene, tinyxml2::XMLElement *e) {
    KRNode *new_node = NULL;
    const char *szElementName = e->Name();
    const char *szName = e->Attribute("name");
    if(strcmp(szElementName, "node") == 0) {
        new_node = new KRNode(scene, szName);
    } else if(strcmp(szElementName, "point_light") == 0) {
        new_node = new KRPointLight(scene, szName);
    } else if(strcmp(szElementName, "directional_light") == 0) {
        new_node = new KRDirectionalLight(scene, szName);
    } else if(strcmp(szElementName, "spot_light") == 0) {
        new_node = new KRSpotLight(scene, szName);
    } else if(strcmp(szElementName, "mesh") == 0) {
        float lod_min_coverage = 0.0f;
        if(e->QueryFloatAttribute("lod_min_coverage", &lod_min_coverage)  != tinyxml2::XML_SUCCESS) {
            lod_min_coverage = 0.0f; //1.0f / 1024.0f / 768.0f; // FINDME - HACK - Need to dynamically select the absolute minimum based on the render buffer size
        }
        new_node = new KRInstance(scene, szName, szName, e->Attribute("light_map"), lod_min_coverage);
    }
    
    if(new_node) {
        new_node->loadXML(e);
    }
    
    return new_node;
}

#if TARGET_OS_IPHONE

void KRNode::render(KRCamera *pCamera, KRContext *pContext, KRMat4 &viewMatrix, KRVector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers, KRNode::RenderPass renderPass) {
}

#endif

const std::vector<KRNode *> &KRNode::getChildren() {
    return m_childNodes;
}

const std::string &KRNode::getName() {
    return m_name;
}

KRScene &KRNode::getScene() {
    return *m_pScene;
}

KRAABB KRNode::getBounds() {
    return KRAABB::Infinite();
}
