//
//  KRParticleSystem.h
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-11-02.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KRPARTICLESYSTEM_H
#define KRPARTICLESYSTEM_H

#import "KRNode.h"

class KRParticleSystem : public KRNode {
public:
    virtual ~KRParticleSystem();
    
    virtual std::string getElementName() = 0;
    virtual void loadXML(tinyxml2::XMLElement *e);
    virtual tinyxml2::XMLElement *saveXML( tinyxml2::XMLNode *parent);
    
    virtual KRAABB getBounds() = 0;
    
#if TARGET_OS_IPHONE
    
    virtual void render(KRCamera *pCamera, KRContext *pContext, const KRViewport &viewport, const KRViewport *pShadowViewports, KRVector3 &lightDirection, GLuint *shadowDepthTextures, int cShadowBuffers, KRNode::RenderPass renderPass) = 0;
#endif
    
protected:
    KRParticleSystem(KRScene &scene, std::string name);
private:
    
};

#endif
