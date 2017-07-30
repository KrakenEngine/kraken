//
//  KRViewport.h
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-10-25.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KRENGINE_KRVIEWPORT_H
#define KRENGINE_KRVIEWPORT_H

#include "KREngine-common.h"

class KRLight;

class KRViewport {
public:
    KRViewport();
    KRViewport(const Vector2 &size, const Matrix4 &matView, const Matrix4 &matProjection);
    ~KRViewport();
    
    const Vector2 &getSize() const;
    const Matrix4 &getViewMatrix() const;
    const Matrix4 &getProjectionMatrix() const;
    const Matrix4 &getViewProjectionMatrix() const;
    const Matrix4 &getInverseViewMatrix() const;
    const Matrix4 &getInverseProjectionMatrix() const;
    const Vector3 &getCameraDirection() const;
    const Vector3 &getCameraPosition() const;
    const int *getFrontToBackOrder() const;
    const int *getBackToFrontOrder() const;
    void setSize(const Vector2 &size);
    void setViewMatrix(const Matrix4 &matView);
    void setProjectionMatrix(const Matrix4 &matProjection);
    float getLODBias() const;
    void setLODBias(float lod_bias);
    
    // Overload assignment operator
    KRViewport& operator=(const KRViewport &v);
    
    unordered_map<KRAABB, int> &getVisibleBounds();
    
    const std::set<KRLight *> &getVisibleLights();
    void setVisibleLights(const std::set<KRLight *> visibleLights);
    
    bool visible(const KRAABB &b) const;
    float coverage(const KRAABB &b) const;
    
private:
    Vector2 m_size;
    Matrix4 m_matView;
    Matrix4 m_matProjection;
    
    float m_lodBias;
    
    // Derived values
    Matrix4 m_matViewProjection;
    Matrix4 m_matInverseView;
    Matrix4 m_matInverseProjection;
    Vector3 m_cameraDirection;
    Vector3 m_cameraPosition;
    
    int m_frontToBackOrder[8];
    int m_backToFrontOrder[8];
    
    void calculateDerivedValues();
    
    unordered_map<KRAABB, int> m_visibleBounds; // AABB's that output fragments in the last frame
    
    
};

#endif
