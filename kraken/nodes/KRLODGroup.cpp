//
//  KRLODGroup.cpp
//  Kraken Engine
//
//  Copyright 2025 Kearwood Gilbert. All rights reserved.
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

#include "KRLODGroup.h"
#include "KRLODSet.h"
#include "KRContext.h"

using namespace hydra;

/* static */
void KRLODGroup::InitNodeInfo(KrNodeInfo* nodeInfo)
{
  KRNode::InitNodeInfo(nodeInfo);
  nodeInfo->lod_group.min_distance = decltype(m_min_distance)::defaultVal;
  nodeInfo->lod_group.max_distance = decltype(m_max_distance)::defaultVal;
  nodeInfo->lod_group.reference_min = decltype(m_reference)::defaultVal.min;
  nodeInfo->lod_group.reference_max = decltype(m_reference)::defaultVal.max;
  nodeInfo->lod_group.use_world_units = true;
}

KRLODGroup::KRLODGroup(KRScene& scene, std::string name) : KRNode(scene, name)
{
}

KRLODGroup::~KRLODGroup()
{}

std::string KRLODGroup::getElementName()
{
  return "lod_group";
}

tinyxml2::XMLElement* KRLODGroup::saveXML(tinyxml2::XMLNode* parent)
{
  tinyxml2::XMLElement* e = KRNode::saveXML(parent);
  m_min_distance.save(e);
  m_max_distance.save(e);
  m_reference.save(e);
  m_use_world_units.save(e);
  return e;
}

void KRLODGroup::loadXML(tinyxml2::XMLElement* e)
{
  KRNode::loadXML(e);
  m_min_distance.load(e);
  m_max_distance.load(e);
  m_reference.load(e);
  m_use_world_units.load(e);
}


const AABB& KRLODGroup::getReference() const
{
  return m_reference;
}

void KRLODGroup::setReference(const AABB& reference)
{
  m_reference = reference;
}

KRNode::LodVisibility KRLODGroup::calcLODVisibility(const KRViewport& viewport)
{
  if (m_min_distance == 0 && m_max_distance == 0) {
    return LOD_VISIBILITY_VISIBLE;
  } else {
    float lod_bias = viewport.getLODBias();
    lod_bias = pow(2.0f, -lod_bias);

    // Compare using squared distances as sqrt is expensive
    float sqr_distance;
    float sqr_prestream_distance;

    Vector3 world_camera_position = viewport.getCameraPosition();
    Vector3 local_camera_position = worldToLocal(world_camera_position);
    Vector3 local_reference_point = m_reference.val.nearestPoint(local_camera_position);

    if (m_use_world_units) {
      Vector3 world_reference_point = localToWorld(local_reference_point);
      sqr_distance = (world_camera_position - world_reference_point).sqrMagnitude() * (lod_bias * lod_bias);
      sqr_prestream_distance = (float)(getContext().KRENGINE_PRESTREAM_DISTANCE * getContext().KRENGINE_PRESTREAM_DISTANCE);
    } else {
      sqr_distance = (local_camera_position - local_reference_point).sqrMagnitude() * (lod_bias * lod_bias);

      Vector3 world_reference_point = localToWorld(local_reference_point);
      sqr_prestream_distance = worldToLocal(Vector3::Normalize(world_reference_point - world_camera_position) * (float)getContext().KRENGINE_PRESTREAM_DISTANCE).sqrMagnitude(); // TODO, FINDME - Optimize with precalc?

    }

    float sqr_min_visible_distance = m_min_distance * m_min_distance;
    float sqr_max_visible_distance = m_max_distance * m_max_distance;
    if ((sqr_distance >= sqr_min_visible_distance || m_min_distance == 0) && (sqr_distance < sqr_max_visible_distance || m_max_distance == 0)) {
      return LOD_VISIBILITY_VISIBLE;
    } else if ((sqr_distance >= sqr_min_visible_distance - sqr_prestream_distance || m_min_distance == 0) && (sqr_distance < sqr_max_visible_distance + sqr_prestream_distance || m_max_distance == 0)) {
      return LOD_VISIBILITY_PRESTREAM;
    } else {
      return LOD_VISIBILITY_HIDDEN;
    }
  }
}

float KRLODGroup::getMinDistance()
{
  return m_min_distance;
}

float KRLODGroup::getMaxDistance()
{
  return m_max_distance;
}

void KRLODGroup::setMinDistance(float min_distance)
{
  m_min_distance = min_distance;
}

void KRLODGroup::setMaxDistance(float max_distance)
{
  m_max_distance = max_distance;
}

void KRLODGroup::setUseWorldUnits(bool use_world_units)
{
  m_use_world_units = use_world_units;
}

bool KRLODGroup::getUseWorldUnits() const
{
  return m_use_world_units;
}
