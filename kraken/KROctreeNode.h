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
#include "hitinfo.h"

class KRNode;

class KROctreeNode {
public:
    KROctreeNode(KROctreeNode *parent, const AABB &bounds);
    KROctreeNode(KROctreeNode *parent, const AABB &bounds, int iChild, KROctreeNode *pChild);
    ~KROctreeNode();

    KROctreeNode **getChildren();
    std::set<KRNode *> &getSceneNodes();

    void add(KRNode *pNode);
    void remove(KRNode *pNode);
    void update(KRNode *pNode);

    AABB getBounds();

    KROctreeNode *getParent();
    void setChildNode(int iChild, KROctreeNode *pChild);
    int getChildIndex(KRNode *pNode);
    AABB getChildBounds(int iChild);
    void trim();
    bool isEmpty() const;

    bool canShrinkRoot() const;
    KROctreeNode *stripChild();

    void beginOcclusionQuery();
    void endOcclusionQuery();


    GLuint m_occlusionQuery;
    bool m_occlusionTested;
    bool m_activeQuery;

    bool lineCast(const Vector3 &v0, const Vector3 &v1, HitInfo &hitinfo, unsigned int layer_mask);
    bool rayCast(const Vector3 &v0, const Vector3 &dir, HitInfo &hitinfo, unsigned int layer_mask);
    bool sphereCast(const Vector3 &v0, const Vector3 &v1, float radius, HitInfo &hitinfo, unsigned int layer_mask);

private:

    AABB m_bounds;

    KROctreeNode *m_parent;
    KROctreeNode *m_children[8];

    std::set<KRNode *>m_sceneNodes;
};


#endif /* defined(KROCTREENODE_H) */
