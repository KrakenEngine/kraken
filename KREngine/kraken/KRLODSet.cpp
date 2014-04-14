//
//  KRLODSet.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-12-06.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KRLODSet.h"
#include "KRLODGroup.h"
#include "KRContext.h"

KRLODSet::KRLODSet(KRScene &scene, std::string name) : KRNode(scene, name)
{
    m_activeLODGroup = NULL;
}

KRLODSet::~KRLODSet()
{
}

std::string KRLODSet::getElementName() {
    return "lod_set";
}

tinyxml2::XMLElement *KRLODSet::saveXML( tinyxml2::XMLNode *parent)
{
    tinyxml2::XMLElement *e = KRNode::saveXML(parent);
    
    return e;
}

void KRLODSet::loadXML(tinyxml2::XMLElement *e)
{
    KRNode::loadXML(e);
}


void KRLODSet::updateLODVisibility(const KRViewport &viewport)
{
    if(m_lod_visible) {
        KRLODGroup *new_active_lod_group = NULL;
        
        // Upgrade and downgrade LOD groups as needed
        for(std::set<KRNode *>::iterator itr=m_childNodes.begin(); itr != m_childNodes.end(); ++itr) {
            KRLODGroup *lod_group = dynamic_cast<KRLODGroup *>(*itr);
            assert(lod_group != NULL);
            if(lod_group->getLODVisibility(viewport)) {
                new_active_lod_group = lod_group;
            }
        }
        
        if(new_active_lod_group == NULL) {
            m_activeLODGroup = NULL;
        } else if(m_activeLODGroup == NULL) {
            m_activeLODGroup = new_active_lod_group;
        } else if(new_active_lod_group != m_activeLODGroup) {
            if(true || new_active_lod_group->getStreamLevel(true, viewport) >= kraken_stream_level::STREAM_LEVEL_IN_LQ) { // FINDME, HACK!  Disabled due to performance issues.
                // fprintf(stderr, "LOD %s -> %s\n", m_activeLODGroup->getName().c_str(), new_active_lod_group->getName().c_str());
                m_activeLODGroup = new_active_lod_group;
            } else {
                // fprintf(stderr, "LOD %s -> %s - waiting for streaming...\n", m_activeLODGroup->getName().c_str(), new_active_lod_group->getName().c_str());
            }
        }

        for(std::set<KRNode *>::iterator itr=m_childNodes.begin(); itr != m_childNodes.end(); ++itr) {
            KRNode *child = *itr;
            if(child == m_activeLODGroup) {
                child->showLOD();
            } else {
                child->hideLOD();
            }
        }
        
        KRNode::updateLODVisibility(viewport);
    }
}

KRLODGroup *KRLODSet::getActiveLODGroup() const
{
    return m_activeLODGroup;
}

void KRLODSet::childDeleted(KRNode *child_node)
{
    KRNode::childDeleted(child_node);
    if(m_activeLODGroup == child_node) {
        m_activeLODGroup = NULL;
    }
}

void KRLODSet::hideLOD()
{
    KRNode::hideLOD();
    m_activeLODGroup = NULL; // Ensure that the streamer will wait for the group to load in next time
}

void KRLODSet::showLOD()
{
    // Don't automatically recurse into our children, as only one of those will be activated, by updateLODVisibility
    if(!m_lod_visible) {
        getScene().notify_sceneGraphCreate(this);
        m_lod_visible = true;
    }
}

kraken_stream_level KRLODSet::getStreamLevel(bool prime, const KRViewport &viewport)
{
    KRLODGroup *new_active_lod_group = NULL;

    // Upgrade and downgrade LOD groups as needed
    for(std::set<KRNode *>::iterator itr=m_childNodes.begin(); itr != m_childNodes.end(); ++itr) {
        KRLODGroup *lod_group = dynamic_cast<KRLODGroup *>(*itr);
        assert(lod_group != NULL);
        if(lod_group->getLODVisibility(viewport)) {
            new_active_lod_group = lod_group;
        }
    }
    
    if(new_active_lod_group) {
        return new_active_lod_group->getStreamLevel(prime, viewport);
    } else {
        return kraken_stream_level::STREAM_LEVEL_IN_HQ;
    }
}

