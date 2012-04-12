//
//  KRSpotLight.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-05.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KREngine_KRSpotLight_h
#define KREngine_KRSpotLight_h

#import "KRLight.h"

class KRSpotLight : public KRLight {
public:
    KRSpotLight(std::string name);
    virtual ~KRSpotLight();
    
    virtual std::string getElementName();
    virtual tinyxml2::XMLElement *saveXML( tinyxml2::XMLNode *parent);
    virtual void loadXML(tinyxml2::XMLElement *e);
    
    
    float getInnerAngle();
    float getOuterAngle();
    void setInnerAngle(float innerAngle);
    void setOuterAngle(float outerAngle);
    
private:
    float m_innerAngle; // Inner angle of the cone, in radians.  Inside this radius, the light will be at full brightness
    float m_outerAngle; // Outer angle of the cone, in radians.  Outside this radius, the light will be completely attenuated
};


#endif
