//
//  KROctreeNode.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-08-29.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KROctreeNode.h"
#include "KRNode.h"

KROctreeNode::KROctreeNode(const KRVector3 &minPoint, const KRVector3 &maxPoint)
{
    m_minPoint = minPoint;
    m_maxPoint = maxPoint;
    for(int i=0; i<8; i++) m_children[i] = NULL;
}

KROctreeNode::KROctreeNode(const KRVector3 &minPoint, const KRVector3 &maxPoint, int iChild, KROctreeNode *pChild)
{
    // This constructor is used when expanding the octree and replacing the root node with a new root that encapsulates it
    m_minPoint = minPoint;
    m_maxPoint = maxPoint;
    for(int i=0; i<8; i++) m_children[i] = NULL;
    m_children[iChild] = pChild;
}

KROctreeNode::~KROctreeNode()
{
    for(int i=0; i<8; i++) {
        if(m_children[i] != NULL) {
            delete m_children[i];
        }
    }
}

void KROctreeNode::add(KRNode *pNode)
{

    
    KRVector3 center = (m_minPoint + m_maxPoint) / 2.0f;
    
    int iChild = getChildIndex(pNode);
    if(iChild == -1) {
        m_sceneNodes.insert(pNode);
    } else {
        if(m_children[iChild] == NULL) {
            m_children[iChild] = new KROctreeNode(
                  KRVector3(
                    (iChild & 1) == 0 ? m_minPoint.x : center.x,
                    (iChild & 2) == 0 ? m_minPoint.y : center.y,
                    (iChild & 4) == 0 ? m_minPoint.z : center.z),
                  KRVector3(
                    (iChild & 1) == 0 ? center.x : m_maxPoint.x,
                    (iChild & 2) == 0 ? center.y : m_maxPoint.y,
                    (iChild & 4) == 0 ? center.z : m_maxPoint.z)
            );
        }
        m_children[iChild]->add(pNode);
    }
}

int KROctreeNode::getChildIndex(KRNode *pNode)
{
    KRVector3 min = pNode->getMinPoint();
    KRVector3 max = pNode->getMaxPoint();
    // 0: max.x < center.x && max.y < center.y && max.z < center.z
    // 1: min.x > center.x && max.y < center.y && max.z < center.z
    // 2: max.x < center.x && min.y > center.y && max.z < center.z
    // 3: min.x > center.x && min.y > center.y && max.z < center.z
    // 4: max.x < center.x && max.y < center.y && min.z > center.z
    // 5: min.x > center.x && max.y < center.y && min.z > center.z
    // 6: max.x < center.x && min.y > center.y && min.z > center.z
    // 7: min.x > center.x && min.y > center.y && min.z > center.z
    
    KRVector3 center = (m_minPoint + m_maxPoint) / 2.0f;
    int iChild = -1;
    if(max.z < center.z) {
        if(max.y < center.y) {
            if(max.x < center.x) {
                iChild = 0;
            } else if(min.x > center.x) {
                iChild = 1;
            }
        } else if(min.y > center.y) {
            if(max.x < center.x) {
                iChild = 2;
            } else if(min.x > center.x) {
                iChild = 3;
            }
        }
    } else if(min.z > center.z) {
        if(max.y < center.y) {
            if(min.x > center.x) {
                iChild = 4;
            } else if(min.x > center.x) {
                iChild = 5;
            }
        } else if(min.y > center.y) {
            if(max.x < center.x) {
                iChild = 6;
            } else if(min.x > center.x) {
                iChild = 7;
            }
        }
    }
    return iChild;
}

void KROctreeNode::remove(KRNode *pNode)
{
    if(!m_sceneNodes.erase(pNode)) {
        int iChild = getChildIndex(pNode);
        if(m_children[iChild]) {
            m_children[iChild]->remove(pNode);
            if(m_children[iChild]->isEmpty()) {
                delete m_children[iChild];
                m_children[iChild] = NULL;
            }
        }
    }
}

void KROctreeNode::update(KRNode *pNode)
{
    
}

KRVector3 KROctreeNode::getMinPoint()
{
    return m_minPoint;
}

KRVector3 KROctreeNode::getMaxPoint()
{
    return m_maxPoint;
}

bool KROctreeNode::isEmpty() const
{
    for(int i=0; i<8; i++) {
        if(m_children[i]) {
            return false;
        }
    }
    return m_sceneNodes.empty();
}

bool KROctreeNode::canShrinkRoot() const
{
    int cChildren = 0;
    for(int i=0; i<8; i++) {
        if(m_children[i]) {
            cChildren++;
        }
    }
    return cChildren <= 1 && m_sceneNodes.empty();
}

KROctreeNode *KROctreeNode::stripChild()
{
    // Return the first found child and update its reference to NULL so that the destructor will not free it.  This is used for shrinking the octree
    // NOTE: The caller of this function will be responsible for freeing the child object.  It is also possible to return a NULL
    for(int i=0; i<8; i++) {
        if(m_children[i]) {
            KROctreeNode *child = m_children[i];
            m_children[i] = NULL;
            return child;
        }
    }
    return NULL;
}


KROctreeNode **KROctreeNode::getChildren()
{
    return m_children;
}

std::set<KRNode *> &KROctreeNode::getSceneNodes()
{
    return m_sceneNodes;
}
