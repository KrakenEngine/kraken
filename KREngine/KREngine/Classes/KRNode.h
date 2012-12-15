//
//  KRNode.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-11.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KRNODE_H
#define KRNODE_H

#import "KRResource.h"
#import "KRVector3.h"
#import "KRViewport.h"
#import "tinyxml2.h"
#import <stack.h>

class KRCamera;
class KRShaderManager;
class KRModelManager;
class KRMaterialManager;
class KRMat4;
class KRTextureManager;
class KRContext;
class KRScene;
class KRAABB;

class KRNode : public KRContextObject
{
public:
    enum RenderPass {
        RENDER_PASS_FORWARD_OPAQUE,
        RENDER_PASS_DEFERRED_GBUFFER,
        RENDER_PASS_DEFERRED_LIGHTS,
        RENDER_PASS_DEFERRED_OPAQUE,
        RENDER_PASS_FORWARD_TRANSPARENT,
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
    const std::string &getName();
    
    void addChild(KRNode *child);
    const std::vector<KRNode *> &getChildren();
    
    KRNode *findChild(const std::string &name);
    
    void setLocalTranslation(const KRVector3 &v, bool set_original = false);
    void setLocalScale(const KRVector3 &v, bool set_original = false);
    void setLocalRotation(const KRVector3 &v, bool set_original = false);
    
    const KRVector3 &getLocalTranslation();
    const KRVector3 &getLocalScale();
    const KRVector3 &getLocalRotation();
    
    const KRVector3 &getInitialLocalTranslation();
    const KRVector3 &getInitialLocalScale();
    const KRVector3 &getInitialLocalRotation();
    
    const KRVector3 &getWorldTranslation();
    const KRVector3 &getWorldScale();
    const KRVector3 &getWorldRotation();
    
    virtual KRAABB getBounds();
    const KRMat4 &getModelMatrix();
    const KRMat4 &getInverseModelMatrix();
    const KRMat4 &getBindPoseMatrix();
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
#if TARGET_OS_IPHONE
    
    virtual void render(KRCamera *pCamera, std::vector<KRLight *> &lights, const KRViewport &viewport, RenderPass renderPass);

#endif
    
    virtual void physicsUpdate(float deltaTime);
    virtual bool hasPhysics();
    
protected:
    KRVector3 m_localTranslation;
    KRVector3 m_localScale;
    KRVector3 m_localRotation;
    
    KRVector3 m_initialLocalTranslation;
    KRVector3 m_initialLocalScale;
    KRVector3 m_initialLocalRotation;
    
private:
    void invalidateModelMatrix();
    void invalidateBindPoseMatrix();
    KRMat4 m_modelMatrix;
    KRMat4 m_inverseModelMatrix;
    KRMat4 m_bindPoseMatrix;
    KRMat4 m_inverseBindPoseMatrix;
    bool m_modelMatrixValid;
    bool m_inverseModelMatrixValid;
    bool m_bindPoseMatrixValid;
    bool m_inverseBindPoseMatrixValid;
    
    std::string m_name;
    
    std::vector<KRNode *> m_childNodes;
    KRNode *m_parentNode;
    
    KRScene *m_pScene;
    
};


#endif
