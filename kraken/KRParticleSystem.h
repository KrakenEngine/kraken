//
//  KRParticleSystem.h
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-11-02.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KRPARTICLESYSTEM_H
#define KRPARTICLESYSTEM_H

#include "KRNode.h"

class KRParticleSystem : public KRNode {
public:
    virtual ~KRParticleSystem();
    
    virtual std::string getElementName() = 0;
    virtual void loadXML(tinyxml2::XMLElement *e);
    virtual tinyxml2::XMLElement *saveXML( tinyxml2::XMLNode *parent);
    
    virtual AABB getBounds() = 0;
    
    virtual void render(KRCamera *pCamera, std::vector<KRPointLight *> &point_lights, std::vector<KRDirectionalLight *> &directional_lights, std::vector<KRSpotLight *>&spot_lights, const KRViewport &viewport, KRNode::RenderPass renderPass) = 0;
    
protected:
    KRParticleSystem(KRScene &scene, std::string name);
private:
    
};

#endif
