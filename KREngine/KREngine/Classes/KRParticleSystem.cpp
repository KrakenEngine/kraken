//
//  KRParticleSystem.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-11-02.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KRParticleSystem.h"

KRParticleSystem::KRParticleSystem(KRScene &scene, std::string name) : KRNode(scene, name)
{
    
}

KRParticleSystem::~KRParticleSystem()
{
    
}

void KRParticleSystem::loadXML(tinyxml2::XMLElement *e)
{
    
}

tinyxml2::XMLElement *KRParticleSystem::saveXML( tinyxml2::XMLNode *parent)
{
    tinyxml2::XMLElement *e = KRNode::saveXML(parent);
    return e;
}

