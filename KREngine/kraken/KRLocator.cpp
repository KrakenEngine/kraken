//
//  KRLocator.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-12-06.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KRLocator.h"
#include "KRContext.h"

KRLocator::KRLocator(KRScene &scene, std::string name) : KRNode(scene, name)
{
 
}

KRLocator::~KRLocator()
{
}

std::string KRLocator::getElementName() {
    return "locator";
}

tinyxml2::XMLElement *KRLocator::saveXML( tinyxml2::XMLNode *parent)
{
    tinyxml2::XMLElement *e = KRNode::saveXML(parent);
    
    return e;
}

void KRLocator::loadXML(tinyxml2::XMLElement *e)
{
    KRNode::loadXML(e);
}
