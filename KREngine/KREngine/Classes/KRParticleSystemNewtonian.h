//
//  KRParticleSystemNewtonian.h
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-11-02.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KRPARTICLESYSTEMNEWTONIAN_H
#define KRPARTICLESYSTEMNEWTONIAN_H

#import "KRParticleSystem.h"

class KRParticleSystemNewtonian : public KRParticleSystem {
public:
    KRParticleSystemNewtonian(KRScene &scene, std::string name);
    virtual ~KRParticleSystemNewtonian();
    
    virtual std::string getElementName();
    virtual void loadXML(tinyxml2::XMLElement *e);
    virtual tinyxml2::XMLElement *saveXML( tinyxml2::XMLNode *parent);
    
    
    virtual KRAABB getBounds();
    
    virtual void render(KRCamera *pCamera, std::vector<KRLight *> &lights, const KRViewport &viewport, KRNode::RenderPass renderPass);
    
    
    virtual void physicsUpdate(float deltaTime);
    virtual bool hasPhysics();
private:
    float m_particlesAbsoluteTime;
};

#endif
