//
//  KROctreeNode.h
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-08-29.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KROCTREENODE_H
#define KROCTREENODE_H

#include "KREngine-common.h"
#include "KRHitInfo.h"

class KRNode;

class KROctreeNode {
public:
    KROctreeNode(KROctreeNode *parent, const KRAABB &bounds);
    KROctreeNode(KROctreeNode *parent, const KRAABB &bounds, int iChild, KROctreeNode *pChild);
    ~KROctreeNode();
    
    KROctreeNode **getChildren();
    std::set<KRNode *> &getSceneNodes();
    
    void add(KRNode *pNode);
    void remove(KRNode *pNode);
    void update(KRNode *pNode);
    
    KRAABB getBounds();
    
    KROctreeNode *getParent();
    void setChildNode(int iChild, KROctreeNode *pChild);
    int getChildIndex(KRNode *pNode);
    KRAABB getChildBounds(int iChild);
    void trim();
    bool isEmpty() const;
    
    bool canShrinkRoot() const;
    KROctreeNode *stripChild();
    
    void beginOcclusionQuery();
    void endOcclusionQuery();
    
    
    GLuint m_occlusionQuery;
    bool m_occlusionTested;
    bool m_activeQuery;
    
    bool lineCast(const Vector3 &v0, const Vector3 &v1, KRHitInfo &hitinfo, unsigned int layer_mask);
    bool rayCast(const Vector3 &v0, const Vector3 &dir, KRHitInfo &hitinfo, unsigned int layer_mask);
    bool sphereCast(const Vector3 &v0, const Vector3 &v1, float radius, KRHitInfo &hitinfo, unsigned int layer_mask);

private:
    
    KRAABB m_bounds;
    
    KROctreeNode *m_parent;
    KROctreeNode *m_children[8];
    
    std::set<KRNode *>m_sceneNodes;
};


#endif /* defined(KROCTREENODE_H) */
