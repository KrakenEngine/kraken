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
    m_occlusionQueryTransparent = 0;
    m_occlusionTestedTransparent = false;
    m_activeQuery = false;
}

KROctreeNode::KROctreeNode(const KRAABB &bounds, int iChild, KROctreeNode *pChild) : m_bounds(bounds)
{
    // This constructor is used when expanding the octree and replacing the root node with a new root that encapsulates it
    for(int i=0; i<8; i++) m_children[i] = NULL;
    m_children[iChild] = pChild;
    
    m_occlusionQuery = 0;
    m_occlusionTested = false;
    m_occlusionQueryTransparent = 0;
    m_occlusionTestedTransparent = false;
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
    if(m_occlusionTestedTransparent) {
        GLDEBUG(glDeleteQueriesEXT(1, &m_occlusionQueryTransparent));
    }
#endif
}

#if TARGET_OS_IPHONE
void KROctreeNode::beginOcclusionQuery(bool bTransparentPass)
{
    if(bTransparentPass && !m_occlusionTestedTransparent) {
        GLDEBUG(glGenQueriesEXT(1, &m_occlusionQueryTransparent));
        GLDEBUG(glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, m_occlusionQueryTransparent));
        m_occlusionTestedTransparent = true;
        m_activeQuery = true;
    } else if(!bTransparentPass && !m_occlusionTested){
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

bool KROctreeNode::getOcclusionQueryResults(std::set<KRAABB> &renderedBounds)
{
    bool bRendered = false;
    bool bGoDeeper = false;
    
    if(m_occlusionTested) {
        GLuint params = 0;
        GLDEBUG(glGetQueryObjectuivEXT(m_occlusionQuery, GL_QUERY_RESULT_EXT, &params));
        if(params) bRendered = true; // At least one opaque fragment processed
        
        GLDEBUG(glDeleteQueriesEXT(1, &m_occlusionQuery));
        m_occlusionTested = false;
        bGoDeeper = true;
    }
    if(m_occlusionTestedTransparent) {
        GLuint params = 0;
        GLDEBUG(glGetQueryObjectuivEXT(m_occlusionQueryTransparent, GL_QUERY_RESULT_EXT, &params));
        if(params) bRendered = true; // At least one transparent fragment processed
        
        GLDEBUG(glDeleteQueriesEXT(1, &m_occlusionQueryTransparent));
        m_occlusionTestedTransparent = false;
        
        bGoDeeper = true;
    }
    
    // FINDME - Test Code:
    //bGoDeeper = true;
    //bRendered = true;
    
    if(bGoDeeper) { // Only recurse deeper if we reached this level in the previous pass
        for(int i=0; i<8; i++) {
            if(m_children[i]) {
                if(m_children[i]->getOcclusionQueryResults(renderedBounds)) {
                    bRendered = true; // We must always include the parent, even if the parent's local scene graph nodes are fully occluded
                }
            }
        }
    }
    
    if(bRendered) {
        renderedBounds.insert(m_bounds);
    }
    
    return bRendered;
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
    /*
     
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
    
    KRVector3 center = m_bounds.center();
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
     */
    
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
