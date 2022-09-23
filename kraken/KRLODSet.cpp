//
//  KRLODSet.cpp
//  Kraken Engine
//
//  Copyright 2022 Kearwood Gilbert. All rights reserved.
//  
//  Redistribution and use in source and binary forms, with or without modification, are
//  permitted provided that the following conditions are met:
//  
//  1. Redistributions of source code must retain the above copyright notice, this list of
//  conditions and the following disclaimer.
//  
//  2. Redistributions in binary form must reproduce the above copyright notice, this list
//  of conditions and the following disclaimer in the documentation and/or other materials
//  provided with the distribution.
//  
//  THIS SOFTWARE IS PROVIDED BY KEARWOOD GILBERT ''AS IS'' AND ANY EXPRESS OR IMPLIED
//  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL KEARWOOD GILBERT OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//  
//  The views and conclusions contained in the software and documentation are those of the
//  authors and should not be interpreted as representing official policies, either expressed
//  or implied, of Kearwood Gilbert.
//

#include "KRLODSet.h"
#include "KRLODGroup.h"
#include "KRContext.h"

/* static */
void KRLODSet::InitNodeInfo(KrNodeInfo* nodeInfo)
{
  KRNode::InitNodeInfo(nodeInfo);
  // No additional members
}

KRLODSet::KRLODSet(KRScene& scene, std::string name) : KRNode(scene, name)
{

}

KRLODSet::~KRLODSet()
{}

std::string KRLODSet::getElementName()
{
  return "lod_set";
}

tinyxml2::XMLElement* KRLODSet::saveXML(tinyxml2::XMLNode* parent)
{
  tinyxml2::XMLElement* e = KRNode::saveXML(parent);

  return e;
}

void KRLODSet::loadXML(tinyxml2::XMLElement* e)
{
  KRNode::loadXML(e);
}


void KRLODSet::updateLODVisibility(const KRViewport& viewport)
{
  if (m_lod_visible >= LOD_VISIBILITY_PRESTREAM) {
    /*
    // FINDME, TODO, HACK - Disabled streamer delayed LOD load due to performance issues:
    KRLODGroup *new_active_lod_group = NULL;
    */

    // Upgrade and downgrade LOD groups as needed
    for (std::list<KRNode*>::iterator itr = m_childNodes.begin(); itr != m_childNodes.end(); ++itr) {
      KRLODGroup* lod_group = dynamic_cast<KRLODGroup*>(*itr);
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

    if (streamer_ready) {
      // Upgrade and downgrade LOD groups as needed
      for (std::list<KRNode*>::iterator itr = m_childNodes.begin(); itr != m_childNodes.end(); ++itr) {
        KRLODGroup* lod_group = dynamic_cast<KRLODGroup*>(*itr);
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
  if (lod_visibility == LOD_VISIBILITY_HIDDEN) {
    KRNode::setLODVisibility(lod_visibility);
  } else if (m_lod_visible != lod_visibility) {
    // Don't automatically recurse into our children, as only one of those will be activated, by updateLODVisibility
    if (m_lod_visible == LOD_VISIBILITY_HIDDEN && lod_visibility >= LOD_VISIBILITY_PRESTREAM) {
      getScene().notify_sceneGraphCreate(this);
    }
    m_lod_visible = lod_visibility;
  }
}

kraken_stream_level KRLODSet::getStreamLevel(const KRViewport& viewport)
{
  KRLODGroup* new_active_lod_group = NULL;

  // Upgrade and downgrade LOD groups as needed
  for (std::list<KRNode*>::iterator itr = m_childNodes.begin(); itr != m_childNodes.end(); ++itr) {
    KRLODGroup* lod_group = dynamic_cast<KRLODGroup*>(*itr);
    assert(lod_group != NULL);
    if (lod_group->calcLODVisibility(viewport) == LOD_VISIBILITY_VISIBLE) {
      new_active_lod_group = lod_group;
    }
  }

  if (new_active_lod_group) {
    return new_active_lod_group->getStreamLevel(viewport);
  } else {
    return kraken_stream_level::STREAM_LEVEL_IN_HQ;
  }
}

