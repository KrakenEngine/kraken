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
    if(m_lod_visible >= LOD_VISIBILITY_PRESTREAM) {
        /*
        // FINDME, TODO, HACK - Disabled streamer delayed LOD load due to performance issues:
        KRLODGroup *new_active_lod_group = NULL;
        */
        
        // Upgrade and downgrade LOD groups as needed
        for(std::set<KRNode *>::iterator itr=m_childNodes.begin(); itr != m_childNodes.end(); ++itr) {
            KRLODGroup *lod_group = dynamic_cast<KRLODGroup *>(*itr);
            assert(lod_group != NULL);
            LodVisibility group_lod_visibility = KRMIN(lod_group->calcLODVisibility(viewport), m_lod_visible);
           /*
           // FINDME, TODO, HACK - Disabled streamer delayed LOD load due to performance issues:
            if(group_lod_visibility == LOD_VISIBILITY_VISIBLE) {
                new_active_lod_group = lod_group;
            }
            */
            lod_group->setLODVisibility(group_lod_visibility);
        }
        
        /*
        // FINDME, TODO, HACK - Disabled streamer delayed LOD load due to performance issues:
        bool streamer_ready = false;
        if(new_active_lod_group == NULL) {
            streamer_ready = true;
        } else if(new_active_lod_group->getStreamLevel(viewport) >= kraken_stream_level::STREAM_LEVEL_IN_LQ) {
            streamer_ready = true;
        }
        */
        bool streamer_ready = true;
        
        if(streamer_ready) {
            // Upgrade and downgrade LOD groups as needed
            for(std::set<KRNode *>::iterator itr=m_childNodes.begin(); itr != m_childNodes.end(); ++itr) {
                KRLODGroup *lod_group = dynamic_cast<KRLODGroup *>(*itr);
                assert(lod_group != NULL);
                LodVisibility group_lod_visibility = KRMIN(lod_group->calcLODVisibility(viewport), m_lod_visible);
                lod_group->setLODVisibility(group_lod_visibility);
            }
        }
        
        KRNode::updateLODVisibility(viewport);
    }
}

void KRLODSet::setLODVisibility(KRNode::LodVisibility lod_visibility)
{
    if(lod_visibility == LOD_VISIBILITY_HIDDEN) {
        KRNode::setLODVisibility(lod_visibility);
    } else if(m_lod_visible != lod_visibility) {
        // Don't automatically recurse into our children, as only one of those will be activated, by updateLODVisibility
        if(m_lod_visible == LOD_VISIBILITY_HIDDEN && lod_visibility >= LOD_VISIBILITY_PRESTREAM) {
            getScene().notify_sceneGraphCreate(this);
        }
        m_lod_visible = lod_visibility;
    }
}

kraken_stream_level KRLODSet::getStreamLevel(const KRViewport &viewport)
{
    KRLODGroup *new_active_lod_group = NULL;

    // Upgrade and downgrade LOD groups as needed
    for(std::set<KRNode *>::iterator itr=m_childNodes.begin(); itr != m_childNodes.end(); ++itr) {
        KRLODGroup *lod_group = dynamic_cast<KRLODGroup *>(*itr);
        assert(lod_group != NULL);
        if(lod_group->calcLODVisibility(viewport) == LOD_VISIBILITY_VISIBLE) {
            new_active_lod_group = lod_group;
        }
    }
    
    if(new_active_lod_group) {
        return new_active_lod_group->getStreamLevel(viewport);
    } else {
        return kraken_stream_level::STREAM_LEVEL_IN_HQ;
    }
}

