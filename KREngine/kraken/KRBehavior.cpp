//
//  KRBehavior.cpp
//  Kraken
//
//  Created by Kearwood Gilbert on 2013-05-17.
//  Copyright (c) 2013 Kearwood Software. All rights reserved.
//

#include "KRBehavior.h"
#include "KRNode.h"

KRBehaviorFactoryFunctionMap m_factoryFunctions;

KRBehavior::KRBehavior()
{
    __node = NULL;
}

KRBehavior::~KRBehavior()
{
    
}

void KRBehavior::init()
{
    // Note: Subclasses are not expected to call this method
}

KRNode *KRBehavior::getNode() const
{
    return __node;
}

void KRBehavior::__setNode(KRNode *node)
{
    __node = node;
}


KRBehavior *KRBehavior::LoadXML(KRNode *node, tinyxml2::XMLElement *e)
{
    std::map<std::string, std::string> attributes;
    for(const tinyxml2::XMLAttribute *attribute = e->FirstAttribute(); attribute != NULL; attribute = attribute->Next()) {
        attributes[attribute->Name()] = attribute->Value();
    }
    
    const char *szElementName = e->Attribute("type");
    if(szElementName == NULL) {
        return NULL;
    }
    KRBehaviorFactoryFunctionMap::const_iterator itr = m_factoryFunctions.find(szElementName);
    if(itr == m_factoryFunctions.end()) {
        return NULL;
    }
    return (*itr->second)(attributes);
}

void KRBehavior::RegisterFactoryCTOR(std::string behaviorName, KRBehaviorFactoryFunction fnFactory)
{
    m_factoryFunctions[behaviorName] = fnFactory;
}

void KRBehavior::UnregisterFactoryCTOR(std::string behaviorName)
{
    m_factoryFunctions.erase(behaviorName);
}
