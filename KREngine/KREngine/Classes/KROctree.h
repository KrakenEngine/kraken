//
//  KROctree.h
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-08-29.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KRAABBTREE_H
#define KRAABBTREE_H

#import "KREngine-common.h"
#include "KROctreeNode.h"

class KRNode;

class KROctree {
public:
    KROctree();
    ~KROctree();
    
    void add(KRNode *pNode);
    void remove(KRNode *pNode);
    void update(KRNode *pNode);
    
private:
    KROctreeNode *m_pRootNode;
    std::set<KRNode *>m_outerSceneNodes;
    
    void shrink();
};

#endif /* defined(KRAABBTREE_H) */
