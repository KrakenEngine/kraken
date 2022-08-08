//
//  KRLODGroup.cpp
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

#include "KRLODGroup.h"
#include "KRLODSet.h"
#include "KRContext.h"

/* static */
void KRLODGroup::InitNodeInfo(KrNodeInfo* nodeInfo)
{
  KRNode::InitNodeInfo(nodeInfo);
  nodeInfo->lod_group.min_distance = 0.0f;
  nodeInfo->lod_group.max_distance = 0.0f;
  nodeInfo->lod_group.reference_min = Vector3::Zero();
  nodeInfo->lod_group.reference_max = Vector3::Zero();
  nodeInfo->lod_group.use_world_units = true;
}

KRLODGroup::KRLODGroup(KRScene& scene, std::string name) : KRNode(scene, name)
{
  m_min_distance = 0.0f;
  m_max_distance = 0.0f;
  m_reference = AABB::Create(Vector3::Zero(), Vector3::Zero());
  m_use_world_units = true;
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
  e->SetAttribute("min_distance", m_min_distance);
  e->SetAttribute("max_distance", m_max_distance);

  e->SetAttribute("reference_min_x", m_reference.min.x);
  e->SetAttribute("reference_min_y", m_reference.min.y);
  e->SetAttribute("reference_min_z", m_reference.min.z);


  e->SetAttribute("reference_max_x", m_reference.max.x);
  e->SetAttribute("reference_max_y", m_reference.max.y);
  e->SetAttribute("reference_max_z", m_reference.max.z);

  e->SetAttribute("use_world_units", m_use_world_units ? "true" : "false");
  return e;
}

void KRLODGroup::loadXML(tinyxml2::XMLElement* e)
{
  KRNode::loadXML(e);

  m_min_distance = 0.0f;
  if (e->QueryFloatAttribute("min_distance", &m_min_distance) != tinyxml2::XML_SUCCESS) {
    m_min_distance = 0.0f;
  }

  m_max_distance = 0.0f;
  if (e->QueryFloatAttribute("max_distance", &m_max_distance) != tinyxml2::XML_SUCCESS) {
    m_max_distance = 0.0f;
  }


  float x = 0.0f, y = 0.0f, z = 0.0f;
  if (e->QueryFloatAttribute("reference_min_x", &x) != tinyxml2::XML_SUCCESS) {
    x = 0.0f;
  }
  if (e->QueryFloatAttribute("reference_min_y", &y) != tinyxml2::XML_SUCCESS) {
    y = 0.0f;
  }
  if (e->QueryFloatAttribute("reference_min_z", &z) != tinyxml2::XML_SUCCESS) {
    z = 0.0f;
  }

  m_reference.min = Vector3::Create(x, y, z);

  x = 0.0f; y = 0.0f; z = 0.0f;
  if (e->QueryFloatAttribute("reference_max_x", &x) != tinyxml2::XML_SUCCESS) {
    x = 0.0f;
  }
  if (e->QueryFloatAttribute("reference_max_y", &y) != tinyxml2::XML_SUCCESS) {
    y = 0.0f;
  }
  if (e->QueryFloatAttribute("reference_max_z", &z) != tinyxml2::XML_SUCCESS) {
    z = 0.0f;
  }
  m_reference.max = Vector3::Create(x, y, z);

  m_use_world_units = true;
  if (e->QueryBoolAttribute("use_world_units", &m_use_world_units) != tinyxml2::XML_SUCCESS) {
    m_use_world_units = true;
  }
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
    Vector3 local_reference_point = m_reference.nearestPoint(local_camera_position);

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
