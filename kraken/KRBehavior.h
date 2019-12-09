//
//  KRBehavior.h
//  Kraken
//
//  Created by Kearwood Gilbert on 2013-05-17.
//  Copyright (c) 2013 Kearwood Software. All rights reserved.
//

#ifndef KRBEHAVIOR_H
#define KRBEHAVIOR_H

#include <map>
#include <string>

/*

 This class is a pure-virtual base class intended to be subclassed to define behavior of KRNode's in the scene

*/

class KRBehavior;
class KRNode;
namespace tinyxml2 {
class XMLElement;
} // namespace tinyxml2

typedef KRBehavior *(*KRBehaviorFactoryFunction)(std::map<std::string, std::string> attributes);
typedef std::map<std::string, KRBehaviorFactoryFunction> KRBehaviorFactoryFunctionMap;

class KRBehavior
{
public:
    static void RegisterFactoryCTOR(std::string behaviorName, KRBehaviorFactoryFunction fnFactory);
    static void UnregisterFactoryCTOR(std::string behaviorName);

    KRBehavior();
    virtual ~KRBehavior();
    KRNode *getNode() const;

    virtual void init();
    virtual void update(float deltaTime) = 0;
    virtual void visibleUpdate(float deltatime) = 0;
    void __setNode(KRNode *node);

    static KRBehavior *LoadXML(KRNode *node, tinyxml2::XMLElement *e);
private:
    KRNode *__node;
};

#endif /* defined(KRBEHAVIOR_H) */
