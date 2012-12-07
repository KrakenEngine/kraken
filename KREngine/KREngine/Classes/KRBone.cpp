//
//  KRBone.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-12-06.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KRBone.h"

KRBone::KRBone(KRScene &scene, std::string name) : KRNode(scene, name)
{
}

KRBone::~KRBone()
{
    
}

std::string KRBone::getElementName() {
    return "bone";
}

tinyxml2::XMLElement *KRBone::saveXML( tinyxml2::XMLNode *parent)
{
    tinyxml2::XMLElement *e = KRNode::saveXML(parent);

    return e;
}

void KRBone::loadXML(tinyxml2::XMLElement *e)
{
    KRNode::loadXML(e);
}
