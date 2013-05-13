//
//  KRNode.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-11.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KRNODE_H
#define KRNODE_H

#include "KRResource.h"
#include "KRVector3.h"
#include "KRViewport.h"
#include "tinyxml2.h"
#include "KROctreeNode.h"

class KRCamera;
class KRShaderManager;
class KRMeshManager;
class KRMaterialManager;
class KRMat4;
class KRTextureManager;
class KRContext;
class KRScene;
class KRAABB;
class KRNode;
class KRPointLight;
class KRSpotLight;
class KRDirectionalLight;

class KRNode : public KRContextObject
{
public:
    enum RenderPass {
        RENDER_PASS_FORWARD_OPAQUE,
        RENDER_PASS_DEFERRED_GBUFFER,
        RENDER_PASS_DEFERRED_LIGHTS,
        RENDER_PASS_DEFERRED_OPAQUE,
        RENDER_PASS_FORWARD_TRANSPARENT,
        RENDER_PASS_PARTICLE_OCCLUSION,
        RENDER_PASS_ADDITIVE_PARTICLES,
        RENDER_PASS_VOLUMETRIC_EFFECTS_ADDITIVE,
        RENDER_PASS_GENERATE_SHADOWMAPS,
        RENDER_PASS_SHADOWMAP
    };
    
    KRNode(KRScene &scene, std::string name);
    virtual ~KRNode();
    
    virtual tinyxml2::XMLElement *saveXML( tinyxml2::XMLNode *parent);
    static KRNode *LoadXML(KRScene &scene, tinyxml2::XMLElement *e);
    virtual void loadXML(tinyxml2::XMLElement *e);
    
    virtual std::string getElementName();
    const std::string &getName() const;
    
    void addChild(KRNode *child);
    const std::set<KRNode *> &getChildren();
    
    void setLocalTranslation(const KRVector3 &v, bool set_original = false);
    void setLocalScale(const KRVector3 &v, bool set_original = false);
    void setLocalRotation(const KRVector3 &v, bool set_original = false);
    
    
    void setRotationOffset(const KRVector3 &v, bool set_original = false);
    void setScalingOffset(const KRVector3 &v, bool set_original = false);
    void setRotationPivot(const KRVector3 &v, bool set_original = false);
    void setScalingPivot(const KRVector3 &v, bool set_original = false);
    void setPreRotation(const KRVector3 &v, bool set_original = false);
    void setPostRotation(const KRVector3 &v, bool set_original = false);
    
    const KRVector3 &getRotationOffset();
    const KRVector3 &getScalingOffset();
    const KRVector3 &getRotationPivot();
    const KRVector3 &getScalingPivot();
    const KRVector3 &getPreRotation();
    const KRVector3 &getPostRotation();
    
    const KRVector3 &getInitialRotationOffset();
    const KRVector3 &getInitialScalingOffset();
    const KRVector3 &getInitialRotationPivot();
    const KRVector3 &getInitialScalingPivot();
    const KRVector3 &getInitialPreRotation();
    const KRVector3 &getInitialPostRotation();
    
    
    const KRVector3 &getLocalTranslation();
    const KRVector3 &getLocalScale();
    const KRVector3 &getLocalRotation();
    
    const KRVector3 &getInitialLocalTranslation();
    const KRVector3 &getInitialLocalScale();
    const KRVector3 &getInitialLocalRotation();
    
    const KRVector3 getWorldTranslation();
    const KRVector3 getWorldScale();
    const KRVector3 getWorldRotation();
    
    const KRVector3 getBindPoseWorldRotation();
    const KRVector3 getActivePoseWorldRotation();
    
    const KRVector3 localToWorld(const KRVector3 &local_point);
    const KRVector3 worldToLocal(const KRVector3 &world_point);
    
    void setWorldTranslation(const KRVector3 &v);
    void setWorldScale(const KRVector3 &v);
    void setWorldRotation(const KRVector3 &v);
    
    virtual KRAABB getBounds();
    const KRMat4 &getModelMatrix();
    const KRMat4 &getInverseModelMatrix();
    const KRMat4 &getBindPoseMatrix();
    const KRMat4 &getActivePoseMatrix();
    const KRMat4 &getInverseBindPoseMatrix();
    
    enum node_attribute_type {
        KRENGINE_NODE_ATTRIBUTE_NONE,
        KRENGINE_NODE_ATTRIBUTE_TRANSLATE_X,
        KRENGINE_NODE_ATTRIBUTE_TRANSLATE_Y,
        KRENGINE_NODE_ATTRIBUTE_TRANSLATE_Z,
        KRENGINE_NODE_ATTRIBUTE_SCALE_X,
        KRENGINE_NODE_ATTRIBUTE_SCALE_Y,
        KRENGINE_NODE_ATTRIBUTE_SCALE_Z,
        KRENGINE_NODE_ATTRIBUTE_ROTATE_X,
        KRENGINE_NODE_ATTRIBUTE_ROTATE_Y,
        KRENGINE_NODE_ATTRIBUTE_ROTATE_Z
    };
    
    void SetAttribute(node_attribute_type attrib, float v);
    
    KRScene &getScene();
    
    virtual void render(KRCamera *pCamera, std::vector<KRPointLight *> &point_lights, std::vector<KRDirectionalLight *> &directional_lights, std::vector<KRSpotLight *>&spot_lights, const KRViewport &viewport, RenderPass renderPass);
    
    virtual void physicsUpdate(float deltaTime);
    virtual bool hasPhysics();
    
    virtual void updateLODVisibility(const KRViewport &viewport);
    bool lodIsVisible();
    
    void setScaleCompensation(bool scale_compensation);
    bool getScaleCompensation();
    
protected:
    KRVector3 m_localTranslation;
    KRVector3 m_localScale;
    KRVector3 m_localRotation;
    
    KRVector3 m_rotationOffset;
    KRVector3 m_scalingOffset;
    KRVector3 m_rotationPivot;
    KRVector3 m_scalingPivot;
    KRVector3 m_preRotation;
    KRVector3 m_postRotation;
    
    KRVector3 m_initialLocalTranslation;
    KRVector3 m_initialLocalScale;
    KRVector3 m_initialLocalRotation;
    
    KRVector3 m_initialRotationOffset;
    KRVector3 m_initialScalingOffset;
    KRVector3 m_initialRotationPivot;
    KRVector3 m_initialScalingPivot;
    KRVector3 m_initialPreRotation;
    KRVector3 m_initialPostRotation;
    
    bool m_lod_visible;
    void hideLOD();
    void showLOD();
    float m_lod_min_coverage;
    float m_lod_max_coverage;
    
    KRNode *m_parentNode;
    std::set<KRNode *> m_childNodes;
    
private:
    void invalidateModelMatrix();
    void invalidateBindPoseMatrix();
    KRMat4 m_modelMatrix;
    KRMat4 m_inverseModelMatrix;
    KRMat4 m_bindPoseMatrix;
    KRMat4 m_activePoseMatrix;
    KRMat4 m_inverseBindPoseMatrix;
    bool m_modelMatrixValid;
    bool m_inverseModelMatrixValid;
    bool m_bindPoseMatrixValid;
    bool m_activePoseMatrixValid;
    bool m_inverseBindPoseMatrixValid;
    
    std::string m_name;
    
    
    
    KRScene *m_pScene;
    
    std::set<KROctreeNode *> m_octree_nodes;
    bool m_scale_compensation;
    
public:
    
    void removeFromOctreeNodes();
    void addToOctreeNode(KROctreeNode *octree_node);
    void childDeleted(KRNode *child_node);
    
    template <class T> T *find()
    {
        T *match = dynamic_cast<T *>(this);
        if(match) {
            return match;
        }
        
        for(std::set<KRNode *>::const_iterator itr=m_childNodes.begin(); itr != m_childNodes.end(); ++itr) {
            match = (*itr)->find<T>();
            if(match) {
                return match;
            }
        }
        
        return NULL;
    }
    
    template <class T> T *find(const std::string &name)
    {
        T *match = dynamic_cast<T *>(this);
        if(match) {
            if(name.compare(match->getName()) == 0) {
                return match;
            }
        }
        
        for(std::set<KRNode *>::const_iterator itr=m_childNodes.begin(); itr != m_childNodes.end(); ++itr) {
            match = (*itr)->find<T>(name);
            if(match) {
                return match;
            }
        }
        
        return NULL;
    }
};


#endif
