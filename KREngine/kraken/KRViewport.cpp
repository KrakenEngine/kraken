//
//  KRViewport.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-10-25.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#define KRENGINE_SWAP_INT(x,y) {int t;t=x;x=y;y=t;}

#include "KRVector2.h"
#include "KRMat4.h"
#include "KRViewport.h"
#include "KRLight.h"


KRViewport::KRViewport()
{
    m_size = KRVector2::One();
    m_matProjection = KRMat4();
    m_matView = KRMat4();
    m_lodBias = 0.0f;
    calculateDerivedValues();
}

KRViewport::KRViewport(const KRVector2 &size, const KRMat4 &matView, const KRMat4 &matProjection)
{
    m_size = size;
    m_matView = matView;
    m_matProjection = matProjection;
    calculateDerivedValues();
}


KRViewport& KRViewport::operator=(const KRViewport &v) {
    if(this != &v) { // Prevent self-assignment.
        m_size = v.m_size;
        m_matProjection = v.m_matProjection;
        m_matView = v.m_matView;
        m_lodBias = v.m_lodBias;
        
        calculateDerivedValues();
    }
    return *this;
}


KRViewport::~KRViewport()
{
    
}

const KRVector2 &KRViewport::getSize() const
{
    return m_size;
}

const KRMat4 &KRViewport::getViewMatrix() const
{
    return m_matView;
}

const KRMat4 &KRViewport::getProjectionMatrix() const
{
    return m_matProjection;
}

void KRViewport::setSize(const KRVector2 &size)
{
    m_size = size;
}

void KRViewport::setViewMatrix(const KRMat4 &matView)
{
    m_matView = matView;
    calculateDerivedValues();
}

void KRViewport::setProjectionMatrix(const KRMat4 &matProjection)
{
    m_matProjection = matProjection;
    calculateDerivedValues();
}

const KRMat4 &KRViewport::KRViewport::getViewProjectionMatrix() const
{
    return m_matViewProjection;
}

const KRMat4 &KRViewport::getInverseViewMatrix() const
{
    return m_matInverseView;
}

const KRMat4 &KRViewport::getInverseProjectionMatrix() const
{
    return m_matInverseProjection;
}

const KRVector3 &KRViewport::getCameraDirection() const
{
    return m_cameraDirection;
}

const KRVector3 &KRViewport::getCameraPosition() const
{
    return m_cameraPosition;
}

const int *KRViewport::getFrontToBackOrder() const
{
    return &m_frontToBackOrder[0];
}

const int *KRViewport::getBackToFrontOrder() const
{
    return &m_backToFrontOrder[0];
}

void KRViewport::calculateDerivedValues()
{
    m_matViewProjection = m_matView * m_matProjection;
    m_matInverseView = KRMat4::Invert(m_matView);
    m_matInverseProjection = KRMat4::Invert(m_matProjection);
    m_cameraPosition = KRMat4::Dot(m_matInverseView, KRVector3::Zero());
    m_cameraDirection = KRMat4::Dot(m_matInverseView, KRVector3(0.0, 0.0, 1.0)) - KRMat4::Dot(m_matInverseView, KRVector3(0.0, 0.0, 0.0));

    for(int i=0; i<8; i++) {
        m_frontToBackOrder[i] = i;
    }
    
    if(m_cameraDirection.x > 0.0) {
        KRENGINE_SWAP_INT(m_frontToBackOrder[0], m_frontToBackOrder[1]);
        KRENGINE_SWAP_INT(m_frontToBackOrder[2], m_frontToBackOrder[3]);
        KRENGINE_SWAP_INT(m_frontToBackOrder[4], m_frontToBackOrder[5]);
        KRENGINE_SWAP_INT(m_frontToBackOrder[6], m_frontToBackOrder[7]);
    }
    
    if(m_cameraDirection.y > 0.0) {
        KRENGINE_SWAP_INT(m_frontToBackOrder[0], m_frontToBackOrder[2]);
        KRENGINE_SWAP_INT(m_frontToBackOrder[1], m_frontToBackOrder[3]);
        KRENGINE_SWAP_INT(m_frontToBackOrder[4], m_frontToBackOrder[6]);
        KRENGINE_SWAP_INT(m_frontToBackOrder[5], m_frontToBackOrder[7]);
    }
    
    if(m_cameraDirection.z > 0.0) {
        KRENGINE_SWAP_INT(m_frontToBackOrder[0], m_frontToBackOrder[4]);
        KRENGINE_SWAP_INT(m_frontToBackOrder[1], m_frontToBackOrder[5]);
        KRENGINE_SWAP_INT(m_frontToBackOrder[2], m_frontToBackOrder[6]);
        KRENGINE_SWAP_INT(m_frontToBackOrder[3], m_frontToBackOrder[7]);
    }
    
    for(int i=0; i<8; i++) {
        m_backToFrontOrder[i] = m_frontToBackOrder[7-i];
    }
}


std::unordered_map<KRAABB, int> &KRViewport::getVisibleBounds()
{
    return m_visibleBounds;
}

float KRViewport::getLODBias() const
{
    return m_lodBias;
}

void KRViewport::setLODBias(float lod_bias)
{
    m_lodBias = lod_bias;
}

float KRViewport::coverage(const KRAABB &b) const
{
    if(!visible(b)) {
        return 0.0f; // Culled out by view frustrum
    } else {
        KRVector2 screen_min;
        KRVector2 screen_max;
        // Loop through all corners and transform them to screen space
        for(int i=0; i<8; i++) {
            KRVector3 screen_pos = KRMat4::DotWDiv(m_matViewProjection, KRVector3(i & 1 ? b.min.x : b.max.x, i & 2 ? b.min.y : b.max.y, i & 4 ? b.min.z : b.max.z));
            if(i==0) {
                screen_min.x = screen_pos.x;
                screen_min.y = screen_pos.y;
                screen_max.x = screen_pos.x;
                screen_max.y = screen_pos.y;
            } else {
                if(screen_pos.x < screen_min.x) screen_min.x = screen_pos.x;
                if(screen_pos.y < screen_min.y) screen_min.y = screen_pos.y;
                if(screen_pos.x > screen_max.x) screen_max.x = screen_pos.x;
                if(screen_pos.y > screen_max.y) screen_max.y = screen_pos.y;
            }
        }
        
        return (screen_max.x - screen_min.x) * (screen_max.y - screen_min.y);
    }
}


bool KRViewport::visible(const KRAABB &b) const
{
    // test if bounding box would be within the visible range of the clip space transformed by matViewProjection
    // This is used for view frustrum culling
    
    int outside_count[6] = {0, 0, 0, 0, 0, 0};
    
    for(int iCorner=0; iCorner<8; iCorner++) {
        KRVector4 sourceCornerVertex = KRVector4(
                                                 (iCorner & 1) == 0 ? b.min.x : b.max.x,
                                                 (iCorner & 2) == 0 ? b.min.y : b.max.y,
                                                 (iCorner & 4) == 0 ? b.min.z : b.max.z, 1.0f);
        
        KRVector4 cornerVertex = KRMat4::Dot4(m_matViewProjection, sourceCornerVertex);
        
        if(cornerVertex.x < -cornerVertex.w) {
            outside_count[0]++;
        }
        if(cornerVertex.y < -cornerVertex.w) {
            outside_count[1]++;
        }
        if(cornerVertex.z < -cornerVertex.w) {
            outside_count[2]++;
        }
        if(cornerVertex.x > cornerVertex.w) {
            outside_count[3]++;
        }
        if(cornerVertex.y > cornerVertex.w) {
            outside_count[4]++;
        }
        if(cornerVertex.z > cornerVertex.w) {
            outside_count[5]++;
        }
    }
    
    bool is_visible = true;
    for(int iFace=0; iFace < 6; iFace++) {
        if(outside_count[iFace] == 8) {
            is_visible = false;
        }
    }
    
    return is_visible;
}





