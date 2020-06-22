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
    static void InitNodeInfo(KrNodeInfo* nodeInfo);

    KRLocator(KRScene &scene, std::string name);
    virtual ~KRLocator();
    virtual std::string getElementName();
    virtual tinyxml2::XMLElement *saveXML( tinyxml2::XMLNode *parent);
    virtual void loadXML(tinyxml2::XMLElement *e);
    unordered_map<std::string, int> &getUserIntAttributes();
    unordered_map<std::string, double> &getUserDoubleAttributes();
    unordered_map<std::string, bool> &getUserBoolAttributes();
    unordered_map<std::string, std::string> &getUserStringAttributes();
    
private:
    unordered_map<std::string, int> m_userIntAttributes;
    unordered_map<std::string, double> m_userDoubleAttributes;
    unordered_map<std::string, bool> m_userBoolAttributes;
    unordered_map<std::string, std::string> m_userStringAttributes;
};


#endif
