//
//  KRAmbientZone.h
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-12-06.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KRAMBIENT_ZONE_H
#define KRAMBIENT_ZONE_H

#include "KRResource.h"
#include "KRNode.h"
#include "KRTexture.h"

class KRAmbientZone : public KRNode {
public:
    KRAmbientZone(KRScene &scene, std::string name);
    virtual ~KRAmbientZone();
    virtual std::string getElementName();
    virtual tinyxml2::XMLElement *saveXML( tinyxml2::XMLNode *parent);
    virtual void loadXML(tinyxml2::XMLElement *e);
    
    void render(KRCamera *pCamera, std::vector<KRPointLight *> &point_lights, std::vector<KRDirectionalLight *> &directional_lights, std::vector<KRSpotLight *>&spot_lights, const KRViewport &viewport, KRNode::RenderPass renderPass);
    
    std::string getZone();
    void setZone(const std::string &zone);
    
    float getGradientDistance();
    void setGradientDistance(float gradient_distance);
    
    std::string getAmbient();
    void setAmbient(const std::string &ambient);
    
    float getAmbientGain();
    void setAmbientGain(float ambient_gain);
    
    virtual KRAABB getBounds();
    
    float getContainment(const Vector3 &pos);
    
private:
    std::string m_zone;
    
    float m_gradient_distance;
    
    std::string m_ambient;
    float m_ambient_gain;
};


#endif
