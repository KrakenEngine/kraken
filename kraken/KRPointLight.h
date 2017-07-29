//
//  KRPointLight.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-05.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KRPOINTLIGHT_H
#define KRPOINTLIGHT_H

#include "KRLight.h"

class KRPointLight : public KRLight {
    
public:
    
    KRPointLight(KRScene &scene, std::string name);
    virtual ~KRPointLight();
    
    virtual std::string getElementName();
    virtual KRAABB getBounds();

    virtual void render(KRCamera *pCamera, std::vector<KRPointLight *> &point_lights, std::vector<KRDirectionalLight *> &directional_lights, std::vector<KRSpotLight *>&spot_lights, const KRViewport &viewport, KRNode::RenderPass renderPass);
    
private:
    void generateMesh();
    
    GLfloat *m_sphereVertices;
    int m_cVertices;
};

#endif
