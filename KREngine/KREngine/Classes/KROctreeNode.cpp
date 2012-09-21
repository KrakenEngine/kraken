//
//  KROctreeNode.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-08-29.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KROctreeNode.h"
#include "KRNode.h"

KROctreeNode::KROctreeNode(const KRAABB &bounds) : m_bounds(bounds)
{
    for(int i=0; i<8; i++) m_children[i] = NULL;
    
    m_occlusionQuery = 0;
    m_occlusionTested = false;
    m_activeQuery = false;
}

KROctreeNode::KROctreeNode(const KRAABB &bounds, int iChild, KROctreeNode *pChild) : m_bounds(bounds)
{
    // This constructor is used when expanding the octree and replacing the root node with a new root that encapsulates it
    for(int i=0; i<8; i++) m_children[i] = NULL;
    m_children[iChild] = pChild;
    
    m_occlusionQuery = 0;
    m_occlusionTested = false;
    m_activeQuery = false;
}

KROctreeNode::~KROctreeNode()
{
    for(int i=0; i<8; i++) {
        if(m_children[i] != NULL) {
            delete m_children[i];
        }
    }
#if TARGET_OS_IPHONE
    if(m_occlusionTested) {
        GLDEBUG(glDeleteQueriesEXT(1, &m_occlusionQuery));
    }
#endif
}

#if TARGET_OS_IPHONE
void KROctreeNode::beginOcclusionQuery()
{
    if(!m_occlusionTested){
        GLDEBUG(glGenQueriesEXT(1, &m_occlusionQuery));
        GLDEBUG(glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, m_occlusionQuery));
        m_occlusionTested = true;
        m_activeQuery = true;
    }
}

void KROctreeNode::endOcclusionQuery()
{
    if(m_activeQuery) {
        // Only end a query if we started one
        GLDEBUG(glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT));
    }
}

#endif


KRAABB KROctreeNode::getBounds()
{
    return m_bounds;
}

void KROctreeNode::add(KRNode *pNode)
{
    int iChild = getChildIndex(pNode);
    if(iChild == -1) {
        m_sceneNodes.insert(pNode);
    } else {
        if(m_children[iChild] == NULL) {
            m_children[iChild] = new KROctreeNode(getChildBounds(iChild));
        }
        m_children[iChild]->add(pNode);
    }
}

KRAABB KROctreeNode::getChildBounds(int iChild)
{
    KRVector3 center = m_bounds.center();
    
    return KRAABB(
       KRVector3(
                 (iChild & 1) == 0 ? m_bounds.min.x : center.x,
                 (iChild & 2) == 0 ? m_bounds.min.y : center.y,
                 (iChild & 4) == 0 ? m_bounds.min.z : center.z),
       KRVector3(
                 (iChild & 1) == 0 ? center.x : m_bounds.max.x,
                 (iChild & 2) == 0 ? center.y : m_bounds.max.y,
                 (iChild & 4) == 0 ? center.z : m_bounds.max.z)
    );
}

int KROctreeNode::getChildIndex(KRNode *pNode)
{    
    for(int iChild=0; iChild < 8; iChild++) {
        if(getChildBounds(iChild).contains(pNode->getBounds())) {
            return iChild;
        }
    }
    return -1;
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
