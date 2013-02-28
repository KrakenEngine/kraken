//
//  KRAmbientSphere.h
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

class KRAmbientSphere : public KRNode {
public:
    KRAmbientSphere(KRScene &scene, std::string name);
    virtual ~KRAmbientSphere();
    virtual std::string getElementName();
    virtual tinyxml2::XMLElement *saveXML( tinyxml2::XMLNode *parent);
    virtual void loadXML(tinyxml2::XMLElement *e);
    
    void render(KRCamera *pCamera, std::vector<KRLight *> &lights, const KRViewport &viewport, KRNode::RenderPass renderPass);
    
    std::string getZone();
    void setZone(const std::string &zone);
    
    float getGradientDistance();
    void setGradientDistance(float gradient_distance);
    
    std::string getReverb();
    void setReverb(const std::string &reverb);
    
    float getReverbGain();
    void setReverbGain(float reverb_gain);
    
    std::string getAmbient();
    void setAmbient(const std::string &ambient);
    
    float getAmbientGain();
    void setAmbientGain(float ambient_gain);
    
    virtual KRAABB getBounds();
    
private:
    std::string m_zone;
    
    float m_gradient_distance;
    
    std::string m_reverb;
    float m_reverb_gain;
    
    std::string m_ambient;
    float m_ambient_gain;
};


#endif
