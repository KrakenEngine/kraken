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
            m_pRootNode = new KROctreeNode(KRAABB(pNode->getMinPoint(), pNode->getMaxPoint()));
            m_pRootNode->add(pNode);
        } else {
            // Keep encapsulating the root node until the new root contains the inserted node
            bool bInsideRoot = false;
            while(!bInsideRoot) {
                KRAABB rootBounds = m_pRootNode->getBounds();
                KRVector3 rootSize = rootBounds.size();
                if(minPoint.x < rootBounds.min.x || minPoint.y < rootBounds.min.y || minPoint.z < rootBounds.min.z) {
                    m_pRootNode = new KROctreeNode(KRAABB(rootBounds.min - rootSize, rootBounds.max), 7, m_pRootNode);
                } else if(maxPoint.x > rootBounds.max.x || maxPoint.y > rootBounds.max.y || maxPoint.z > rootBounds.max.z) {
                    m_pRootNode = new KROctreeNode(KRAABB(rootBounds.max, rootBounds.max + rootSize), 0, m_pRootNode);
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

void KROctree::getOcclusionQueryResults(std::set<KRAABB> &renderedBounds)
{
    renderedBounds.clear();
    if(m_pRootNode) {
        m_pRootNode->getOcclusionQueryResults(renderedBounds);
    }
}
