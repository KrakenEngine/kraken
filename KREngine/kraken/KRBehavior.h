//
//  KRBehavior.h
//  Kraken
//
//  Created by Kearwood Gilbert on 2013-05-17.
//  Copyright (c) 2013 Kearwood Software. All rights reserved.
//

#ifndef KRBEHAVIOR_H
#define KRBEHAVIOR_H



/*

 This class is a pure-virtual base class intended to be subclassed to define behavior of KRNode's in the scene
 
*/

class KRNode;

class KRBehavior
{
public:
    KRBehavior();
    virtual ~KRBehavior();
    KRNode *getNode() const;
    
    virtual void update(float deltaTime) = 0;
    virtual void visibleUpdate(float deltatime) = 0;
    void __setNode(KRNode *node);
private:
    KRNode *__node;
};

#endif /* defined(KRBEHAVIOR_H) */
