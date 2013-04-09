//
//  KRDirectionalLight.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-05.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KREngine_KRDirectionalLight_h
#define KREngine_KRDirectionalLight_h

#include "KRLight.h"
#include "KRMat4.h"

class KRDirectionalLight : public KRLight {
    
public:
    
    KRDirectionalLight(KRScene &scene, std::string name);
    virtual ~KRDirectionalLight();
    
    virtual std::string getElementName();
    KRVector3 getLocalLightDirection();
    KRVector3 getWorldLightDirection();

    virtual void render(KRCamera *pCamera, std::vector<KRLight *> &lights, const KRViewport &viewport, KRNode::RenderPass renderPass);
    virtual KRAABB getBounds();
    
protected:
    
    virtual int configureShadowBufferViewports(const KRViewport &viewport);
    
};


#endif
