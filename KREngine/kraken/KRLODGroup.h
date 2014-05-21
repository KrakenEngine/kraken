//
//  KRLODGroup.h
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-12-06.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KRLODGROUP_H
#define KRLODGROUP_H

#include "KRResource.h"
#include "KRNode.h"

class KRLODGroup : public KRNode {
public:
    KRLODGroup(KRScene &scene, std::string name);
    virtual ~KRLODGroup();
    virtual std::string getElementName();
    virtual tinyxml2::XMLElement *saveXML( tinyxml2::XMLNode *parent);
    virtual void loadXML(tinyxml2::XMLElement *e);
    
    float getMinDistance();
    float getMaxDistance();
    void setMinDistance(float min_distance);
    void setMaxDistance(float max_distance);
    
    const KRAABB &getReference() const;
    void setReference(const KRAABB &reference);
    void setUseWorldUnits(bool use_world_units);
    bool getUseWorldUnits() const;
    
    LodVisibility calcLODVisibility(const KRViewport &viewport);
    
private:
    float m_min_distance;
    float m_max_distance;
    KRAABB m_reference; // Point of reference, used for distance calculation.  Usually set to the bounding box center
    bool m_use_world_units;
};


#endif
