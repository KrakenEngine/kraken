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

class KRDirectionalLight : public KRLight {
    
public:
    
    KRDirectionalLight(KRScene &scene, std::string name);
    virtual ~KRDirectionalLight();
    
    virtual std::string getElementName();
    Vector3 getLocalLightDirection();
    Vector3 getWorldLightDirection();

    virtual void render(KRCamera *pCamera, std::vector<KRPointLight *> &point_lights, std::vector<KRDirectionalLight *> &directional_lights, std::vector<KRSpotLight *>&spot_lights, const KRViewport &viewport, KRNode::RenderPass renderPass);
    virtual AABB getBounds();
    
protected:
    
    virtual int configureShadowBufferViewports(const KRViewport &viewport);
    
};


#endif
