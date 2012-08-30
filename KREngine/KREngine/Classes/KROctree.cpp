//
//  KROctree.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-08-29.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KROctree.h"
#include "KRNode.h"

KROctree::KROctree()
{
    m_pRootNode = NULL;
}

KROctree::~KROctree()
{
    if(m_pRootNode) {
        delete m_pRootNode;
    }
}

void KROctree::add(KRNode *pNode)
{
    KRVector3 minPoint = pNode->getMinPoint();
    KRVector3 maxPoint = pNode->getMaxPoint();
    if(pNode->getMinPoint() == KRVector3::Min() && pNode->getMaxPoint() == KRVector3::Max()) {
        // This item is infinitely large; we track it separately
        m_outerSceneNodes.insert(pNode);
    } else { 
        if(m_pRootNode == NULL) {
            // First item inserted, create a node large enough to fit it
            m_pRootNode = new KROctreeNode(pNode->getMinPoint(), pNode->getMaxPoint());
            m_pRootNode->add(pNode);
        } else {
            // Keep encapsulating the root node until the new root contains the inserted node
            bool bInsideRoot = false;
            while(!bInsideRoot) {
                KRVector3 rootMinPoint = m_pRootNode->getMinPoint();
                KRVector3 rootMaxPoint = m_pRootNode->getMaxPoint();
                KRVector3 rootSize = rootMaxPoint - rootMinPoint;
                if(minPoint.x < rootMinPoint.x || minPoint.y < rootMinPoint.y || minPoint.z < rootMinPoint.z) {
                    m_pRootNode = new KROctreeNode(rootMinPoint - rootSize, rootMaxPoint, 7, m_pRootNode);
                } else if(maxPoint.x > rootMaxPoint.x || maxPoint.y > rootMaxPoint.y || maxPoint.z > rootMaxPoint.z) {
                    m_pRootNode = new KROctreeNode(rootMaxPoint, rootMaxPoint + rootSize, 0, m_pRootNode);
                } else {
                    bInsideRoot = true;
                }
            }
            m_pRootNode->add(pNode);
        }
    }
}

void KROctree::remove(KRNode *pNode)
{
    if(!m_outerSceneNodes.erase(pNode)) {
        if(m_pRootNode) {
            m_pRootNode->remove(pNode);
        }
    }
    
    shrink();
}

void KROctree::update(KRNode *pNode)
{
    // TODO: This may be more efficient as an incremental operation rather than removing and re-adding the node
    remove(pNode);
    add(pNode);
    shrink();
}

void KROctree::shrink()
{
    if(m_pRootNode) {
        while(m_pRootNode->canShrinkRoot()) {
            KROctreeNode *newRoot = m_pRootNode->stripChild();
            delete m_pRootNode;
            m_pRootNode = newRoot;
        }
    }
}

KROctreeNode *KROctree::getRootNode()
{
    return m_pRootNode;
}

std::set<KRNode *> &KROctree::getOuterSceneNodes()
{
    return m_outerSceneNodes;
}

