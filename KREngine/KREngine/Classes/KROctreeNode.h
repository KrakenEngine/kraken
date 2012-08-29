//
//  KROctreeNode.h
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-08-29.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KRAABBTREENODE_H
#define KRAABBTREENODE_H

#import "KREngine-common.h"
#include "KRVector3.h"

class KRNode;

class KROctreeNode {
public:
    KROctreeNode(const KRVector3 &minPoint, const KRVector3 &maxPoint);
    KROctreeNode(const KRVector3 &minPoint, const KRVector3 &maxPoint, int iChild, KROctreeNode *pChild);
    ~KROctreeNode();
    
    void add(KRNode *pNode);
    void remove(KRNode *pNode);
    void update(KRNode *pNode);
    
    KRVector3 getMinPoint();
    KRVector3 getMaxPoint();
    
    void setChildNode(int iChild, KROctreeNode *pChild);
    int getChildIndex(KRNode *pNode);
    bool isEmpty() const;
    
    bool canShrinkRoot() const;
    KROctreeNode *stripChild();
    
private:
    KRVector3 m_minPoint;
    KRVector3 m_maxPoint;
    
    KROctreeNode *m_children[8];
    
    std::set<KRNode *>m_sceneNodes;
};


#endif /* defined(KRAABBTREENODE_H) */
