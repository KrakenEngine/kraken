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


KRViewport::KRViewport()
{
    m_size = KRVector2::One();
    m_matProjection = KRMat4();
    m_matView = KRMat4();
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
