//
//  KRBehavior.cpp
//  Kraken
//
//  Created by Kearwood Gilbert on 2013-05-17.
//  Copyright (c) 2013 Kearwood Software. All rights reserved.
//

#include "KRBehavior.h"
#include "KRNode.h"

KRBehavior::KRBehavior()
{
    __node = NULL;
}

KRBehavior::~KRBehavior()
{
    
}

KRNode *KRBehavior::getNode() const
{
    return __node;
}

void KRBehavior::__setNode(KRNode *node)
{
    __node = node;
}
