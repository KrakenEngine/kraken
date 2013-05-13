//
//  KRNode.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-11.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KREngine-common.h"

#include "KRNode.h"
#include "KRLODGroup.h"
#include "KRPointLight.h"
#include "KRSpotLight.h"
#include "KRDirectionalLight.h"
#include "KRModel.h"
#include "KRCollider.h"
#include "KRParticleSystem.h"
#include "KRParticleSystemNewtonian.h"
#include "KRAABB.h"
#include "KRQuaternion.h"
#include "KRBone.h"
#include "KRAudioSource.h"
#include "KRAmbientZone.h"
#include "KRReverbZone.h"

KRNode::KRNode(KRScene &scene, std::string name) : KRContextObject(scene.getContext())
{
    m_name = name;
    m_localScale = KRVector3::One();
    m_localRotation = KRVector3::Zero();
    m_localTranslation = KRVector3::Zero();
    m_initialLocalTranslation = m_localTranslation;
    m_initialLocalScale = m_localScale;
    m_initialLocalRotation = m_localRotation;
    
    m_parentNode = NULL;
    m_pScene = &scene;
    m_modelMatrixValid = false;
    m_inverseModelMatrixValid = false;
    m_bindPoseMatrixValid = false;
    m_activePoseMatrixValid = false;
    m_inverseBindPoseMatrixValid = false;
    m_modelMatrix = KRMat4();
    m_bindPoseMatrix = KRMat4();
    m_activePoseMatrix = KRMat4();
    m_lod_visible = false;
    m_scale_compensation = false;
}

KRNode::~KRNode() {
    while(m_childNodes.size() > 0) {
        delete *m_childNodes.begin();
    }

    if(m_parentNode) {
        m_parentNode->childDeleted(this);
    }
    getScene().notify_sceneGraphDelete(this);
    
}

void KRNode::setScaleCompensation(bool scale_compensation)
{
    if(m_scale_compensation != scale_compensation) {
        m_scale_compensation = scale_compensation;
        invalidateModelMatrix();
        invalidateBindPoseMatrix();
    }
}
bool KRNode::getScaleCompensation()
{
    return m_scale_compensation;
}

void KRNode::childDeleted(KRNode *child_node)
{
    m_childNodes.erase(child_node);
    // InvalidateBounds();
    getScene().notify_sceneGraphModify(this);
}

void KRNode::addChild(KRNode *child) {
    assert(child->m_parentNode == NULL);
    child->m_parentNode = this;
    m_childNodes.insert(child);
    if(m_lod_visible) {
        // Child node inherits LOD visibility status from parent
        child->showLOD();
    }
}

tinyxml2::XMLElement *KRNode::saveXML(tinyxml2::XMLNode *parent) {
    tinyxml2::XMLDocument *doc = parent->GetDocument();
    tinyxml2::XMLElement *e = doc->NewElement(getElementName().c_str());
    tinyxml2::XMLNode *n = parent->InsertEndChild(e);
    e->SetAttribute("name", m_name.c_str());
    m_localTranslation.setXMLAttribute("translate", e);
    m_localScale.setXMLAttribute("scale", e);
    (m_localRotation * (180.0f / M_PI)).setXMLAttribute("rotate", e);
    
    
    m_rotationOffset.setXMLAttribute("rotate_offset", e);
    m_scalingOffset.setXMLAttribute("scale_offset", e);
    m_rotationPivot.setXMLAttribute("rotate_pivot", e);
    m_scalingPivot.setXMLAttribute("scale_pivot", e);
    (m_preRotation * (180.0f / M_PI)).setXMLAttribute("pre_rotate", e);
    (m_postRotation * (180.0f / M_PI)).setXMLAttribute("post_rotate", e);
    
    for(std::set<KRNode *>::iterator itr=m_childNodes.begin(); itr != m_childNodes.end(); ++itr) {
        KRNode *child = (*itr);
        child->saveXML(n);
    }
    return e;
}

void KRNode::loadXML(tinyxml2::XMLElement *e) {
    m_name = e->Attribute("name");
    m_localTranslation.getXMLAttribute("translate", e, KRVector3::Zero());
    m_localScale.getXMLAttribute("scale", e, KRVector3::One());
    m_localRotation.getXMLAttribute("rotate", e, KRVector3::Zero());
    m_localRotation *= M_PI / 180.0f; // Convert degrees to radians
    m_preRotation.getXMLAttribute("pre_rotate", e, KRVector3::Zero());
    m_preRotation *= M_PI / 180.0f; // Convert degrees to radians
    m_postRotation.getXMLAttribute("post_rotate", e, KRVector3::Zero());
    m_postRotation *= M_PI / 180.0f; // Convert degrees to radians

    
    m_rotationOffset.getXMLAttribute("rotate_offset", e, KRVector3::Zero());
    m_scalingOffset.getXMLAttribute("scale_offset", e, KRVector3::Zero());
    m_rotationPivot.getXMLAttribute("rotate_pivot", e, KRVector3::Zero());
    m_scalingPivot.getXMLAttribute("scale_pivot", e, KRVector3::Zero());

    
    m_initialLocalTranslation = m_localTranslation;
    m_initialLocalScale = m_localScale;
    m_initialLocalRotation = m_localRotation;
    
    m_initialRotationOffset = m_rotationOffset;
    m_initialScalingOffset = m_scalingOffset;
    m_initialRotationPivot = m_rotationPivot;
    m_initialScalingPivot = m_scalingPivot;
    m_initialPreRotation = m_preRotation;
    m_initialPostRotation = m_postRotation;
    
    m_bindPoseMatrixValid = false;
    m_activePoseMatrixValid = false;
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

void KRNode::setWorldTranslation(const KRVector3 &v)
{
    if(m_parentNode) {
        setLocalTranslation(KRMat4::Dot(m_parentNode->getInverseModelMatrix(), v));
    } else {
        setLocalTranslation(v);
    }
}


void KRNode::setWorldRotation(const KRVector3 &v)
{
    if(m_parentNode) {
        setLocalRotation(KRMat4::DotNoTranslate(m_parentNode->getInverseModelMatrix(), v));
    } else {
        setLocalRotation(v);
    }
}


void KRNode::setWorldScale(const KRVector3 &v)
{
    if(m_parentNode) {
        setLocalScale(KRMat4::DotNoTranslate(m_parentNode->getInverseModelMatrix(), v));
    } else {
        setLocalScale(v);
    }
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


void KRNode::setRotationOffset(const KRVector3 &v, bool set_original)
{
    m_rotationOffset = v;
    if(set_original) {
        m_initialRotationOffset = v;
        invalidateBindPoseMatrix();
    }
    invalidateModelMatrix();
}

void KRNode::setScalingOffset(const KRVector3 &v, bool set_original)
{
    m_scalingOffset = v;
    if(set_original) {
        m_initialScalingOffset = v;
        invalidateBindPoseMatrix();
    }
    invalidateModelMatrix();
}

void KRNode::setRotationPivot(const KRVector3 &v, bool set_original)
{
    m_rotationPivot = v;
    if(set_original) {
        m_initialRotationPivot = v;
        invalidateBindPoseMatrix();
    }
    invalidateModelMatrix();
}
void KRNode::setScalingPivot(const KRVector3 &v, bool set_original)
{
    m_scalingPivot = v;
    if(set_original) {
        m_initialScalingPivot = v;
        invalidateBindPoseMatrix();
    }
    invalidateModelMatrix();
}
void KRNode::setPreRotation(const KRVector3 &v, bool set_original)
{
    m_preRotation = v;
    if(set_original) {
        m_initialPreRotation = v;
        invalidateBindPoseMatrix();
    }
    invalidateModelMatrix();
}
void KRNode::setPostRotation(const KRVector3 &v, bool set_original)
{
    m_postRotation = v;
    if(set_original) {
        m_initialPostRotation = v;
        invalidateBindPoseMatrix();
    }
    invalidateModelMatrix();
}

const KRVector3 &KRNode::getRotationOffset()
{
    return m_rotationOffset;
}
const KRVector3 &KRNode::getScalingOffset()
{
    return m_scalingOffset;
}
const KRVector3 &KRNode::getRotationPivot()
{
    return m_rotationPivot;
}
const KRVector3 &KRNode::getScalingPivot()
{
    return m_scalingPivot;
}
const KRVector3 &KRNode::getPreRotation()
{
    return m_preRotation;
}
const KRVector3 &KRNode::getPostRotation()
{
    return m_postRotation;
}
const KRVector3 &KRNode::getInitialRotationOffset()
{
    return m_initialRotationOffset;
}
const KRVector3 &KRNode::getInitialScalingOffset()
{
    return m_initialScalingOffset;
}
const KRVector3 &KRNode::getInitialRotationPivot()
{
    return m_initialRotationPivot;
}
const KRVector3 &KRNode::getInitialScalingPivot()
{
    return m_initialScalingPivot;
}
const KRVector3 &KRNode::getInitialPreRotation()
{
    return m_initialPreRotation;
}
const KRVector3 &KRNode::getInitialPostRotation()
{
    return m_initialPostRotation;
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

const KRVector3 KRNode::getWorldTranslation() {
    return localToWorld(KRVector3::Zero());
}

const KRVector3 KRNode::getWorldScale() {
    return KRMat4::DotNoTranslate(getModelMatrix(), m_localScale);
}
const KRVector3 KRNode::getWorldRotation() {
    KRVector3 world_rotation = (-KRQuaternion(m_postRotation) * KRQuaternion(m_localRotation) * KRQuaternion(m_preRotation)).eulerXYZ();;
    if(m_parentNode) {
        KRVector3 parent_rotation = m_parentNode->getWorldRotation();
        world_rotation = (KRQuaternion(world_rotation) * KRQuaternion(parent_rotation)).eulerXYZ();
    }
    return world_rotation;
}

const KRVector3 KRNode::getBindPoseWorldRotation() {
    KRVector3 world_rotation = (-KRQuaternion(m_initialPostRotation) * KRQuaternion(m_initialLocalRotation) * KRQuaternion(m_initialPreRotation)).eulerXYZ();
    if(dynamic_cast<KRBone *>(m_parentNode)) {
        KRVector3 parent_rotation = m_parentNode->getBindPoseWorldRotation();
        world_rotation = (KRQuaternion(world_rotation) * KRQuaternion(parent_rotation)).eulerXYZ();
    }
    return world_rotation;
}

const KRVector3 KRNode::getActivePoseWorldRotation() {
    KRVector3 world_rotation = (KRQuaternion(m_preRotation) * KRQuaternion(m_localRotation) * -KRQuaternion(m_postRotation)).eulerXYZ();
    if(dynamic_cast<KRBone *>(m_parentNode)) {
        KRVector3 parent_rotation = m_parentNode->getActivePoseWorldRotation();
        world_rotation = (KRQuaternion(world_rotation) * KRQuaternion(parent_rotation)).eulerXYZ();
    }
    return world_rotation;
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
    } if(strcmp(szElementName, "lod_group") == 0) {
        new_node = new KRLODGroup(scene, szName);
    } else if(strcmp(szElementName, "point_light") == 0) {
        new_node = new KRPointLight(scene, szName);
    } else if(strcmp(szElementName, "directional_light") == 0) {
        new_node = new KRDirectionalLight(scene, szName);
    } else if(strcmp(szElementName, "spot_light") == 0) {
        new_node = new KRSpotLight(scene, szName);
    } else if(strcmp(szElementName, "particles_newtonian") == 0) {
        new_node = new KRParticleSystemNewtonian(scene, szName);
    } else if(strcmp(szElementName, "model") == 0) {
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
        new_node = new KRModel(scene, szName, e->Attribute("mesh"), e->Attribute("light_map"), lod_min_coverage, receives_shadow, faces_camera);
    } else if(strcmp(szElementName, "collider") == 0) {
        new_node = new KRCollider(scene, szName, e->Attribute("mesh"), 65535, 1.0f);
    } else if(strcmp(szElementName, "bone") == 0) {
        new_node = new KRBone(scene, szName);
    } else if(strcmp(szElementName, "audio_source") == 0) {
        new_node = new KRAudioSource(scene, szName);
    } else if(strcmp(szElementName, "ambient_zone") == 0) {
        new_node = new KRAmbientZone(scene, szName);
    } else if(strcmp(szElementName, "reverb_zone") == 0) {
        new_node = new KRReverbZone(scene, szName);
    }
    
    if(new_node) {
        new_node->loadXML(e);
    }
    
    return new_node;
}

void KRNode::render(KRCamera *pCamera, std::vector<KRPointLight *> &point_lights, std::vector<KRDirectionalLight *> &directional_lights, std::vector<KRSpotLight *>&spot_lights, const KRViewport &viewport, RenderPass renderPass)
{
}

const std::set<KRNode *> &KRNode::getChildren() {
    return m_childNodes;
}

const std::string &KRNode::getName() const {
    return m_name;
}

KRScene &KRNode::getScene() {
    return *m_pScene;
}

KRAABB KRNode::getBounds() {
    KRAABB bounds = KRAABB::Zero();

    bool first_child = true;
    for(std::set<KRNode *>::iterator itr=m_childNodes.begin(); itr != m_childNodes.end(); ++itr) {
        KRNode *child = (*itr);
        if(child->getBounds() != KRAABB::Zero()) {
            if(first_child) {
                first_child = false;
                bounds = child->getBounds();
            } else {
                bounds.encapsulate(child->getBounds());
            }
        }
    }

    return bounds;
}

void KRNode::invalidateModelMatrix()
{
    m_modelMatrixValid = false;
    m_activePoseMatrixValid = false;
    m_inverseModelMatrixValid = false;
    for(std::set<KRNode *>::iterator itr=m_childNodes.begin(); itr != m_childNodes.end(); ++itr) {
        KRNode *child = (*itr);
        child->invalidateModelMatrix();
    }
    
    // InvalidateBounds
    getScene().notify_sceneGraphModify(this);
}

void KRNode::invalidateBindPoseMatrix()
{
    m_bindPoseMatrixValid = false;
    m_inverseBindPoseMatrixValid = false;
    for(std::set<KRNode *>::iterator itr=m_childNodes.begin(); itr != m_childNodes.end(); ++itr) {
        KRNode *child = (*itr);
        child->invalidateBindPoseMatrix();
    }
}

const KRMat4 &KRNode::getModelMatrix()
{
    
    if(!m_modelMatrixValid) {
        m_modelMatrix = KRMat4();
        
        bool parent_is_bone = false;
        if(dynamic_cast<KRBone *>(m_parentNode)) {
            parent_is_bone = true;
        }
        
        if(getScaleCompensation() && parent_is_bone) {
            m_modelMatrix.translate(-m_scalingPivot);
            m_modelMatrix.scale(m_localScale);
            m_modelMatrix.translate(m_scalingPivot);
            m_modelMatrix.translate(m_scalingOffset);
            m_modelMatrix.translate(-m_rotationPivot);
            m_modelMatrix.rotate(-m_postRotation.z, Z_AXIS);
            m_modelMatrix.rotate(-m_postRotation.y, Y_AXIS);
            m_modelMatrix.rotate(-m_postRotation.x, X_AXIS);
            m_modelMatrix.rotate(m_localRotation.x, X_AXIS);
            m_modelMatrix.rotate(m_localRotation.y, Y_AXIS);
            m_modelMatrix.rotate(m_localRotation.z, Z_AXIS);
            m_modelMatrix.rotate(m_preRotation.x, X_AXIS);
            m_modelMatrix.rotate(m_preRotation.y, Y_AXIS);
            m_modelMatrix.rotate(m_preRotation.z, Z_AXIS);
            m_modelMatrix.translate(m_rotationPivot);
            m_modelMatrix.translate(m_rotationOffset);
            //m_modelMatrix.translate(m_localTranslation);
            if(m_parentNode) {
            
                m_modelMatrix.rotate(m_parentNode->getWorldRotation());
                m_modelMatrix.translate(KRMat4::Dot(m_parentNode->getModelMatrix(), m_localTranslation));
            } else {
                m_modelMatrix.translate(m_localTranslation);
            }
        } else {

            // WorldTransform = ParentWorldTransform * T * Roff * Rp * Rpre * R * Rpost * Rp-1 * Soff * Sp * S * Sp-1
            m_modelMatrix.translate(-m_scalingPivot);
            m_modelMatrix.scale(m_localScale);
            m_modelMatrix.translate(m_scalingPivot);
            m_modelMatrix.translate(m_scalingOffset);
            m_modelMatrix.translate(-m_rotationPivot);
            m_modelMatrix.rotate(-m_postRotation.z, Z_AXIS);
            m_modelMatrix.rotate(-m_postRotation.y, Y_AXIS);
            m_modelMatrix.rotate(-m_postRotation.x, X_AXIS);
            m_modelMatrix.rotate(m_localRotation.x, X_AXIS);
            m_modelMatrix.rotate(m_localRotation.y, Y_AXIS);
            m_modelMatrix.rotate(m_localRotation.z, Z_AXIS);
            m_modelMatrix.rotate(m_preRotation.x, X_AXIS);
            m_modelMatrix.rotate(m_preRotation.y, Y_AXIS);
            m_modelMatrix.rotate(m_preRotation.z, Z_AXIS);
            m_modelMatrix.translate(m_rotationPivot);
            m_modelMatrix.translate(m_rotationOffset);
            m_modelMatrix.translate(m_localTranslation);
            
            
            if(m_parentNode) {
                m_modelMatrix *= m_parentNode->getModelMatrix();
            }
        }
        
        m_modelMatrixValid = true;
        
    }
    return m_modelMatrix;
}

const KRMat4 &KRNode::getBindPoseMatrix()
{
    if(!m_bindPoseMatrixValid) {
        m_bindPoseMatrix = KRMat4();
        
        bool parent_is_bone = false;
        if(dynamic_cast<KRBone *>(m_parentNode)) {
            parent_is_bone = true;
        }
        
        if(getScaleCompensation() && parent_is_bone) {
            m_bindPoseMatrix.translate(-m_initialScalingPivot);
            m_bindPoseMatrix.scale(m_initialLocalScale);
            m_bindPoseMatrix.translate(m_initialScalingPivot);
            m_bindPoseMatrix.translate(m_initialScalingOffset);
            m_bindPoseMatrix.translate(-m_initialRotationPivot);
            m_bindPoseMatrix.rotate(-m_initialPostRotation.z, Z_AXIS);
            m_bindPoseMatrix.rotate(-m_initialPostRotation.y, Y_AXIS);
            m_bindPoseMatrix.rotate(-m_initialPostRotation.x, X_AXIS);
            m_bindPoseMatrix.rotate(m_initialLocalRotation.x, X_AXIS);
            m_bindPoseMatrix.rotate(m_initialLocalRotation.y, Y_AXIS);
            m_bindPoseMatrix.rotate(m_initialLocalRotation.z, Z_AXIS);
            m_bindPoseMatrix.rotate(m_initialPreRotation.x, X_AXIS);
            m_bindPoseMatrix.rotate(m_initialPreRotation.y, Y_AXIS);
            m_bindPoseMatrix.rotate(m_initialPreRotation.z, Z_AXIS);
            m_bindPoseMatrix.translate(m_initialRotationPivot);
            m_bindPoseMatrix.translate(m_initialRotationOffset);
            //m_bindPoseMatrix.translate(m_localTranslation);
            if(m_parentNode) {
                
                m_bindPoseMatrix.rotate(m_parentNode->getBindPoseWorldRotation());
                m_bindPoseMatrix.translate(KRMat4::Dot(m_parentNode->getBindPoseMatrix(), m_localTranslation));
            } else {
                m_bindPoseMatrix.translate(m_localTranslation);
            }
        } else {
            
            // WorldTransform = ParentWorldTransform * T * Roff * Rp * Rpre * R * Rpost * Rp-1 * Soff * Sp * S * Sp-1
            m_bindPoseMatrix.translate(-m_scalingPivot);
            m_bindPoseMatrix.scale(m_localScale);
            m_bindPoseMatrix.translate(m_scalingPivot);
            m_bindPoseMatrix.translate(m_scalingOffset);
            m_bindPoseMatrix.translate(-m_rotationPivot);
            m_bindPoseMatrix.rotate(-m_postRotation.z, Z_AXIS);
            m_bindPoseMatrix.rotate(-m_postRotation.y, Y_AXIS);
            m_bindPoseMatrix.rotate(-m_postRotation.x, X_AXIS);
            m_bindPoseMatrix.rotate(m_localRotation.x, X_AXIS);
            m_bindPoseMatrix.rotate(m_localRotation.y, Y_AXIS);
            m_bindPoseMatrix.rotate(m_localRotation.z, Z_AXIS);
            m_bindPoseMatrix.rotate(m_preRotation.x, X_AXIS);
            m_bindPoseMatrix.rotate(m_preRotation.y, Y_AXIS);
            m_bindPoseMatrix.rotate(m_preRotation.z, Z_AXIS);
            m_bindPoseMatrix.translate(m_rotationPivot);
            m_bindPoseMatrix.translate(m_rotationOffset);
            m_bindPoseMatrix.translate(m_localTranslation);
            
            
            if(m_parentNode && parent_is_bone) {
                m_bindPoseMatrix *= m_parentNode->getBindPoseMatrix();
            }
        }
        
        m_bindPoseMatrixValid = true;
        
    }
    return m_bindPoseMatrix;
}

const KRMat4 &KRNode::getActivePoseMatrix()
{
    
    if(!m_activePoseMatrixValid) {
        m_activePoseMatrix = KRMat4();
        
        bool parent_is_bone = false;
        if(dynamic_cast<KRBone *>(m_parentNode)) {
            parent_is_bone = true;
        }
        
        if(getScaleCompensation() && parent_is_bone) {
            m_activePoseMatrix.translate(-m_scalingPivot);
            m_activePoseMatrix.scale(m_localScale);
            m_activePoseMatrix.translate(m_scalingPivot);
            m_activePoseMatrix.translate(m_scalingOffset);
            m_activePoseMatrix.translate(-m_rotationPivot);
            m_activePoseMatrix.rotate(-m_postRotation.z, Z_AXIS);
            m_activePoseMatrix.rotate(-m_postRotation.y, Y_AXIS);
            m_activePoseMatrix.rotate(-m_postRotation.x, X_AXIS);
            m_activePoseMatrix.rotate(m_localRotation.x, X_AXIS);
            m_activePoseMatrix.rotate(m_localRotation.y, Y_AXIS);
            m_activePoseMatrix.rotate(m_localRotation.z, Z_AXIS);
            m_activePoseMatrix.rotate(m_preRotation.x, X_AXIS);
            m_activePoseMatrix.rotate(m_preRotation.y, Y_AXIS);
            m_activePoseMatrix.rotate(m_preRotation.z, Z_AXIS);
            m_activePoseMatrix.translate(m_rotationPivot);
            m_activePoseMatrix.translate(m_rotationOffset);
            if(m_parentNode) {
                
                m_activePoseMatrix.rotate(m_parentNode->getWorldRotation());
                m_activePoseMatrix.translate(KRMat4::Dot(m_parentNode->getActivePoseMatrix(), m_localTranslation));
            } else {
                m_activePoseMatrix.translate(m_localTranslation);
            }
        } else {
            
            // WorldTransform = ParentWorldTransform * T * Roff * Rp * Rpre * R * Rpost * Rp-1 * Soff * Sp * S * Sp-1
            m_activePoseMatrix.translate(-m_scalingPivot);
            m_activePoseMatrix.scale(m_localScale);
            m_activePoseMatrix.translate(m_scalingPivot);
            m_activePoseMatrix.translate(m_scalingOffset);
            m_activePoseMatrix.translate(-m_rotationPivot);
            m_activePoseMatrix.rotate(-m_postRotation.z, Z_AXIS);
            m_activePoseMatrix.rotate(-m_postRotation.y, Y_AXIS);
            m_activePoseMatrix.rotate(-m_postRotation.x, X_AXIS);
            m_activePoseMatrix.rotate(m_localRotation.x, X_AXIS);
            m_activePoseMatrix.rotate(m_localRotation.y, Y_AXIS);
            m_activePoseMatrix.rotate(m_localRotation.z, Z_AXIS);
            m_activePoseMatrix.rotate(m_preRotation.x, X_AXIS);
            m_activePoseMatrix.rotate(m_preRotation.y, Y_AXIS);
            m_activePoseMatrix.rotate(m_preRotation.z, Z_AXIS);
            m_activePoseMatrix.translate(m_rotationPivot);
            m_activePoseMatrix.translate(m_rotationOffset);
            m_activePoseMatrix.translate(m_localTranslation);
            
            
            if(m_parentNode && parent_is_bone) {
                m_activePoseMatrix *= m_parentNode->getActivePoseMatrix();
            }
        }
        
        m_activePoseMatrixValid = true;
        
    }
    return m_activePoseMatrix;

}

const KRMat4 &KRNode::getInverseModelMatrix()
{
    if(!m_inverseModelMatrixValid) {
        m_inverseModelMatrix = KRMat4::Invert(getModelMatrix());
    }
    return m_inverseModelMatrix;
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

void KRNode::removeFromOctreeNodes()
{
    for(std::set<KROctreeNode *>::iterator itr=m_octree_nodes.begin(); itr != m_octree_nodes.end(); itr++) {
        KROctreeNode *octree_node = *itr;
        octree_node->remove(this);
        
        // FINDME, TODO - This should be moved to the KROctree class
        while(octree_node) {
            octree_node->trim();
            if(octree_node->isEmpty()) {
                octree_node = octree_node->getParent();
            } else {
                octree_node = NULL;
            }
        }
    }
    m_octree_nodes.clear();
}

void KRNode::addToOctreeNode(KROctreeNode *octree_node)
{
    m_octree_nodes.insert(octree_node);
}

void KRNode::updateLODVisibility(const KRViewport &viewport)
{
    // If we aren't an LOD group node, then we just add ourselves and all our children to the octree
    showLOD();
}

void KRNode::hideLOD()
{
    if(m_lod_visible) {
        m_lod_visible = false;
        getScene().notify_sceneGraphDelete(this);
        for(std::set<KRNode *>::iterator itr=m_childNodes.begin(); itr != m_childNodes.end(); ++itr) {
            (*itr)->hideLOD();
        }
    }
}

void KRNode::showLOD()
{
    if(!m_lod_visible) {
        getScene().notify_sceneGraphCreate(this);
        m_lod_visible = true;
        for(std::set<KRNode *>::iterator itr=m_childNodes.begin(); itr != m_childNodes.end(); ++itr) {
            (*itr)->showLOD();
        }
    }
}


bool KRNode::lodIsVisible()
{
    return m_lod_visible;
}

const KRVector3 KRNode::localToWorld(const KRVector3 &local_point)
{
    return KRMat4::Dot(getModelMatrix(), local_point);
}

const KRVector3 KRNode::worldToLocal(const KRVector3 &world_point)
{
    return KRMat4::Dot(getInverseModelMatrix(), world_point);
}
