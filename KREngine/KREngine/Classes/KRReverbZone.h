//
//  KRReverbZone.h
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-12-06.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KRREVERB_ZONE_H
#define KRREVERB_ZONE_H

#import "KRResource.h"
#import "KRNode.h"
#import "KRTexture.h"

class KRReverbZone : public KRNode {
public:
    KRReverbZone(KRScene &scene, std::string name);
    virtual ~KRReverbZone();
    virtual std::string getElementName();
    virtual tinyxml2::XMLElement *saveXML( tinyxml2::XMLNode *parent);
    virtual void loadXML(tinyxml2::XMLElement *e);
    
    void render(KRCamera *pCamera, std::vector<KRLight *> &lights, const KRViewport &viewport, KRNode::RenderPass renderPass);
    
    float getGradientDistance();
    void setGradientDistance(float gradient_distance);
    
    std::string getReverbPreset();
    void setReverbPreset(const std::string &reverb_preset_name);
    
    unsigned int getReverbSettingId();
    
    virtual KRAABB getBounds();
    
private:
    float m_gradient_distance;
    std::string m_reverb_preset_name;
};


#endif
