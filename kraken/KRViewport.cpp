//
//  KRViewport.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-10-25.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#define KRENGINE_SWAP_INT(x,y) {int t;t=x;x=y;y=t;}

#include "KREngine-common.h"

#include "KRViewport.h"

KRViewport::KRViewport()
{
    m_size = Vector2::One();
    m_matProjection = Matrix4();
    m_matView = Matrix4();
    m_lodBias = 0.0f;
    calculateDerivedValues();
}

KRViewport::KRViewport(const Vector2 &size, const Matrix4 &matView, const Matrix4 &matProjection)
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

const Vector2 &KRViewport::getSize() const
{
    return m_size;
}

const Matrix4 &KRViewport::getViewMatrix() const
{
    return m_matView;
}

const Matrix4 &KRViewport::getProjectionMatrix() const
{
    return m_matProjection;
}

void KRViewport::setSize(const Vector2 &size)
{
    m_size = size;
}

void KRViewport::setViewMatrix(const Matrix4 &matView)
{
    m_matView = matView;
    calculateDerivedValues();
}

void KRViewport::setProjectionMatrix(const Matrix4 &matProjection)
{
    m_matProjection = matProjection;
    calculateDerivedValues();
}

const Matrix4 &KRViewport::KRViewport::getViewProjectionMatrix() const
{
    return m_matViewProjection;
}

const Matrix4 &KRViewport::getInverseViewMatrix() const
{
    return m_matInverseView;
}

const Matrix4 &KRViewport::getInverseProjectionMatrix() const
{
    return m_matInverseProjection;
}

const Vector3 &KRViewport::getCameraDirection() const
{
    return m_cameraDirection;
}

const Vector3 &KRViewport::getCameraPosition() const
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
    m_matInverseView = Matrix4::Invert(m_matView);
    m_matInverseProjection = Matrix4::Invert(m_matProjection);
    m_cameraPosition = Matrix4::Dot(m_matInverseView, Vector3::Zero());
    m_cameraDirection = Matrix4::Dot(m_matInverseView, Vector3::Create(0.0, 0.0, 1.0)) - Matrix4::Dot(m_matInverseView, Vector3::Create(0.0, 0.0, 0.0));

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


unordered_map<AABB, int> &KRViewport::getVisibleBounds()
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

float KRViewport::coverage(const AABB &b) const
{
    if(!visible(b)) {
        return 0.0f; // Culled out by view frustrum
    } else {
        Vector3 nearest_point = b.nearestPoint(getCameraPosition());
        float distance = (nearest_point - getCameraPosition()).magnitude();
        
        Vector3 v = Matrix4::DotWDiv(m_matProjection, getCameraPosition() + getCameraDirection() * distance);
        
        float screen_depth = distance / 1000.0f;
        
        return KRCLAMP(1.0f - screen_depth, 0.01f, 1.0f);
        
        /*
        
        Vector2 screen_min;
        Vector2 screen_max;
        // Loop through all corners and transform them to screen space
        for(int i=0; i<8; i++) {
            Vector3 screen_pos = Matrix4::DotWDiv(m_matViewProjection, Vector3(i & 1 ? b.min.x : b.max.x, i & 2 ? b.min.y : b.max.y, i & 4 ? b.min.z : b.max.z));
            if(i==0) {
                screen_min = screen_pos.xy();
                screen_max = screen_pos.xy();
            } else {
                if(screen_pos.x < screen_min.x) screen_min.x = screen_pos.x;
                if(screen_pos.y < screen_min.y) screen_min.y = screen_pos.y;
                if(screen_pos.x > screen_max.x) screen_max.x = screen_pos.x;
                if(screen_pos.y > screen_max.y) screen_max.y = screen_pos.y;
            }
        }
        
        screen_min.x = KRCLAMP(screen_min.x, 0.0f, 1.0f);
        screen_min.y = KRCLAMP(screen_min.y, 0.0f, 1.0f);
        screen_max.x = KRCLAMP(screen_max.x, 0.0f, 1.0f);
        screen_max.y = KRCLAMP(screen_max.y, 0.0f, 1.0f);
        
        float c = (screen_max.x - screen_min.x) * (screen_max.y - screen_min.y);
        return KRCLAMP(c, 0.01f, 1.0f);
        */
    }
}


bool KRViewport::visible(const AABB &b) const
{
    // test if bounding box would be within the visible range of the clip space transformed by matViewProjection
    // This is used for view frustrum culling
    
    int outside_count[6] = {0, 0, 0, 0, 0, 0};
    
    for(int iCorner=0; iCorner<8; iCorner++) {
        Vector4 sourceCornerVertex = Vector4::Create(
                                                 (iCorner & 1) == 0 ? b.min.x : b.max.x,
                                                 (iCorner & 2) == 0 ? b.min.y : b.max.y,
                                                 (iCorner & 4) == 0 ? b.min.z : b.max.z, 1.0f);
        
        Vector4 cornerVertex = Matrix4::Dot4(m_matViewProjection, sourceCornerVertex);
        
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





