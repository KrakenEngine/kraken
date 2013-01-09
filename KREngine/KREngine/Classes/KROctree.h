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
#include "KRHitInfo.h"

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
    
    bool lineCast(const KRVector3 &v0, const KRVector3 &v1, KRHitInfo &hitinfo, unsigned int layer_mask);
    bool rayCast(const KRVector3 &v0, const KRVector3 &dir, KRHitInfo &hitinfo, unsigned int layer_mask);

private:
    KROctreeNode *m_pRootNode;
    std::set<KRNode *> m_outerSceneNodes;
    //std::set<KRMat4> visibleMVPs;
    
    void shrink();
};

#endif /* defined(KROCTREE_H) */
