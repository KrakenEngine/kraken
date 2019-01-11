//
//  KROctreeNode.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-08-29.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KROctreeNode.h"
#include "KRNode.h"
#include "KRCollider.h"

KROctreeNode::KROctreeNode(KROctreeNode *parent, const AABB &bounds) : m_bounds(bounds)
{
    m_parent = parent;
    
    for(int i=0; i<8; i++) m_children[i] = NULL;
    
    m_occlusionQuery = 0;
    m_occlusionTested = false;
    m_activeQuery = false;
}

KROctreeNode::KROctreeNode(KROctreeNode *parent, const AABB &bounds, int iChild, KROctreeNode *pChild) : m_bounds(bounds)
{
    // This constructor is used when expanding the octree and replacing the root node with a new root that encapsulates it
    m_parent = parent;
    
    for(int i=0; i<8; i++) m_children[i] = NULL;
    m_children[iChild] = pChild;
    pChild->m_parent = this;
    
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

    if(m_occlusionTested) {
        GLDEBUG(glDeleteQueriesEXT(1, &m_occlusionQuery));
    }
}


void KROctreeNode::beginOcclusionQuery()
{
    if(!m_occlusionTested){
        GLDEBUG(glGenQueriesEXT(1, &m_occlusionQuery));
#if TARGET_OS_IPHONE || defined(ANDROID)
        GLDEBUG(glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, m_occlusionQuery));
#else
        GLDEBUG(glBeginQuery(GL_SAMPLES_PASSED, m_occlusionQuery));
#endif
        m_occlusionTested = true;
        m_activeQuery = true;
    }
}

void KROctreeNode::endOcclusionQuery()
{
    if(m_activeQuery) {
        // Only end a query if we started one
#if TARGET_OS_IPHONE || defined(ANDROID)
        GLDEBUG(glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT));
#else
        GLDEBUG(glEndQuery(GL_SAMPLES_PASSED));
#endif
    }
}


AABB KROctreeNode::getBounds()
{
    return m_bounds;
}

void KROctreeNode::add(KRNode *pNode)
{
    int iChild = getChildIndex(pNode);
    if(iChild == -1) {
        m_sceneNodes.insert(pNode);
        pNode->addToOctreeNode(this);
    } else {
        if(m_children[iChild] == NULL) {
            m_children[iChild] = new KROctreeNode(this, getChildBounds(iChild));
        }
        m_children[iChild]->add(pNode);
    }
}

AABB KROctreeNode::getChildBounds(int iChild)
{
    Vector3 center = m_bounds.center();
    
    return AABB::Create(
       Vector3::Create(
                 (iChild & 1) == 0 ? m_bounds.min.x : center.x,
                 (iChild & 2) == 0 ? m_bounds.min.y : center.y,
                 (iChild & 4) == 0 ? m_bounds.min.z : center.z),
       Vector3::Create(
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

void KROctreeNode::trim()
{
    for(int iChild = 0; iChild < 8; iChild++) {
        if(m_children[iChild]) {
            if(m_children[iChild]->isEmpty()) {
                delete m_children[iChild];
                m_children[iChild] = NULL;
            }
        }
    }
}

void KROctreeNode::remove(KRNode *pNode)
{
    m_sceneNodes.erase(pNode);
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
            child->m_parent = NULL;
            m_children[i] = NULL;
            return child;
        }
    }
    return NULL;
}


KROctreeNode *KROctreeNode::getParent()
{
    return m_parent;
}

KROctreeNode **KROctreeNode::getChildren()
{
    return m_children;
}

std::set<KRNode *> &KROctreeNode::getSceneNodes()
{
    return m_sceneNodes;
}


bool KROctreeNode::lineCast(const Vector3 &v0, const Vector3 &v1, HitInfo &hitinfo, unsigned int layer_mask)
{
    bool hit_found = false;
    if(hitinfo.didHit() && v1 != hitinfo.getPosition()) {
        // Optimization: If we already have a hit, only search for hits that are closer
        hit_found = lineCast(v0, hitinfo.getPosition(), hitinfo, layer_mask);
    } else {
        if(getBounds().intersectsLine(v0, v1)) {
            for(std::set<KRNode *>::iterator nodes_itr=m_sceneNodes.begin(); nodes_itr != m_sceneNodes.end(); nodes_itr++) {
                KRCollider *collider = dynamic_cast<KRCollider *>(*nodes_itr);
                if(collider) {
                    if(collider->lineCast(v0, v1, hitinfo, layer_mask)) hit_found = true;
                }
            }
            
            for(int i=0; i<8; i++) {
                if(m_children[i]) {
                    if(m_children[i]->lineCast(v0, v1, hitinfo, layer_mask)) {
                        hit_found = true;
                    }
                }
            }
        }
    }

    return hit_found;
}

bool KROctreeNode::rayCast(const Vector3 &v0, const Vector3 &dir, HitInfo &hitinfo, unsigned int layer_mask)
{
    bool hit_found = false;
    if(hitinfo.didHit()) {
        // Optimization: If we already have a hit, only search for hits that are closer
        hit_found = lineCast(v0, hitinfo.getPosition(), hitinfo, layer_mask); // Note: This is purposefully lineCast as opposed to RayCast
    } else {
        if(getBounds().intersectsRay(v0, dir)) {
            for(std::set<KRNode *>::iterator nodes_itr=m_sceneNodes.begin(); nodes_itr != m_sceneNodes.end(); nodes_itr++) {
                KRCollider *collider = dynamic_cast<KRCollider *>(*nodes_itr);
                if(collider) {
                    if(collider->rayCast(v0, dir, hitinfo, layer_mask)) hit_found = true;
                }
            }
            
            for(int i=0; i<8; i++) {
                if(m_children[i]) {
                    if(m_children[i]->rayCast(v0, dir, hitinfo, layer_mask)) {
                        hit_found = true;
                    }
                }
            }
        }
    }

    return hit_found;
}

bool KROctreeNode::sphereCast(const Vector3 &v0, const Vector3 &v1, float radius, HitInfo &hitinfo, unsigned int layer_mask)
{
    bool hit_found = false;
    /*
     // FINDME, TODO - Adapt this optimization to work with sphereCasts
     
    if(hitinfo.didHit() && v1 != hitinfo.getPosition()) {
        // Optimization: If we already have a hit, only search for hits that are closer
        hit_found = sphereCast(v0, hitinfo.getPosition(), radius, hitinfo, layer_mask);
    } else {
    */
    
        AABB swept_bounds = AABB::Create(Vector3::Create(KRMIN(v0.x, v1.x) - radius, KRMIN(v0.y, v1.y) - radius, KRMIN(v0.z, v1.z) - radius), Vector3::Create(KRMAX(v0.x, v1.x) + radius, KRMAX(v0.y, v1.y) + radius, KRMAX(v0.z, v1.z) + radius));
        // FINDME, TODO - Investigate AABB - swept sphere intersections or OBB - AABB intersections: "if(getBounds().intersectsSweptSphere(v0, v1, radius)) {"
        if(getBounds().intersects(swept_bounds)) {
        
            for(std::set<KRNode *>::iterator nodes_itr=m_sceneNodes.begin(); nodes_itr != m_sceneNodes.end(); nodes_itr++) {
                KRCollider *collider = dynamic_cast<KRCollider *>(*nodes_itr);
                if(collider) {
                    if(collider->sphereCast(v0, v1, radius, hitinfo, layer_mask)) hit_found = true;
                }
            }
            
            for(int i=0; i<8; i++) {
                if(m_children[i]) {
                    if(m_children[i]->sphereCast(v0, v1, radius, hitinfo, layer_mask)) {
                        hit_found = true;
                    }
                }
            }
        }
    // }
    
    return hit_found;
}

