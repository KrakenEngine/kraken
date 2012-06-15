//
//  KRNode.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-11.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KREngine_KRNode_h
#define KREngine_KRNode_h

#import "KRResource.h"
#import "KRVector3.h"
#import "tinyxml2.h"

class KRBoundingVolume;
class KRCamera;
class KRShaderManager;
class KRModelManager;
class KRMaterialManager;
class KRMat4;
class KRTextureManager;
class KRContext;

class KRNode
{
public:
    enum RenderPass {
        RENDER_PASS_FORWARD_OPAQUE,
        RENDER_PASS_DEFERRED_GBUFFER,
        RENDER_PASS_DEFERRED_LIGHTS,
        RENDER_PASS_DEFERRED_OPAQUE,
        RENDER_PASS_FORWARD_TRANSPARENT,
        RENDER_PASS_FLARES,
        RENDER_PASS_SHADOWMAP
    };
    
    KRNode(std::string name);
    virtual ~KRNode();
    
    virtual tinyxml2::XMLElement *saveXML( tinyxml2::XMLNode *parent);
    static KRNode *LoadXML(tinyxml2::XMLElement *e);
    virtual void loadXML(tinyxml2::XMLElement *e);
    
    virtual std::string getElementName();
    const std::string &getName();
    
    void addChild(KRNode *child);
    const std::vector<KRNode *> &getChildren();
    
    void setLocalTranslation(const KRVector3 &v);
    void setLocalScale(const KRVector3 &v);
    void setLocalRotation(const KRVector3 &v);
    
    const KRVector3 &getLocalTranslation();
    const KRVector3 &getLocalScale();
    const KRVector3 &getLocalRotation();
    
    const KRVector3 &getWorldTranslation();
    const KRVector3 &getWorldScale();
    const KRVector3 &getWorldRotation();
    
    void clearExtents();
    virtual void calcExtents(KRContext *Context);
    KRBoundingVolume getExtents(KRContext *pContext);    
#if TARGET_OS_IPHONE
    
    virtual void render(KRCamera *pCamera, KRContext *pContext, KRBoundingVolume &frustrumVolume, KRMat4 &viewMatrix, KRVector3 &cameraPosition, KRVector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers, RenderPass renderPass);

#endif
    
protected:
    KRBoundingVolume *m_pExtents;
    
private:
    KRVector3 m_localTranslation;
    KRVector3 m_localScale;
    KRVector3 m_localRotation;
    
    std::string m_name;
    
    std::vector<KRNode *> m_childNodes;
    KRNode *m_parentNode;
    
};


#endif
