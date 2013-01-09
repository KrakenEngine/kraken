//
//  KROctreeNode.h
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-08-29.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KROCTREENODE_H
#define KROCTREENODE_H

#import "KREngine-common.h"
#include "KRVector3.h"
#include "KRAABB.h"
#include "KRHitInfo.h"

class KRNode;

class KROctreeNode {
public:
    KROctreeNode(const KRAABB &bounds);
    KROctreeNode(const KRAABB &bounds, int iChild, KROctreeNode *pChild);
    ~KROctreeNode();
    
    KROctreeNode **getChildren();
    std::set<KRNode *> &getSceneNodes();
    
    void add(KRNode *pNode);
    void remove(KRNode *pNode);
    void update(KRNode *pNode);
    
    KRAABB getBounds();
    
    void setChildNode(int iChild, KROctreeNode *pChild);
    int getChildIndex(KRNode *pNode);
    KRAABB getChildBounds(int iChild);
    bool isEmpty() const;
    
    bool canShrinkRoot() const;
    KROctreeNode *stripChild();
    
    void beginOcclusionQuery();
    void endOcclusionQuery();
    
    
    GLuint m_occlusionQuery;
    bool m_occlusionTested;
    bool m_activeQuery;
    
    bool lineCast(const KRVector3 &v0, const KRVector3 &v1, KRHitInfo &hitinfo, unsigned int layer_mask);
    bool rayCast(const KRVector3 &v0, const KRVector3 &dir, KRHitInfo &hitinfo, unsigned int layer_mask);
private:
    
    KRAABB m_bounds;
    
    KROctreeNode *m_children[8];
    
    std::set<KRNode *>m_sceneNodes;
};


#endif /* defined(KROCTREENODE_H) */
