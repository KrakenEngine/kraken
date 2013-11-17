//
//  KRLocator
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-12-06.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KRLOCATOR_H
#define KRLOCATOR_H

#include "KRResource.h"
#include "KRNode.h"
#include "KRTexture.h"

class KRLocator : public KRNode {
public:
    KRLocator(KRScene &scene, std::string name);
    virtual ~KRLocator();
    virtual std::string getElementName();
    virtual tinyxml2::XMLElement *saveXML( tinyxml2::XMLNode *parent);
    virtual void loadXML(tinyxml2::XMLElement *e);

};


#endif
