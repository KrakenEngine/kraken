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
#include "KRLODSet.h"
#include "KRPointLight.h"
#include "KRSpotLight.h"
#include "KRDirectionalLight.h"
#include "KRModel.h"
#include "KRCollider.h"
#include "KRParticleSystem.h"
#include "KRParticleSystemNewtonian.h"
#include "KRBone.h"
#include "KRLocator.h"
#include "KRAudioSource.h"
#include "KRAmbientZone.h"
#include "KRReverbZone.h"
#include "KRSprite.h"

KRNode::KRNode(KRScene &scene, std::string name) : KRContextObject(scene.getContext())
{
    m_name = name;
    m_localScale = Vector3::One();
    m_localRotation = Vector3::Zero();
    m_localTranslation = Vector3::Zero();
    m_initialLocalTranslation = m_localTranslation;
    m_initialLocalScale = m_localScale;
    m_initialLocalRotation = m_localRotation;
    
    
    
    m_rotationOffset = Vector3::Zero();
    m_scalingOffset = Vector3::Zero();
    m_rotationPivot = Vector3::Zero();
    m_scalingPivot = Vector3::Zero();
    m_preRotation = Vector3::Zero();
    m_postRotation = Vector3::Zero();
    
    m_initialRotationOffset = Vector3::Zero();
    m_initialScalingOffset = Vector3::Zero();
    m_initialRotationPivot = Vector3::Zero();
    m_initialScalingPivot = Vector3::Zero();
    m_initialPreRotation = Vector3::Zero();
    m_initialPostRotation = Vector3::Zero();
    
    m_parentNode = NULL;
    m_pScene = &scene;
    m_modelMatrixValid = false;
    m_inverseModelMatrixValid = false;
    m_bindPoseMatrixValid = false;
    m_activePoseMatrixValid = false;
    m_inverseBindPoseMatrixValid = false;
    m_modelMatrix = Matrix4();
    m_bindPoseMatrix = Matrix4();
    m_activePoseMatrix = Matrix4();
    m_lod_visible = LOD_VISIBILITY_HIDDEN;
    m_scale_compensation = false;
    m_boundsValid = false;
    
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
    invalidateBounds();
    getScene().notify_sceneGraphModify(this);
}

void KRNode::addChild(KRNode *child) {
    assert(child->m_parentNode == NULL);
    child->m_parentNode = this;
    m_childNodes.insert(child);
    child->setLODVisibility(m_lod_visible); // Child node inherits LOD visibility status from parent
}

tinyxml2::XMLElement *KRNode::saveXML(tinyxml2::XMLNode *parent) {
    tinyxml2::XMLDocument *doc = parent->GetDocument();
    tinyxml2::XMLElement *e = doc->NewElement(getElementName().c_str());
    tinyxml2::XMLNode *n = parent->InsertEndChild(e);
    e->SetAttribute("name", m_name.c_str());
    kraken::setXMLAttribute("translate", e, m_localTranslation, Vector3::Zero());
    kraken::setXMLAttribute("scale", e, m_localScale, Vector3::One());
    kraken::setXMLAttribute("rotate", e, (m_localRotation * (180.0f / M_PI)), Vector3::Zero());
    kraken::setXMLAttribute("rotate_offset", e, m_rotationOffset, Vector3::Zero());
    kraken::setXMLAttribute("scale_offset", e, m_scalingOffset, Vector3::Zero());
    kraken::setXMLAttribute("rotate_pivot", e, m_rotationPivot, Vector3::Zero());
    kraken::setXMLAttribute("scale_pivot", e, m_scalingPivot, Vector3::Zero());
    kraken::setXMLAttribute("pre_rotate", e, (m_preRotation * (180.0f / M_PI)), Vector3::Zero());
    kraken::setXMLAttribute("post_rotate", e, (m_postRotation * (180.0f / M_PI)), Vector3::Zero());
    
    for(std::set<KRNode *>::iterator itr=m_childNodes.begin(); itr != m_childNodes.end(); ++itr) {
        KRNode *child = (*itr);
        child->saveXML(n);
    }
    return e;
}

void KRNode::loadXML(tinyxml2::XMLElement *e) {
    m_name = e->Attribute("name");
    m_localTranslation = kraken::getXMLAttribute("translate", e, Vector3::Zero());
    m_localScale = kraken::getXMLAttribute("scale", e, Vector3::One());
    m_localRotation = kraken::getXMLAttribute("rotate", e, Vector3::Zero());
    m_localRotation *= M_PI / 180.0f; // Convert degrees to radians
    m_preRotation = kraken::getXMLAttribute("pre_rotate", e, Vector3::Zero());
    m_preRotation *= M_PI / 180.0f; // Convert degrees to radians
    m_postRotation = kraken::getXMLAttribute("post_rotate", e, Vector3::Zero());
    m_postRotation *= M_PI / 180.0f; // Convert degrees to radians

    m_rotationOffset = kraken::getXMLAttribute("rotate_offset", e, Vector3::Zero());
    m_scalingOffset = kraken::getXMLAttribute("scale_offset", e, Vector3::Zero());
    m_rotationPivot = kraken::getXMLAttribute("rotate_pivot", e, Vector3::Zero());
    m_scalingPivot = kraken::getXMLAttribute("scale_pivot", e, Vector3::Zero());
    
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
        const char *szElementName = child_element->Name();
        if(strcmp(szElementName, "behavior") == 0) {
            KRBehavior *behavior = KRBehavior::LoadXML(this, child_element);
            if(behavior) {
                addBehavior(behavior);
                behavior->init();
            }
        } else {
            KRNode *child_node = KRNode::LoadXML(getScene(), child_element);
            
            if(child_node) {
                addChild(child_node);
            }
        }
    }
}

void KRNode::setLocalTranslation(const Vector3 &v, bool set_original) {
    m_localTranslation = v;
    if(set_original) {
        m_initialLocalTranslation = v;
        invalidateBindPoseMatrix();
    }
    invalidateModelMatrix();
}

void KRNode::setWorldTranslation(const Vector3 &v)
{
    if(m_parentNode) {
        setLocalTranslation(Matrix4::Dot(m_parentNode->getInverseModelMatrix(), v));
    } else {
        setLocalTranslation(v);
    }
}


void KRNode::setWorldRotation(const Vector3 &v)
{
    if(m_parentNode) {
        setLocalRotation((KRQuaternion(v) * -m_parentNode->getWorldRotation()).eulerXYZ());
        setPreRotation(Vector3::Zero());
        setPostRotation(Vector3::Zero());
    } else {
        setLocalRotation(v);
        setPreRotation(Vector3::Zero());
        setPostRotation(Vector3::Zero());
    }
}


void KRNode::setWorldScale(const Vector3 &v)
{
    if(m_parentNode) {
        setLocalScale(Matrix4::DotNoTranslate(m_parentNode->getInverseModelMatrix(), v));
    } else {
        setLocalScale(v);
    }
}

void KRNode::setLocalScale(const Vector3 &v, bool set_original) {
    m_localScale = v;
    if(set_original) {
        m_initialLocalScale = v;
        invalidateBindPoseMatrix();
    }
    invalidateModelMatrix();
}

void KRNode::setLocalRotation(const Vector3 &v, bool set_original) {
    m_localRotation = v;
    if(set_original) {
        m_initialLocalRotation = v;
        invalidateBindPoseMatrix();
    }
    invalidateModelMatrix();
}


void KRNode::setRotationOffset(const Vector3 &v, bool set_original)
{
    m_rotationOffset = v;
    if(set_original) {
        m_initialRotationOffset = v;
        invalidateBindPoseMatrix();
    }
    invalidateModelMatrix();
}

void KRNode::setScalingOffset(const Vector3 &v, bool set_original)
{
    m_scalingOffset = v;
    if(set_original) {
        m_initialScalingOffset = v;
        invalidateBindPoseMatrix();
    }
    invalidateModelMatrix();
}

void KRNode::setRotationPivot(const Vector3 &v, bool set_original)
{
    m_rotationPivot = v;
    if(set_original) {
        m_initialRotationPivot = v;
        invalidateBindPoseMatrix();
    }
    invalidateModelMatrix();
}
void KRNode::setScalingPivot(const Vector3 &v, bool set_original)
{
    m_scalingPivot = v;
    if(set_original) {
        m_initialScalingPivot = v;
        invalidateBindPoseMatrix();
    }
    invalidateModelMatrix();
}
void KRNode::setPreRotation(const Vector3 &v, bool set_original)
{
    m_preRotation = v;
    if(set_original) {
        m_initialPreRotation = v;
        invalidateBindPoseMatrix();
    }
    invalidateModelMatrix();
}
void KRNode::setPostRotation(const Vector3 &v, bool set_original)
{
    m_postRotation = v;
    if(set_original) {
        m_initialPostRotation = v;
        invalidateBindPoseMatrix();
    }
    invalidateModelMatrix();
}

const Vector3 &KRNode::getRotationOffset()
{
    return m_rotationOffset;
}
const Vector3 &KRNode::getScalingOffset()
{
    return m_scalingOffset;
}
const Vector3 &KRNode::getRotationPivot()
{
    return m_rotationPivot;
}
const Vector3 &KRNode::getScalingPivot()
{
    return m_scalingPivot;
}
const Vector3 &KRNode::getPreRotation()
{
    return m_preRotation;
}
const Vector3 &KRNode::getPostRotation()
{
    return m_postRotation;
}
const Vector3 &KRNode::getInitialRotationOffset()
{
    return m_initialRotationOffset;
}
const Vector3 &KRNode::getInitialScalingOffset()
{
    return m_initialScalingOffset;
}
const Vector3 &KRNode::getInitialRotationPivot()
{
    return m_initialRotationPivot;
}
const Vector3 &KRNode::getInitialScalingPivot()
{
    return m_initialScalingPivot;
}
const Vector3 &KRNode::getInitialPreRotation()
{
    return m_initialPreRotation;
}
const Vector3 &KRNode::getInitialPostRotation()
{
    return m_initialPostRotation;
}

const Vector3 &KRNode::getLocalTranslation() {
    return m_localTranslation;
}
const Vector3 &KRNode::getLocalScale() {
    return m_localScale;
}
const Vector3 &KRNode::getLocalRotation() {
    return m_localRotation;
}

const Vector3 &KRNode::getInitialLocalTranslation() {
    return m_initialLocalTranslation;
}
const Vector3 &KRNode::getInitialLocalScale() {
    return m_initialLocalScale;
}
const Vector3 &KRNode::getInitialLocalRotation() {
    return m_initialLocalRotation;
}

const Vector3 KRNode::getWorldTranslation() {
    return localToWorld(Vector3::Zero());
}

const Vector3 KRNode::getWorldScale() {
    return Matrix4::DotNoTranslate(getModelMatrix(), m_localScale);
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
    } else if(strcmp(szElementName, "lod_set") == 0) {
        new_node = new KRLODSet(scene, szName);
    } else if(strcmp(szElementName, "lod_group") == 0) {
        new_node = new KRLODGroup(scene, szName);
    } else if(strcmp(szElementName, "point_light") == 0) {
        new_node = new KRPointLight(scene, szName);
    } else if(strcmp(szElementName, "directional_light") == 0) {
        new_node = new KRDirectionalLight(scene, szName);
    } else if(strcmp(szElementName, "spot_light") == 0) {
        new_node = new KRSpotLight(scene, szName);
    } else if(strcmp(szElementName, "particles_newtonian") == 0) {
        new_node = new KRParticleSystemNewtonian(scene, szName);
    } else if(strcmp(szElementName, "sprite") == 0) {
        new_node = new KRSprite(scene, szName);
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
        float rim_power = 0.0f;
        if(e->QueryFloatAttribute("rim_power", &rim_power) != tinyxml2::XML_SUCCESS) {
            rim_power = 0.0f;
        }
        Vector3 rim_color = Vector3::Zero();
        rim_color = kraken::getXMLAttribute("rim_color", e, Vector3::Zero());
        new_node = new KRModel(scene, szName, e->Attribute("mesh"), e->Attribute("light_map"), lod_min_coverage, receives_shadow, faces_camera, rim_color, rim_power);
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
    if(m_lod_visible <= LOD_VISIBILITY_PRESTREAM) return;
    
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
    if(!m_boundsValid) {
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
        
        m_bounds = bounds;
        m_boundsValid = true;
    }
    return m_bounds;
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
    
    invalidateBounds();
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

const Matrix4 &KRNode::getModelMatrix()
{
    
    if(!m_modelMatrixValid) {
        m_modelMatrix = Matrix4();
        
        bool parent_is_bone = false;
        if(dynamic_cast<KRBone *>(m_parentNode)) {
            parent_is_bone = true;
        }
        
        if(getScaleCompensation() && parent_is_bone) {
            
            
            // WorldTransform = ParentWorldTransform * T * Roff * Rp * Rpre * R * Rpost * Rp-1 * Soff * Sp * S * Sp-1
            m_modelMatrix = Matrix4::Translation(-m_scalingPivot)
                * Matrix4::Scaling(m_localScale)
                * Matrix4::Translation(m_scalingPivot)
                * Matrix4::Translation(m_scalingOffset)
                * Matrix4::Translation(-m_rotationPivot)
                //* (KRQuaternion(m_postRotation) * KRQuaternion(m_localRotation) * KRQuaternion(m_preRotation)).rotationMatrix()
                * Matrix4::Rotation(m_postRotation)
                * Matrix4::Rotation(m_localRotation)
                * Matrix4::Rotation(m_preRotation)
                * Matrix4::Translation(m_rotationPivot)
                * Matrix4::Translation(m_rotationOffset);
            
            if(m_parentNode) {
            
                m_modelMatrix.rotate(m_parentNode->getWorldRotation());
                m_modelMatrix.translate(Matrix4::Dot(m_parentNode->getModelMatrix(), m_localTranslation));
            } else {
                m_modelMatrix.translate(m_localTranslation);
            }
        } else {

            // WorldTransform = ParentWorldTransform * T * Roff * Rp * Rpre * R * Rpost * Rp-1 * Soff * Sp * S * Sp-1
            m_modelMatrix = Matrix4::Translation(-m_scalingPivot)
                * Matrix4::Scaling(m_localScale)
                * Matrix4::Translation(m_scalingPivot)
                * Matrix4::Translation(m_scalingOffset)
                * Matrix4::Translation(-m_rotationPivot)
            //* (KRQuaternion(m_postRotation) * KRQuaternion(m_localRotation) * KRQuaternion(m_preRotation)).rotationMatrix()
                            * Matrix4::Rotation(m_postRotation)
                            * Matrix4::Rotation(m_localRotation)
                            * Matrix4::Rotation(m_preRotation)
                * Matrix4::Translation(m_rotationPivot)
                * Matrix4::Translation(m_rotationOffset)
                * Matrix4::Translation(m_localTranslation);

            if(m_parentNode) {
                m_modelMatrix *= m_parentNode->getModelMatrix();
            }
        }
        
        m_modelMatrixValid = true;
        
    }
    return m_modelMatrix;
}

const Matrix4 &KRNode::getBindPoseMatrix()
{
    if(!m_bindPoseMatrixValid) {
        m_bindPoseMatrix = Matrix4();
        
        bool parent_is_bone = false;
        if(dynamic_cast<KRBone *>(m_parentNode)) {
            parent_is_bone = true;
        }
        
        if(getScaleCompensation() && parent_is_bone) {
            m_bindPoseMatrix = Matrix4::Translation(-m_initialScalingPivot)
            * Matrix4::Scaling(m_initialLocalScale)
            * Matrix4::Translation(m_initialScalingPivot)
            * Matrix4::Translation(m_initialScalingOffset)
            * Matrix4::Translation(-m_initialRotationPivot)
            //* (KRQuaternion(m_initialPostRotation) * KRQuaternion(m_initialLocalRotation) * KRQuaternion(m_initialPreRotation)).rotationMatrix()
            * Matrix4::Rotation(m_initialPostRotation)
            * Matrix4::Rotation(m_initialLocalRotation)
            * Matrix4::Rotation(m_initialPreRotation)
            * Matrix4::Translation(m_initialRotationPivot)
            * Matrix4::Translation(m_initialRotationOffset);
            //m_bindPoseMatrix.translate(m_localTranslation);
            if(m_parentNode) {
                
                m_bindPoseMatrix.rotate(m_parentNode->getBindPoseWorldRotation());
                m_bindPoseMatrix.translate(Matrix4::Dot(m_parentNode->getBindPoseMatrix(), m_localTranslation));
            } else {
                m_bindPoseMatrix.translate(m_localTranslation);
            }
        } else {
            
            // WorldTransform = ParentWorldTransform * T * Roff * Rp * Rpre * R * Rpost * Rp-1 * Soff * Sp * S * Sp-1
            
            m_bindPoseMatrix = Matrix4::Translation(-m_initialScalingPivot)
            * Matrix4::Scaling(m_initialLocalScale)
            * Matrix4::Translation(m_initialScalingPivot)
            * Matrix4::Translation(m_initialScalingOffset)
            * Matrix4::Translation(-m_initialRotationPivot)
           // * (KRQuaternion(m_initialPostRotation) * KRQuaternion(m_initialLocalRotation) * KRQuaternion(m_initialPreRotation)).rotationMatrix()
                        * Matrix4::Rotation(m_initialPostRotation)
                        * Matrix4::Rotation(m_initialLocalRotation)
                        * Matrix4::Rotation(m_initialPreRotation)
            * Matrix4::Translation(m_initialRotationPivot)
            * Matrix4::Translation(m_initialRotationOffset)
            * Matrix4::Translation(m_initialLocalTranslation);
            
            if(m_parentNode && parent_is_bone) {

                m_bindPoseMatrix *= m_parentNode->getBindPoseMatrix();
            }
        }
        
        m_bindPoseMatrixValid = true;
        
    }
    return m_bindPoseMatrix;
}

const Matrix4 &KRNode::getActivePoseMatrix()
{
    
    if(!m_activePoseMatrixValid) {
        m_activePoseMatrix = Matrix4();
        
        bool parent_is_bone = false;
        if(dynamic_cast<KRBone *>(m_parentNode)) {
            parent_is_bone = true;
        }
        
        if(getScaleCompensation() && parent_is_bone) {
            m_activePoseMatrix= Matrix4::Translation(-m_scalingPivot)
            * Matrix4::Scaling(m_localScale)
            * Matrix4::Translation(m_scalingPivot)
            * Matrix4::Translation(m_scalingOffset)
            * Matrix4::Translation(-m_rotationPivot)
            * Matrix4::Rotation(m_postRotation)
            * Matrix4::Rotation(m_localRotation)
            * Matrix4::Rotation(m_preRotation)
            * Matrix4::Translation(m_rotationPivot)
            * Matrix4::Translation(m_rotationOffset);
            
            if(m_parentNode) {
                
                m_activePoseMatrix.rotate(m_parentNode->getActivePoseWorldRotation());
                m_activePoseMatrix.translate(Matrix4::Dot(m_parentNode->getActivePoseMatrix(), m_localTranslation));
            } else {
                m_activePoseMatrix.translate(m_localTranslation);
            }
        } else {
            
            // WorldTransform = ParentWorldTransform * T * Roff * Rp * Rpre * R * Rpost * Rp-1 * Soff * Sp * S * Sp-1
            m_activePoseMatrix = Matrix4::Translation(-m_scalingPivot)
            * Matrix4::Scaling(m_localScale)
            * Matrix4::Translation(m_scalingPivot)
            * Matrix4::Translation(m_scalingOffset)
            * Matrix4::Translation(-m_rotationPivot)
            * Matrix4::Rotation(m_postRotation)
            * Matrix4::Rotation(m_localRotation)
            * Matrix4::Rotation(m_preRotation)
            * Matrix4::Translation(m_rotationPivot)
            * Matrix4::Translation(m_rotationOffset)
            * Matrix4::Translation(m_localTranslation);
            
            
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

const Matrix4 &KRNode::getInverseModelMatrix()
{
    if(!m_inverseModelMatrixValid) {
        m_inverseModelMatrix = Matrix4::Invert(getModelMatrix());
    }
    return m_inverseModelMatrix;
}

const Matrix4 &KRNode::getInverseBindPoseMatrix()
{
    if(!m_inverseBindPoseMatrixValid ) {
        m_inverseBindPoseMatrix = Matrix4::Invert(getBindPoseMatrix());
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
            setLocalTranslation(Vector3(v, m_localTranslation.y, m_localTranslation.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_TRANSLATE_Y:
            setLocalTranslation(Vector3(m_localTranslation.x, v, m_localTranslation.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_TRANSLATE_Z:
            setLocalTranslation(Vector3(m_localTranslation.x, m_localTranslation.y, v));
            break;
        case KRENGINE_NODE_ATTRIBUTE_SCALE_X:
            setLocalScale(Vector3(v, m_localScale.y, m_localScale.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_SCALE_Y:
            setLocalScale(Vector3(m_localScale.x, v, m_localScale.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_SCALE_Z:
            setLocalScale(Vector3(m_localScale.x, m_localScale.y, v));
            break;
        case KRENGINE_NODE_ATTRIBUTE_ROTATE_X:
            setLocalRotation(Vector3(v * DEGREES_TO_RAD, m_localRotation.y, m_localRotation.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_ROTATE_Y:
            setLocalRotation(Vector3(m_localRotation.x, v * DEGREES_TO_RAD, m_localRotation.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_ROTATE_Z:
            setLocalRotation(Vector3(m_localRotation.x, m_localRotation.y, v * DEGREES_TO_RAD));
            break;
            

        case KRENGINE_NODE_ATTRIBUTE_PRE_ROTATION_X:
            setPreRotation(Vector3(v * DEGREES_TO_RAD, m_preRotation.y, m_preRotation.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_PRE_ROTATION_Y:
            setPreRotation(Vector3(m_preRotation.x, v * DEGREES_TO_RAD, m_preRotation.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_PRE_ROTATION_Z:
            setPreRotation(Vector3(m_preRotation.x, m_preRotation.y, v * DEGREES_TO_RAD));
            break;
        case KRENGINE_NODE_ATTRIBUTE_POST_ROTATION_X:
            setPostRotation(Vector3(v * DEGREES_TO_RAD, m_postRotation.y, m_postRotation.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_POST_ROTATION_Y:
            setPostRotation(Vector3(m_postRotation.x, v * DEGREES_TO_RAD, m_postRotation.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_POST_ROTATION_Z:
            setPostRotation(Vector3(m_postRotation.x, m_postRotation.y, v * DEGREES_TO_RAD));
            break;
        case KRENGINE_NODE_ATTRIBUTE_ROTATION_PIVOT_X:
            setRotationPivot(Vector3(v, m_rotationPivot.y, m_rotationPivot.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_ROTATION_PIVOT_Y:
            setRotationPivot(Vector3(m_rotationPivot.x, v, m_rotationPivot.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_ROTATION_PIVOT_Z:
            setRotationPivot(Vector3(m_rotationPivot.x, m_rotationPivot.y, v));
            break;
        case KRENGINE_NODE_ATTRIBUTE_SCALE_PIVOT_X:
            setScalingPivot(Vector3(v, m_scalingPivot.y, m_scalingPivot.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_SCALE_PIVOT_Y:
            setScalingPivot(Vector3(m_scalingPivot.x, v, m_scalingPivot.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_SCALE_PIVOT_Z:
            setScalingPivot(Vector3(m_scalingPivot.x, m_scalingPivot.y, v));
            break;
        case KRENGINE_NODE_ATTRIBUTE_ROTATE_OFFSET_X:
            setRotationOffset(Vector3(v, m_rotationOffset.y, m_rotationOffset.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_ROTATE_OFFSET_Y:
            setRotationOffset(Vector3(m_rotationOffset.x, v, m_rotationOffset.z));
            break;
        case KRENGINE_NODE_ATTRIBUTE_ROTATE_OFFSET_Z:
            setRotationOffset(Vector3(m_rotationOffset.x, m_rotationOffset.y, v));
            break;
        case KRENGINE_NODE_SCALE_OFFSET_X:
            setScalingOffset(Vector3(v, m_scalingOffset.y, m_scalingOffset.z));
            break;
        case KRENGINE_NODE_SCALE_OFFSET_Y:
            setScalingOffset(Vector3(m_scalingOffset.x, v, m_scalingOffset.z));
            break;
        case KRENGINE_NODE_SCALE_OFFSET_Z:
            setScalingOffset(Vector3(m_scalingOffset.x, m_scalingOffset.y, v));
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
    if(m_lod_visible >= LOD_VISIBILITY_PRESTREAM) {
        for(std::set<KRNode *>::iterator itr=m_childNodes.begin(); itr != m_childNodes.end(); ++itr) {
            (*itr)->updateLODVisibility(viewport);
        }
    }
}

void KRNode::setLODVisibility(KRNode::LodVisibility lod_visibility)
{
    if(m_lod_visible != lod_visibility) {
        if(m_lod_visible == LOD_VISIBILITY_HIDDEN && lod_visibility >= LOD_VISIBILITY_PRESTREAM) {
            getScene().notify_sceneGraphCreate(this);
        } else if(m_lod_visible >= LOD_VISIBILITY_PRESTREAM && lod_visibility == LOD_VISIBILITY_HIDDEN) {
            getScene().notify_sceneGraphDelete(this);
        }
        
        m_lod_visible = lod_visibility;
        
        for(std::set<KRNode *>::iterator itr=m_childNodes.begin(); itr != m_childNodes.end(); ++itr) {
            (*itr)->setLODVisibility(lod_visibility);
        }
    }
}

KRNode::LodVisibility KRNode::getLODVisibility()
{
    return m_lod_visible;
}

const Vector3 KRNode::localToWorld(const Vector3 &local_point)
{
    return Matrix4::Dot(getModelMatrix(), local_point);
}

const Vector3 KRNode::worldToLocal(const Vector3 &world_point)
{
    return Matrix4::Dot(getInverseModelMatrix(), world_point);
}

void KRNode::addBehavior(KRBehavior *behavior)
{
    m_behaviors.insert(behavior);
    behavior->__setNode(this);
    getScene().notify_sceneGraphModify(this);
}

std::set<KRBehavior *> &KRNode::getBehaviors()
{
    return m_behaviors;
}

kraken_stream_level KRNode::getStreamLevel(const KRViewport &viewport)
{
    kraken_stream_level stream_level = kraken_stream_level::STREAM_LEVEL_IN_HQ;
    
    for(std::set<KRNode *>::iterator itr=m_childNodes.begin(); itr != m_childNodes.end(); ++itr) {
        stream_level = KRMIN(stream_level, (*itr)->getStreamLevel(viewport));
    }
    
    return stream_level;
}

void KRNode::invalidateBounds() const
{
    m_boundsValid = false;
    if(m_parentNode) {
        m_parentNode->invalidateBounds();
    }
}
