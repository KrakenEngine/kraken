//
//  KRLODGroup.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-12-06.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KRLODGroup.h"
#include "KRContext.h"

KRLODGroup::KRLODGroup(KRScene &scene, std::string name) : KRNode(scene, name)
{
    m_min_distance = 0.0f;
    m_max_distance = 0.0f;
    m_reference = KRAABB(KRVector3::Zero(), KRVector3::Zero());
    m_use_world_units = true;
}

KRLODGroup::~KRLODGroup()
{
}

std::string KRLODGroup::getElementName() {
    return "lod_group";
}

tinyxml2::XMLElement *KRLODGroup::saveXML( tinyxml2::XMLNode *parent)
{
    tinyxml2::XMLElement *e = KRNode::saveXML(parent);
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

void KRLODGroup::loadXML(tinyxml2::XMLElement *e)
{
    KRNode::loadXML(e);

    m_min_distance = 0.0f;
    if(e->QueryFloatAttribute("min_distance", &m_min_distance) != tinyxml2::XML_SUCCESS) {
        m_min_distance = 0.0f;
    }
    
    m_max_distance = 0.0f;
    if(e->QueryFloatAttribute("max_distance", &m_max_distance) != tinyxml2::XML_SUCCESS) {
        m_max_distance = 0.0f;
    }
    
    
    float x=0.0f, y=0.0f, z=0.0f;
    if(e->QueryFloatAttribute("reference_min_x", &x) != tinyxml2::XML_SUCCESS) {
        x = 0.0f;
    }
    if(e->QueryFloatAttribute("reference_min_y", &y) != tinyxml2::XML_SUCCESS) {
        y = 0.0f;
    }
    if(e->QueryFloatAttribute("reference_min_z", &z) != tinyxml2::XML_SUCCESS) {
        z = 0.0f;
    }
    
    m_reference.min = KRVector3(x,y,z);
    
    x=0.0f; y=0.0f; z=0.0f;
    if(e->QueryFloatAttribute("reference_max_x", &x) != tinyxml2::XML_SUCCESS) {
        x = 0.0f;
    }
    if(e->QueryFloatAttribute("reference_max_y", &y) != tinyxml2::XML_SUCCESS) {
        y = 0.0f;
    }
    if(e->QueryFloatAttribute("reference_max_z", &z) != tinyxml2::XML_SUCCESS) {
        z = 0.0f;
    }
    m_reference.max = KRVector3(x,y,z);
    
    m_use_world_units = true;
    if(e->QueryBoolAttribute("use_world_units", &m_use_world_units) != tinyxml2::XML_SUCCESS) {
        m_use_world_units = true;
    }
}


const KRAABB &KRLODGroup::getReference() const
{
    return m_reference;
}

void KRLODGroup::setReference(const KRAABB &reference)
{
    m_reference = reference;
}

bool KRLODGroup::getLODVisibility(const KRViewport &viewport)
{
    if(m_min_distance == 0 && m_max_distance == 0) {
        return true;
    } else {
        // return (m_max_distance == 0); // FINDME, HACK - Test code to enable only the lowest LOD group
        float lod_bias = viewport.getLODBias();
        lod_bias = pow(2.0f, -lod_bias);
        
        float sqr_distance; // Compare using squared distances as sqrt is expensive
        
        KRVector3 world_camera_position = viewport.getCameraPosition();
        KRVector3 local_camera_position = worldToLocal(world_camera_position);
        KRVector3 local_reference_point = m_reference.nearestPoint(local_camera_position);
        
        if(m_use_world_units) {
            KRVector3 world_reference_point = localToWorld(local_reference_point);
            sqr_distance = (world_camera_position - world_reference_point).sqrMagnitude() * (lod_bias * lod_bias);
        } else {
            sqr_distance = (local_camera_position - local_reference_point).sqrMagnitude() * (lod_bias * lod_bias);
        }
        
        float sqr_min_distance = m_min_distance * m_min_distance;
        float sqr_max_distance = m_max_distance * m_max_distance;
        return ((sqr_distance >= sqr_min_distance || m_min_distance == 0) && (sqr_distance < sqr_max_distance || m_max_distance == 0));
    }
}

void KRLODGroup::updateLODVisibility(const KRViewport &viewport)
{
    bool new_visibility = getLODVisibility(viewport);
    if(!new_visibility) {
        hideLOD();
    } else {
        if(!m_lod_visible) {
            getScene().notify_sceneGraphCreate(this);
            m_lod_visible = true;
        }
        for(std::vector<KRNode *>::iterator itr=m_childNodes.begin(); itr < m_childNodes.end(); ++itr) {
            (*itr)->updateLODVisibility(viewport);
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
