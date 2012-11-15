//
//  KRParticleSystemBrownian.h
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-11-02.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KRPARTICLESYSTEMBROWNIAN_H
#define KRPARTICLESYSTEMBROWNIAN_H

#import "KRParticleSystem.h"

class KRParticleSystemBrownian : public KRParticleSystem {
public:
    KRParticleSystemBrownian(KRScene &scene, std::string name);
    virtual ~KRParticleSystemBrownian();
    
    virtual std::string getElementName();
    virtual void loadXML(tinyxml2::XMLElement *e);
    virtual tinyxml2::XMLElement *saveXML( tinyxml2::XMLNode *parent);
    
    
    virtual KRAABB getBounds();
    
    
#if TARGET_OS_IPHONE
    
    virtual void render(KRCamera *pCamera, std::stack<KRLight *> &lights, const KRViewport &viewport, KRNode::RenderPass renderPass);
#endif
    
    
    virtual void physicsUpdate(float deltaTime);
private:
    float m_particlesAbsoluteTime;
};

#endif
