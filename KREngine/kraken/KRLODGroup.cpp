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
    m_referencePoint = KRVector3::Zero();
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
    
    e->SetAttribute("reference_x", m_referencePoint.x);
    e->SetAttribute("reference_y", m_referencePoint.y);
    e->SetAttribute("reference_z", m_referencePoint.z);
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
    if(e->QueryFloatAttribute("reference_x", &x) != tinyxml2::XML_SUCCESS) {
        x = 0.0f;
    }
    if(e->QueryFloatAttribute("reference_y", &y) != tinyxml2::XML_SUCCESS) {
        y = 0.0f;
    }
    if(e->QueryFloatAttribute("reference_z", &z) != tinyxml2::XML_SUCCESS) {
        z = 0.0f;
    }
    m_referencePoint = KRVector3(x,y,z);
}


const KRVector3 KRLODGroup::getReferencePoint()
{
    return m_referencePoint;
}

void KRLODGroup::setReferencePoint(const KRVector3 &referencePoint)
{
    m_referencePoint = referencePoint;
}

const KRVector3 KRLODGroup::getWorldReferencePoint()
{
    return localToWorld(m_referencePoint);
}

bool KRLODGroup::getLODVisibility(const KRViewport &viewport)
{
    if(m_min_distance == 0 && m_max_distance == 0) {
        return true;
    } else {
        // return (m_max_distance == 0); // FINDME, HACK - Test code to enable only the lowest LOD group
        float lod_bias = viewport.getLODBias();
        lod_bias = pow(2.0f, -lod_bias);
        // Compare square distances as sqrt is expensive
        float sqr_distance = (viewport.getCameraPosition() - getWorldReferencePoint()).sqrMagnitude() * (lod_bias * lod_bias);
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

