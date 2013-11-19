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
#include "KRLocator.h"
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
    
    
    
    m_rotationOffset = KRVector3::Zero();
    m_scalingOffset = KRVector3::Zero();
    m_rotationPivot = KRVector3::Zero();
    m_scalingPivot = KRVector3::Zero();
    m_preRotation = KRVector3::Zero();
    m_postRotation = KRVector3::Zero();
    
    m_initialRotationOffset = KRVector3::Zero();
    m_initialScalingOffset = KRVector3::Zero();
    m_initialRotationPivot = KRVector3::Zero();
    m_initialScalingPivot = KRVector3::Zero();
    m_initialPreRotation = KRVector3::Zero();
    m_initialPostRotation = KRVector3::Zero();
    
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
    
    m_lastRenderFrame = -1000;
    for(int i=0; i < KRENGINE_NODE_ATTRIBUTE_COUNT; i++) {
        m_animation_mask[i] = false;
    }
}

KRNode::~KRNode() {
    
    
    while(m_childNodes.size() > 0) {
        delete *m_childNodes.begin();
    }
    
    for(std::set<KRBehavior *>::iterator itr = m_behaviors.begin(); itr != m_behaviors.end(); itr++) {
        delete *itr;
    }
    m_behaviors.clear();

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
        setLocalRotation((KRQuaternion(v) * -m_parentNode->getWorldRotation()).eulerXYZ());
        setPreRotation(KRVector3::Zero());
        setPostRotation(KRVector3::Zero());
    } else {
        setLocalRotation(v);
        setPreRotation(KRVector3::Zero());
        setPostRotation(KRVector3::Zero());
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
    } else if(strcmp(szElementName, "locator") == 0) {
        new_node = new KRLocator(scene, szName);
    } else if(strcmp(szElementName, "audio_source") == 0) {
        new_node = new KRAudioSource(scene, szName);
    } else if(strcmp(szElementName, "ambient_zone") == 0) {
        new_node = new KRAmbientZone(scene, szName);
    } else if(strcmp(szElementName, "reverb_zone") == 0) {
        new_node = new KRReverbZone(scene, szName);
    } else if(strcmp(szElementName, "camera") == 0) {
        new_node = new KRCamera(scene, szName);
    }

    if(new_node) {
        new_node->loadXML(e);
    }
    
    return new_node;
}

void KRNode::render(KRCamera *pCamera, std::vector<KRPointLight *> &point_lights, std::vector<KRDirectionalLight *> &directional_lights, std::vector<KRSpotLight *>&spot_lights, const KRViewport &viewport, RenderPass renderPass)
{
    m_lastRenderFrame = getContext().getCurrentFrame();
}

const std::set<KRNode *> &KRNode::getChildren() {
    return m_childNodes;
}

KRNode *KRNode::getParent() {
    return m_parentNode;
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
            
            
            // WorldTransform = ParentWorldTransform * T * Roff * Rp * Rpre * R * Rpost * Rp-1 * Soff * Sp * S * Sp-1
            m_modelMatrix = KRMat4::Translation(-m_scalingPivot)
                * KRMat4::Scaling(m_localScale)
                * KRMat4::Translation(m_scalingPivot)
                * KRMat4::Translation(m_scalingOffset)
                * KRMat4::Translation(-m_rotationPivot)
                //* (KRQuaternion(m_postRotation) * KRQuaternion(m_localRotation) * KRQuaternion(m_preRotation)).rotationMatrix()
                * KRMat4::Rotation(m_postRotation)
                * KRMat4::Rotation(m_localRotation)
                * KRMat4::Rotation(m_preRotation)
                * KRMat4::Translation(m_rotationPivot)
                * KRMat4::Translation(m_rotationOffset);
            
            if(m_parentNode) {
            
                m_modelMatrix.rotate(m_parentNode->getWorldRotation());
                m_modelMatrix.translate(KRMat4::Dot(m_parentNode->getModelMatrix(), m_localTranslation));
            } else {
                m_modelMatrix.translate(m_localTranslation);
            }
        } else {

            // WorldTransform = ParentWorldTransform * T * Roff * Rp * Rpre * R * Rpost * Rp-1 * Soff * Sp * S * Sp-1
            m_modelMatrix = KRMat4::Translation(-m_scalingPivot)
                * KRMat4::Scaling(m_localScale)
                * KRMat4::Translation(m_scalingPivot)
                * KRMat4::Translation(m_scalingOffset)
                * KRMat4::Translation(-m_rotationPivot)
            //* (KRQuaternion(m_postRotation) * KRQuaternion(m_localRotation) * KRQuaternion(m_preRotation)).rotationMatrix()
                            * KRMat4::Rotation(m_postRotation)
                            * KRMat4::Rotation(m_localRotation)
                            * KRMat4::Rotation(m_preRotation)
                * KRMat4::Translation(m_rotationPivot)
                * KRMat4::Translation(m_rotationOffset)
                * KRMat4::Translation(m_localTranslation);

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
            m_bindPoseMatrix = KRMat4::Translation(-m_initialScalingPivot)
            * KRMat4::Scaling(m_initialLocalScale)
            * KRMat4::Translation(m_initialScalingPivot)
            * KRMat4::Translation(m_initialScalingOffset)
            * KRMat4::Translation(-m_initialRotationPivot)
            //* (KRQuaternion(m_initialPostRotation) * KRQuaternion(m_initialLocalRotation) * KRQuaternion(m_initialPreRotation)).rotationMatrix()
            * KRMat4::Rotation(m_initialPostRotation)
            * KRMat4::Rotation(m_initialLocalRotation)
            * KRMat4::Rotation(m_initialPreRotation)
            * KRMat4::Translation(m_initialRotationPivot)
            * KRMat4::Translation(m_initialRotationOffset);
            //m_bindPoseMatrix.translate(m_localTranslation);
            if(m_parentNode) {
                
                m_bindPoseMatrix.rotate(m_parentNode->getBindPoseWorldRotation());
                m_bindPoseMatrix.translate(KRMat4::Dot(m_parentNode->getBindPoseMatrix(), m_localTranslation));
            } else {
                m_bindPoseMatrix.translate(m_localTranslation);
            }
        } else {
            
            // WorldTransform = ParentWorldTransform * T * Roff * Rp * Rpre * R * Rpost * Rp-1 * Soff * Sp * S * Sp-1
            
            m_bindPoseMatrix = KRMat4::Translation(-m_initialScalingPivot)
            * KRMat4::Scaling(m_initialLocalScale)
            * KRMat4::Translation(m_initialScalingPivot)
            * KRMat4::Translation(m_initialScalingOffset)
            * KRMat4::Translation(-m_initialRotationPivot)
           // * (KRQuaternion(m_initialPostRotation) * KRQuaternion(m_initialLocalRotation) * KRQuaternion(m_initialPreRotation)).rotationMatrix()
                        * KRMat4::Rotation(m_initialPostRotation)
                        * KRMat4::Rotation(m_initialLocalRotation)
                        * KRMat4::Rotation(m_initialPreRotation)
            * KRMat4::Translation(m_initialRotationPivot)
            * KRMat4::Translation(m_initialRotationOffset)
            * KRMat4::Translation(m_initialLocalTranslation);
            
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
            m_activePoseMatrix= KRMat4::Translation(-m_scalingPivot)
            * KRMat4::Scaling(m_localScale)
            * KRMat4::Translation(m_scalingPivot)
            * KRMat4::Translation(m_scalingOffset)
            * KRMat4::Translation(-m_rotationPivot)
            * KRMat4::Rotation(m_postRotation)
            * KRMat4::Rotation(m_localRotation)
            * KRMat4::Rotation(m_preRotation)
            * KRMat4::Translation(m_rotationPivot)
            * KRMat4::Translation(m_rotationOffset);
            
            if(m_parentNode) {
                
                m_activePoseMatrix.rotate(m_parentNode->getActivePoseWorldRotation());
                m_activePoseMatrix.translate(KRMat4::Dot(m_parentNode->getActivePoseMatrix(), m_localTranslation));
            } else {
                m_activePoseMatrix.translate(m_localTranslation);
            }
        } else {
            
            // WorldTransform = ParentWorldTransform * T * Roff * Rp * Rpre * R * Rpost * Rp-1 * Soff * Sp * S * Sp-1
            m_activePoseMatrix = KRMat4::Translation(-m_scalingPivot)
            * KRMat4::Scaling(m_localScale)
            * KRMat4::Translation(m_scalingPivot)
            * KRMat4::Translation(m_scalingOffset)
            * KRMat4::Translation(-m_rotationPivot)
            * KRMat4::Rotation(m_postRotation)
            * KRMat4::Rotation(m_localRotation)
            * KRMat4::Rotation(m_preRotation)
            * KRMat4::Translation(m_rotationPivot)
            * KRMat4::Translation(m_rotationOffset)
            * KRMat4::Translation(m_localTranslation);
            
            
            if(m_parentNode && parent_is_bone) {
                m_activePoseMatrix *= m_parentNode->getActivePoseMatrix();
            }
        }
        
        m_activePoseMatrixValid = true;
        
    }
    return m_activePoseMatrix;

}

const KRQuaternion KRNode::getWorldRotation() {
    KRQuaternion world_rotation = KRQuaternion(m_postRotation) * KRQuaternion(m_localRotation) * KRQuaternion(m_preRotation);
    if(m_parentNode) {
        world_rotation = world_rotation * m_parentNode->getWorldRotation();
    }
    return world_rotation;
}

const KRQuaternion KRNode::getBindPoseWorldRotation() {
    KRQuaternion world_rotation = KRQuaternion(m_initialPostRotation) * KRQuaternion(m_initialLocalRotation) * KRQuaternion(m_initialPreRotation);
    if(dynamic_cast<KRBone *>(m_parentNode)) {
        world_rotation = world_rotation * m_parentNode->getBindPoseWorldRotation();
    }
    return world_rotation;
}

const KRQuaternion KRNode::getActivePoseWorldRotation() {
    KRQuaternion world_rotation = KRQuaternion(m_postRotation) * KRQuaternion(m_localRotation) * KRQuaternion(m_preRotation);
    if(dynamic_cast<KRBone *>(m_parentNode)) {
        world_rotation = world_rotation * m_parentNode->getActivePoseWorldRotation();
    }
    return world_rotation;
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
    const long MIN_DISPLAY_FRAMES = 10;
    bool visible = m_lastRenderFrame + MIN_DISPLAY_FRAMES >= getContext().getCurrentFrame();
    for(std::set<KRBehavior *>::iterator itr=m_behaviors.begin(); itr != m_behaviors.end(); itr++) {
        (*itr)->update(deltaTime);
        if(visible) {
            (*itr)->visibleUpdate(deltaTime);
        }
    }
}

bool KRNode::hasPhysics()
{
    return m_behaviors.size() > 0;
}

void KRNode::SetAttribute(node_attribute_type attrib, float v)
{
    if(m_animation_mask[attrib]) return;
    
    const float DEGREES_TO_RAD = M_PI / 180.0f;
    
    //printf("%s - ", m_name.c_str());
    switch(attrib) {
        case KRENGINE_NODE_ATTRIBUTE_TRANSLATE_X:
            setLocalTranslation(KRVector3(v, m_localTranslation.y, m_localTranslation.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_TRANSLATE_Y:
            setLocalTranslation(KRVector3(m_localTranslation.x, v, m_localTranslation.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_TRANSLATE_Z:
            setLocalTranslation(KRVector3(m_localTranslation.x, m_localTranslation.y, v));
            break;
        case KRENGINE_NODE_ATTRIBUTE_SCALE_X:
            setLocalScale(KRVector3(v, m_localScale.y, m_localScale.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_SCALE_Y:
            setLocalScale(KRVector3(m_localScale.x, v, m_localScale.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_SCALE_Z:
            setLocalScale(KRVector3(m_localScale.x, m_localScale.y, v));
            break;
        case KRENGINE_NODE_ATTRIBUTE_ROTATE_X:
            setLocalRotation(KRVector3(v * DEGREES_TO_RAD, m_localRotation.y, m_localRotation.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_ROTATE_Y:
            setLocalRotation(KRVector3(m_localRotation.x, v * DEGREES_TO_RAD, m_localRotation.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_ROTATE_Z:
            setLocalRotation(KRVector3(m_localRotation.x, m_localRotation.y, v * DEGREES_TO_RAD));
            break;
            

        case KRENGINE_NODE_ATTRIBUTE_PRE_ROTATION_X:
            setPreRotation(KRVector3(v * DEGREES_TO_RAD, m_preRotation.y, m_preRotation.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_PRE_ROTATION_Y:
            setPreRotation(KRVector3(m_preRotation.x, v * DEGREES_TO_RAD, m_preRotation.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_PRE_ROTATION_Z:
            setPreRotation(KRVector3(m_preRotation.x, m_preRotation.y, v * DEGREES_TO_RAD));
            break;
        case KRENGINE_NODE_ATTRIBUTE_POST_ROTATION_X:
            setPostRotation(KRVector3(v * DEGREES_TO_RAD, m_postRotation.y, m_postRotation.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_POST_ROTATION_Y:
            setPostRotation(KRVector3(m_postRotation.x, v * DEGREES_TO_RAD, m_postRotation.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_POST_ROTATION_Z:
            setPostRotation(KRVector3(m_postRotation.x, m_postRotation.y, v * DEGREES_TO_RAD));
            break;
        case KRENGINE_NODE_ATTRIBUTE_ROTATION_PIVOT_X:
            setRotationPivot(KRVector3(v, m_rotationPivot.y, m_rotationPivot.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_ROTATION_PIVOT_Y:
            setRotationPivot(KRVector3(m_rotationPivot.x, v, m_rotationPivot.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_ROTATION_PIVOT_Z:
            setRotationPivot(KRVector3(m_rotationPivot.x, m_rotationPivot.y, v));
            break;
        case KRENGINE_NODE_ATTRIBUTE_SCALE_PIVOT_X:
            setScalingPivot(KRVector3(v, m_scalingPivot.y, m_scalingPivot.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_SCALE_PIVOT_Y:
            setScalingPivot(KRVector3(m_scalingPivot.x, v, m_scalingPivot.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_SCALE_PIVOT_Z:
            setScalingPivot(KRVector3(m_scalingPivot.x, m_scalingPivot.y, v));
            break;
        case KRENGINE_NODE_ATTRIBUTE_ROTATE_OFFSET_X:
            setRotationOffset(KRVector3(v, m_rotationOffset.y, m_rotationOffset.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_ROTATE_OFFSET_Y:
            setRotationOffset(KRVector3(m_rotationOffset.x, v, m_rotationOffset.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_ROTATE_OFFSET_Z:
            setRotationOffset(KRVector3(m_rotationOffset.x, m_rotationOffset.y, v));
            break;
        case KRENGINE_NODE_SCALE_OFFSET_X:
            setScalingOffset(KRVector3(v, m_scalingOffset.y, m_scalingOffset.z));
            break;
        case KRENGINE_NODE_SCALE_OFFSET_Y:
            setScalingOffset(KRVector3(m_scalingOffset.x, v, m_scalingOffset.z));
            break;
        case KRENGINE_NODE_SCALE_OFFSET_Z:
            setScalingOffset(KRVector3(m_scalingOffset.x, m_scalingOffset.y, v));
            break;
    }
}

void KRNode::setAnimationEnabled(node_attribute_type attrib, bool enable)
{
    m_animation_mask[attrib] = !enable;
}
bool KRNode::getAnimationEnabled(node_attribute_type attrib) const
{
    return !m_animation_mask[attrib];
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

void KRNode::addBehavior(KRBehavior *behavior)
{
    m_behaviors.insert(behavior);
    behavior->__setNode(this);
    getScene().notify_sceneGraphModify(this);
}