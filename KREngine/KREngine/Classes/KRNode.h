//
//  KRNode.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-11.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KREngine_KRNode_h
#define KREngine_KRNode_h

#import "KRResource.h"
#import "KRVector3.h"
#import "tinyxml2.h"

class KRNode
{
public:
    KRNode(std::string name);
    virtual ~KRNode();
    
    virtual tinyxml2::XMLElement *saveXML( tinyxml2::XMLNode *parent);
    static KRNode *LoadXML(tinyxml2::XMLElement *e);
    virtual void loadXML(tinyxml2::XMLElement *e);
    
    virtual std::string getElementName();
    
    void addChild(KRNode *child);
    
    void setLocalTranslation(const KRVector3 &v);
    void setLocalScale(const KRVector3 &v);
    void setLocalRotation(const KRVector3 &v);
    
    const KRVector3 &getLocalTranslation();
    const KRVector3 &getLocalScale();
    const KRVector3 &getLocalRotation();
    
private:
    KRVector3 m_localTranslation;
    KRVector3 m_localScale;
    KRVector3 m_localRotation;
    
    std::string m_name;
    
    std::vector<KRNode *> m_childNodes;
};


#endif
