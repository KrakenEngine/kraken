//
//  KRLight.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-05.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KREngine_KRLight_h
#define KREngine_KRLight_h

#import "KRResource.h"
#import "KRNode.h"

class KRLight : public KRNode {
public:
    static const float KRLIGHT_MIN_INFLUENCE = 0.05f;
    
    virtual ~KRLight();
    virtual std::string getElementName() = 0;
    virtual tinyxml2::XMLElement *saveXML( tinyxml2::XMLNode *parent);
    virtual void loadXML(tinyxml2::XMLElement *e);
    
    void setIntensity(float intensity);
    float getIntensity();
    void setDecayStart(float decayStart);
    float getDecayStart();
    const KRVector3 &getColor();
    void setColor(const KRVector3 &color);
    
protected:
    KRLight(std::string name);
    
    float m_intensity;
    float m_decayStart;
    KRVector3 m_color;
};

#endif
