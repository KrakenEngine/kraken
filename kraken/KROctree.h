//
//  KROctree.h
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-08-29.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KROCTREE_H
#define KROCTREE_H

#include "KREngine-common.h"
#include "KROctreeNode.h"
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
    
    bool lineCast(const Vector3 &v0, const Vector3 &v1, KRHitInfo &hitinfo, unsigned int layer_mask);
    bool rayCast(const Vector3 &v0, const Vector3 &dir, KRHitInfo &hitinfo, unsigned int layer_mask);
    bool sphereCast(const Vector3 &v0, const Vector3 &v1, float radius, KRHitInfo &hitinfo, unsigned int layer_mask);

private:
    KROctreeNode *m_pRootNode;
    std::set<KRNode *> m_outerSceneNodes;
    
    void shrink();
};

#endif /* defined(KROCTREE_H) */
