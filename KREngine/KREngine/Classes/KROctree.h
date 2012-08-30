//
//  KROctree.h
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-08-29.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KROCTREE_H
#define KROCTREE_H

#import "KREngine-common.h"
#include "KROctreeNode.h"
#include "KRMat4.h"

class KRNode;

class KROctree {
public:
    KROctree();
    ~KROctree();
    
    void add(KRNode *pNode);
    void remove(KRNode *pNode);
    void update(KRNode *pNode);
    
    KROctreeNode *getRootNode();
    std::set<KRNode *> &getOuterSceneNodes();
    
private:
    KROctreeNode *m_pRootNode;
    std::set<KRNode *>m_outerSceneNodes;
    //std::set<KRMat4> visibleMVPs;
    
    void shrink();
};

#endif /* defined(KROCTREE_H) */
