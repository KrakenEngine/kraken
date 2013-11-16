//
//  KRLODSet.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-12-06.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KRLODSet.h"
#include "KRContext.h"

KRLODSet::KRLODSet(KRScene &scene, std::string name) : KRNode(scene, name)
{

}

KRLODSet::~KRLODSet()
{
}

std::string KRLODSet::getElementName() {
    return "lod_set";
}

tinyxml2::XMLElement *KRLODSet::saveXML( tinyxml2::XMLNode *parent)
{
    tinyxml2::XMLElement *e = KRNode::saveXML(parent);
    
    return e;
}

void KRLODSet::loadXML(tinyxml2::XMLElement *e)
{
    KRNode::loadXML(e);
}
