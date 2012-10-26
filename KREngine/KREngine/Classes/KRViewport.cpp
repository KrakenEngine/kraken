//
//  KRViewport.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-10-25.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KRVector2.h"
#include "KRMat4.h"
#include "KRViewport.h"


KRViewport::KRViewport()
{
    m_size = KRVector2::One();
    m_matProjection = KRMat4();
    m_matView = KRMat4();
}

KRViewport::KRViewport(const KRVector2 &size, const KRMat4 &matView, const KRMat4 &matProjection)
{
    m_size = size;
    m_matView = matView;
    m_matProjection = matProjection;
}


KRViewport& KRViewport::operator=(const KRViewport &v) {
    if(this != &v) { // Prevent self-assignment.
        m_size = v.m_size;
        m_matProjection = v.m_matProjection;
        m_matView = v.m_matView;
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
}

void KRViewport::setProjectionMatrix(const KRMat4 &matProjection)
{
    m_matProjection = matProjection;
}
