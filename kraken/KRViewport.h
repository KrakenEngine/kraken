//
//  KRViewport.h
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

#pragma once

#include "KREngine-common.h"

#include "aabb.h"

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
    
    unordered_map<AABB, int> &getVisibleBounds();
    
    const std::set<KRLight *> &getVisibleLights();
    void setVisibleLights(const std::set<KRLight *> visibleLights);
    
    bool visible(const AABB &b) const;
    float coverage(const AABB &b) const;
    
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
    
    unordered_map<AABB, int> m_visibleBounds; // AABB's that output fragments in the last frame
};
