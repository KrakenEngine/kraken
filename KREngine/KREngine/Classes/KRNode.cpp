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
#import "KRCollider.h"
#import "KRParticleSystem.h"
#import "KRParticleSystemNewtonian.h"
#import "KRAABB.h"
#import "KRQuaternion.h"
#import "KRBone.h"


KRNode::KRNode(KRScene &scene, std::string name) : KRContextObject(scene.getContext())
{
    m_name = name;
    m_localScale = KRVector3::One();
    m_localRotation = KRVector3::Zero();
    m_localTranslation = KRVector3::Zero();
    m_parentNode = NULL;
    m_pScene = &scene;
    getScene().notify_sceneGraphCreate(this);
    m_modelMatrixValid = false;
    m_inverseModelMatrixValid = false;
    m_bindPoseMatrixValid = false;
    m_inverseBindPoseMatrixValid = false;
    m_modelMatrix = KRMat4();
    m_bindPoseMatrix = KRMat4();
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
    e->SetAttribute("translate_x", m_localTranslation.x); // TODO - Increase number of digits after the decimal in floating point format (6 -> 12?)
    e->SetAttribute("translate_y", m_localTranslation.y);
    e->SetAttribute("translate_z", m_localTranslation.z);
    e->SetAttribute("scale_x", m_localScale.x);
    e->SetAttribute("scale_y", m_localScale.y);
    e->SetAttribute("scale_z", m_localScale.z);
    e->SetAttribute("rotate_x", m_localRotation.x * 180 / M_PI);
    e->SetAttribute("rotate_y", m_localRotation.y * 180 / M_PI);
    e->SetAttribute("rotate_z", m_localRotation.z * 180 / M_PI);
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
    m_initialLocalTranslation = m_localTranslation;
    
    e->QueryFloatAttribute("scale_x", &x);
    e->QueryFloatAttribute("scale_y", &y);
    e->QueryFloatAttribute("scale_z", &z);
    m_localScale = KRVector3(x,y,z);
    m_initialLocalScale = m_localScale;
    
    e->QueryFloatAttribute("rotate_x", &x);
    e->QueryFloatAttribute("rotate_y", &y);
    e->QueryFloatAttribute("rotate_z", &z);
    m_localRotation = KRVector3(x,y,z) / 180.0 * M_PI; // Convert degrees to radians
    m_initialLocalRotation = m_localRotation;
    
    m_bindPoseMatrixValid = false;
    m_inverseBindPoseMatrixValid = false;
    m_modelMatrixValid = false;
    m_inverseModelMatrixValid = false;
    
    for(tinyxml2::XMLElement *child_element=e->FirstChildElement(); child_element != NULL; child_element = child_element->NextSiblingElement()) {
        KRNode *child_node = KRNode::LoadXML(getScene(), child_element);
        if(child_node) {
            addChild(child_node);
        }
    }
    
   
}

void KRNode::setLocalTranslation(const KRVector3 &v, bool set_original) {
    m_localTranslation = v;
    if(set_original) {
        m_initialLocalTranslation = v;
        invalidateBindPoseMatrix();
    }
    invalidateModelMatrix();
}
void KRNode::setLocalScale(const KRVector3 &v, bool set_original) {
    m_localScale = v;
    if(set_original) {
        m_initialLocalScale = v;
        invalidateBindPoseMatrix();
    }
    invalidateModelMatrix();
}

void KRNode::setLocalRotation(const KRVector3 &v, bool set_original) {
    m_localRotation = v;
    if(set_original) {
        m_initialLocalRotation = v;
        invalidateBindPoseMatrix();
    }
    invalidateModelMatrix();
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

const KRVector3 &KRNode::getInitialLocalTranslation() {
    return m_initialLocalTranslation;
}
const KRVector3 &KRNode::getInitialLocalScale() {
    return m_initialLocalScale;
}
const KRVector3 &KRNode::getInitialLocalRotation() {
    return m_initialLocalRotation;
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
    } else if(strcmp(szElementName, "particles_newtonian") == 0) {
        new_node = new KRParticleSystemNewtonian(scene, szName);
    } else if(strcmp(szElementName, "mesh") == 0) {
        float lod_min_coverage = 0.0f;
        if(e->QueryFloatAttribute("lod_min_coverage", &lod_min_coverage)  != tinyxml2::XML_SUCCESS) {
            lod_min_coverage = 0.0f;
        }
        bool receives_shadow = true;
        if(e->QueryBoolAttribute("receives_shadow", &receives_shadow) != tinyxml2::XML_SUCCESS) {
            receives_shadow = true;
        }
        bool faces_camera = false;
        if(e->QueryBoolAttribute("faces_camera", &faces_camera) != tinyxml2::XML_SUCCESS) {
            faces_camera = false;
        }
        new_node = new KRInstance(scene, szName, e->Attribute("mesh_name"), e->Attribute("light_map"), lod_min_coverage, receives_shadow, faces_camera);
    } else if(strcmp(szElementName, "collider") == 0) {
        new_node = new KRCollider(scene, szName, e->Attribute("collider_name"));
    } else if(strcmp(szElementName, "bone") == 0) {
        new_node = new KRBone(scene, szName);
    }
    
    if(new_node) {
        new_node->loadXML(e);
    }
    
    return new_node;
}

#if TARGET_OS_IPHONE

void KRNode::render(KRCamera *pCamera, std::vector<KRLight *> &lights, const KRViewport &viewport, RenderPass renderPass)
{
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

void KRNode::invalidateModelMatrix()
{
    m_modelMatrixValid = false;
    m_inverseModelMatrixValid = false;
    for(std::vector<KRNode *>::iterator itr=m_childNodes.begin(); itr < m_childNodes.end(); ++itr) {
        KRNode *child = (*itr);
        child->invalidateModelMatrix();
    }
}

void KRNode::invalidateBindPoseMatrix()
{
    m_bindPoseMatrixValid = false;
    m_inverseBindPoseMatrixValid = false;
    for(std::vector<KRNode *>::iterator itr=m_childNodes.begin(); itr < m_childNodes.end(); ++itr) {
        KRNode *child = (*itr);
        child->invalidateBindPoseMatrix();
    }
}

const KRMat4 &KRNode::getModelMatrix()
{
    
    if(!m_modelMatrixValid) {
        m_modelMatrix = KRMat4();

        m_modelMatrix.scale(m_localScale);
        m_modelMatrix.rotate(m_localRotation.x, X_AXIS);
        m_modelMatrix.rotate(m_localRotation.y, Y_AXIS);
        m_modelMatrix.rotate(m_localRotation.z, Z_AXIS);
        m_modelMatrix.translate(m_localTranslation);
        
        if(m_parentNode) {
            m_modelMatrix *= m_parentNode->getModelMatrix();
        }
        
        m_modelMatrixValid = true;
        
    }
    return m_modelMatrix;
}

const KRMat4 &KRNode::getInverseModelMatrix()
{
    if(!m_inverseModelMatrixValid) {
        m_inverseModelMatrix = KRMat4::Invert(getModelMatrix());
    }
    return m_inverseModelMatrix;
}

const KRMat4 &KRNode::getBindPoseMatrix()
{
    if(!m_bindPoseMatrixValid) {
        m_bindPoseMatrix = KRMat4();
        
        
        m_bindPoseMatrix.scale(m_initialLocalScale);
        m_bindPoseMatrix.rotate(m_initialLocalRotation.x, X_AXIS);
        m_bindPoseMatrix.rotate(m_initialLocalRotation.y, Y_AXIS);
        m_bindPoseMatrix.rotate(m_initialLocalRotation.z, Z_AXIS);
        m_bindPoseMatrix.translate(m_initialLocalTranslation);
        
        KRBone *parentBone = dynamic_cast<KRBone *>(m_parentNode);
        if(parentBone) {

            m_bindPoseMatrix *= parentBone->getBindPoseMatrix();
        }
        
        m_bindPoseMatrixValid = true;
    }
    return m_bindPoseMatrix;
}

const KRMat4 &KRNode::getInverseBindPoseMatrix()
{
    if(!m_inverseBindPoseMatrixValid ) {
        m_inverseBindPoseMatrix = KRMat4::Invert(getBindPoseMatrix());
        m_inverseBindPoseMatrixValid = true;
    }
    return m_inverseBindPoseMatrix;
}

void KRNode::physicsUpdate(float deltaTime)
{
    
}

bool KRNode::hasPhysics()
{
    return false;
}

void KRNode::SetAttribute(node_attribute_type attrib, float v)
{
    const float DEGREES_TO_RAD = M_PI / 180.0f;
    
    //printf("%s - ", m_name.c_str());
    switch(attrib) {
        case KRENGINE_NODE_ATTRIBUTE_TRANSLATE_X:
            //printf("translate_x: %f\n", v);
            setLocalTranslation(KRVector3(v, m_localTranslation.y, m_localTranslation.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_TRANSLATE_Y:
            //printf("translate_y: %f\n", v);
            setLocalTranslation(KRVector3(m_localTranslation.x, v, m_localTranslation.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_TRANSLATE_Z:
            //printf("translate_z: %f\n", v);
            setLocalTranslation(KRVector3(m_localTranslation.x, m_localTranslation.y, v));
            break;
        case KRENGINE_NODE_ATTRIBUTE_SCALE_X:
            //printf("scale_x: %f\n", v);
            setLocalScale(KRVector3(v, m_localScale.y, m_localScale.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_SCALE_Y:
            //printf("scale_y: %f\n", v);
            setLocalScale(KRVector3(m_localScale.x, v, m_localScale.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_SCALE_Z:
            //printf("scale_z: %f\n", v);
            setLocalScale(KRVector3(m_localScale.x, m_localScale.y, v));
            break;
        case KRENGINE_NODE_ATTRIBUTE_ROTATE_X:
            //printf("rotate_x: %f\n", v);
            setLocalRotation(KRVector3(v * DEGREES_TO_RAD, m_localRotation.y, m_localRotation.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_ROTATE_Y:
            //printf("rotate_y: %f\n", v);
            setLocalRotation(KRVector3(m_localRotation.x, v * DEGREES_TO_RAD, m_localRotation.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_ROTATE_Z:
            //printf("rotate_z: %f\n", v);
            setLocalRotation(KRVector3(m_localRotation.x, m_localRotation.y, v * DEGREES_TO_RAD));
            break;
    }
}


KRNode *KRNode::findChild(const std::string &name)
{
    if(m_name == name) {
        return this;
    } else {
        for(std::vector<KRNode *>::iterator child_itr = m_childNodes.begin(); child_itr != m_childNodes.end(); child_itr++) {
            KRNode *match = (*child_itr)->findChild(name);
            if(match) return match;
        }
    }
    
    return NULL;
}