//
//  KRViewport.h
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-10-25.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KRENGINE_KRVIEWPORT_H
#define KRENGINE_KRVIEWPORT_H

#include "KRVector2.h"
#include "KRMat4.h"
#include "KRAABB.h"
#include <map.h>

class KRLight;

class KRViewport {
public:
    KRViewport();
    KRViewport(const KRVector2 &size, const KRMat4 &matView, const KRMat4 &matProjection);
    ~KRViewport();
    
    const KRVector2 &getSize() const;
    const KRMat4 &getViewMatrix() const;
    const KRMat4 &getProjectionMatrix() const;
    const KRMat4 &getViewProjectionMatrix() const;
    const KRMat4 &getInverseViewMatrix() const;
    const KRMat4 &getInverseProjectionMatrix() const;
    const KRVector3 &getCameraDirection() const;
    const KRVector3 &getCameraPosition() const;
    const int *getFrontToBackOrder() const;
    const int *getBackToFrontOrder() const;
    void setSize(const KRVector2 &size);
    void setViewMatrix(const KRMat4 &matView);
    void setProjectionMatrix(const KRMat4 &matProjection);
    
    // Overload assignment operator
    KRViewport& operator=(const KRViewport &v);
    
    std::map<KRAABB, int> &getVisibleBounds();
    
    const std::set<KRLight *> &getVisibleLights();
    void setVisibleLights(const std::set<KRLight *> visibleLights);
    
private:
    KRVector2 m_size;
    KRMat4 m_matView;
    KRMat4 m_matProjection;
    
    
    // Derived values
    KRMat4 m_matViewProjection;
    KRMat4 m_matInverseView;
    KRMat4 m_matInverseProjection;
    KRVector3 m_cameraDirection;
    KRVector3 m_cameraPosition;
    
    int m_frontToBackOrder[8];
    int m_backToFrontOrder[8];
    
    void calculateDerivedValues();
    
    std::map<KRAABB, int> m_visibleBounds; // AABB's that output fragments in the last frame
};

#endif
