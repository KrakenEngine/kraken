//
//  KRBoundingVolume.cpp
//  KREngine
//
//  Copyright 2012 Kearwood Gilbert. All rights reserved.
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

#include <iostream>
#include <math.h>

#import "KRBoundingVolume.h"


KRBoundingVolume::KRBoundingVolume(const KRVector3 *pVertices) {
    for(int iVertex=0; iVertex < 8; iVertex++) {
        m_vertices[iVertex] = pVertices[iVertex];
    }
}

KRBoundingVolume::KRBoundingVolume(const KRVector3 &corner1, const KRVector3 &corner2, const KRMat4 modelMatrix) {
    m_vertices[0] = KRVector3(corner1.x, corner1.y, corner1.z);
    m_vertices[1] = KRVector3(corner2.x, corner1.y, corner1.z);
    m_vertices[2] = KRVector3(corner2.x, corner2.y, corner1.z);
    m_vertices[3] = KRVector3(corner1.x, corner2.y, corner1.z);
    m_vertices[4] = KRVector3(corner1.x, corner1.y, corner2.z);
    m_vertices[5] = KRVector3(corner2.x, corner1.y, corner2.z);
    m_vertices[6] = KRVector3(corner2.x, corner2.y, corner2.z);
    m_vertices[7] = KRVector3(corner1.x, corner2.y, corner2.z);
    
    for(int iVertex=0; iVertex < 8; iVertex++) {
        m_vertices[iVertex] = KRMat4::Dot(modelMatrix, m_vertices[iVertex]);
    }
}

KRBoundingVolume::KRBoundingVolume(const KRMat4 &matView, GLfloat fov, GLfloat aspect, GLfloat nearz, GLfloat farz) {
    // Construct a bounding volume representing the volume of the view frustrum
    
    KRMat4 invView = matView;
    invView.invert();
    
    GLfloat r = tan(fov / 2.0);
    
    m_vertices[0] = KRVector3(-1.0 * r * nearz * aspect, -1.0 * r * nearz, -nearz);
    m_vertices[1] = KRVector3(1.0 * r * nearz * aspect,  -1.0 * r * nearz, -nearz);
    m_vertices[2] = KRVector3(1.0 * r * nearz * aspect,   1.0 * r * nearz, -nearz);
    m_vertices[3] = KRVector3(-1.0 * r * nearz * aspect,  1.0 * r * nearz, -nearz);
    m_vertices[4] = KRVector3(-1.0 * r * farz * aspect, -1.0 * r * farz, -farz);
    m_vertices[5] = KRVector3(1.0 * r * farz * aspect,  -1.0 * r * farz, -farz);
    m_vertices[6] = KRVector3(1.0 * r * farz * aspect,   1.0 * r * farz, -farz);
    m_vertices[7] = KRVector3(-1.0 * r * farz * aspect,  1.0 * r * farz, -farz);
    
    for(int iVertex=0; iVertex < 8; iVertex++) {
        m_vertices[iVertex] = KRMat4::Dot(invView, m_vertices[iVertex]);
    }
}

KRBoundingVolume::~KRBoundingVolume() {
    
}

KRBoundingVolume::KRBoundingVolume(const KRBoundingVolume& p) {
    for(int iVertex=0; iVertex < 8; iVertex++) {
        m_vertices[iVertex] = p.m_vertices[iVertex];
    }    
}

KRBoundingVolume& KRBoundingVolume::operator = ( const KRBoundingVolume& p ) {
    for(int iVertex=0; iVertex < 8; iVertex++) {
        m_vertices[iVertex] = p.m_vertices[iVertex];
    }
    return *this;
}

KRBoundingVolume KRBoundingVolume::get_union(const KRBoundingVolume &p) const {
    // Simple, non-aligned bounding box calculated that contains both volumes.
    
    KRVector3 minPoint = m_vertices[0], maxPoint = m_vertices[0];
    for(int iVertex=1; iVertex < 8; iVertex++) {
        if(m_vertices[iVertex].x < minPoint.x) {
            minPoint.x = m_vertices[iVertex].x;
        }
        if(m_vertices[iVertex].y < minPoint.y) {
            minPoint.y = m_vertices[iVertex].y;
        }
        if(m_vertices[iVertex].z < minPoint.z) {
            minPoint.z = m_vertices[iVertex].z;
        }
        if(m_vertices[iVertex].x > maxPoint.x) {
            maxPoint.x = m_vertices[iVertex].x;
        }
        if(m_vertices[iVertex].y > maxPoint.y) {
            maxPoint.y = m_vertices[iVertex].y;
        }
        if(m_vertices[iVertex].z > maxPoint.z) {
            maxPoint.z = m_vertices[iVertex].z;
        }
    }
    for(int iVertex=0; iVertex < 8; iVertex++) {
        if(p.m_vertices[iVertex].x < minPoint.x) {
            minPoint.x = p.m_vertices[iVertex].x;
        }
        if(p.m_vertices[iVertex].y < minPoint.y) {
            minPoint.y =p.m_vertices[iVertex].y;
        }
        if(p.m_vertices[iVertex].z < minPoint.z) {
            minPoint.z = p.m_vertices[iVertex].z;
        }
        if(p.m_vertices[iVertex].x > maxPoint.x) {
            maxPoint.x = p.m_vertices[iVertex].x;
        }
        if(p.m_vertices[iVertex].y > maxPoint.y) {
            maxPoint.y = p.m_vertices[iVertex].y;
        }
        if(p.m_vertices[iVertex].z > maxPoint.z) {
            maxPoint.z = p.m_vertices[iVertex].z;
        }
    }
    return KRBoundingVolume(minPoint, maxPoint, KRMat4());
}

bool KRBoundingVolume::test_intersect(const KRBoundingVolume &p) const {
    // Simple, non-aligned bounding box intersection test
    
    KRVector3 minPoint = m_vertices[0], maxPoint = m_vertices[0], minPoint2 = p.m_vertices[0], maxPoint2 = p.m_vertices[0];
    for(int iVertex=1; iVertex < 8; iVertex++) {
        if(m_vertices[iVertex].x < minPoint.x) {
            minPoint.x = m_vertices[iVertex].x;
        }
        if(m_vertices[iVertex].y < minPoint.y) {
            minPoint.y = m_vertices[iVertex].y;
        }
        if(m_vertices[iVertex].z < minPoint.z) {
            minPoint.z = m_vertices[iVertex].z;
        }
        if(m_vertices[iVertex].x > maxPoint.x) {
            maxPoint.x = m_vertices[iVertex].x;
        }
        if(m_vertices[iVertex].y > maxPoint.y) {
            maxPoint.y = m_vertices[iVertex].y;
        }
        if(m_vertices[iVertex].z > maxPoint.z) {
            maxPoint.z = m_vertices[iVertex].z;
        }
    }
    for(int iVertex=1; iVertex < 8; iVertex++) {
        if(p.m_vertices[iVertex].x < minPoint2.x) {
            minPoint2.x = p.m_vertices[iVertex].x;
        }
        if(p.m_vertices[iVertex].y < minPoint2.y) {
            minPoint2.y =p.m_vertices[iVertex].y;
        }
        if(p.m_vertices[iVertex].z < minPoint2.z) {
            minPoint2.z = p.m_vertices[iVertex].z;
        }
        if(p.m_vertices[iVertex].x > maxPoint2.x) {
            maxPoint2.x = p.m_vertices[iVertex].x;
        }
        if(p.m_vertices[iVertex].y > maxPoint2.y) {
            maxPoint2.y = p.m_vertices[iVertex].y;
        }
        if(p.m_vertices[iVertex].z > maxPoint2.z) {
            maxPoint2.z = p.m_vertices[iVertex].z;
        }
    }
    
    bool bIntersect = maxPoint.x >= minPoint2.x && maxPoint.y >= minPoint2.y && maxPoint.z >= minPoint2.z
    && minPoint.x <= maxPoint2.x && minPoint.y <= maxPoint2.y && minPoint.z <= maxPoint2.z;
    
    return bIntersect;
}

bool KRBoundingVolume::test_intersect(const KRAABB &p) const {
    // Simple, non-aligned bounding box intersection test
    
    KRVector3 minPoint = m_vertices[0], maxPoint = m_vertices[0], minPoint2 = p.min, maxPoint2 = p.max;
    for(int iVertex=1; iVertex < 8; iVertex++) {
        if(m_vertices[iVertex].x < minPoint.x) {
            minPoint.x = m_vertices[iVertex].x;
        }
        if(m_vertices[iVertex].y < minPoint.y) {
            minPoint.y = m_vertices[iVertex].y;
        }
        if(m_vertices[iVertex].z < minPoint.z) {
            minPoint.z = m_vertices[iVertex].z;
        }
        if(m_vertices[iVertex].x > maxPoint.x) {
            maxPoint.x = m_vertices[iVertex].x;
        }
        if(m_vertices[iVertex].y > maxPoint.y) {
            maxPoint.y = m_vertices[iVertex].y;
        }
        if(m_vertices[iVertex].z > maxPoint.z) {
            maxPoint.z = m_vertices[iVertex].z;
        }
    }
    
    bool bIntersect =
        maxPoint.x >= minPoint2.x
        && maxPoint.y >= minPoint2.y
        && maxPoint.z >= minPoint2.z
        && minPoint.x <= maxPoint2.x
        && minPoint.y <= maxPoint2.y
        && minPoint.z <= maxPoint2.z;
    
    return bIntersect;
}


KRMat4 KRBoundingVolume::calcShadowProj(KRScene *pScene, KRContext *pContext, GLfloat sun_yaw, GLfloat sun_pitch) const {
    KRBoundingVolume sceneVolume = pScene->getExtents(pContext);
   
    KRMat4 shadowvp;
    shadowvp.rotate(sun_pitch, X_AXIS);
    shadowvp.rotate(sun_yaw, Y_AXIS);
    shadowvp.invert();
    shadowvp.scale(1.0, 1.0, -1.0);
    
    KRVector3 minPointFrustrum = KRMat4::Dot(shadowvp, m_vertices[0]), maxPointFrustrum = minPointFrustrum;
    for(int iVertex=1; iVertex < 8; iVertex++) {
        KRVector3 v = KRMat4::Dot(shadowvp, m_vertices[iVertex]);
        if(v.x < minPointFrustrum.x) {
            minPointFrustrum.x = v.x;
        }
        if(v.y < minPointFrustrum.y) {
            minPointFrustrum.y = v.y;
        }
        if(v.z < minPointFrustrum.z) {
            minPointFrustrum.z = v.z;
        }
        if(v.x > maxPointFrustrum.x) {
            maxPointFrustrum.x = v.x;
        }
        if(v.y > maxPointFrustrum.y) {
            maxPointFrustrum.y = v.y;
        }
        if(v.z > maxPointFrustrum.z) {
            maxPointFrustrum.z = v.z;
        }
    }
    
    
    KRVector3 minPointScene = KRMat4::Dot(shadowvp, sceneVolume.m_vertices[0]), maxPointScene = minPointScene;
    for(int iVertex=1; iVertex < 8; iVertex++) {
        KRVector3 v = KRMat4::Dot(shadowvp, sceneVolume.m_vertices[iVertex]);
        if(v.x < minPointScene.x) {
            minPointScene.x = v.x;
        }
        if(v.y < minPointScene.y) {
            minPointScene.y = v.y;
        }
        if(v.z < minPointScene.z) {
            minPointScene.z = v.z;
        }
        if(v.x > maxPointScene.x) {
            maxPointScene.x = v.x;
        }
        if(v.y > maxPointScene.y) {
            maxPointScene.y = v.y;
        }
        if(v.z > maxPointScene.z) {
            maxPointScene.z = v.z;
        }
    }
    
    // Include potential shadow casters outside of view frustrum
    minPointFrustrum.z = minPointScene.z;
    
    if(maxPointScene.z < maxPointFrustrum.z) {
        maxPointFrustrum.z = maxPointScene.z;
    }
    
    /*
    // Include potential shadow casters outside of view frustrum
    GLfloat maxFrustrumDepth = maxPointFrustrum.z;
    
    for(int i=0; i<8; i++) {
        KRVector3 v = shadowvp.dot(sceneVolume.m_vertices[i]);
        if(i == 0) {
            minPointFrustrum.z = v.z;
            maxPointFrustrum.z = v.z;
        } else {
            if(v.z < minPointFrustrum.z) {
                minPointFrustrum.z = v.z;
            }
            if(v.z > maxPointFrustrum.z) {
                maxPointFrustrum.z = v.z;
            }
        }
    }
    if(maxPointFrustrum.z > maxFrustrumDepth) {
        maxPointFrustrum.z = maxFrustrumDepth;
    }
     */
    
    shadowvp.translate(-minPointFrustrum.x, -minPointFrustrum.y, -minPointFrustrum.z);
    shadowvp.scale(2.0/(maxPointFrustrum.x - minPointFrustrum.x), 2.0/(maxPointFrustrum.y - minPointFrustrum.y), 1.0/(maxPointFrustrum.z - minPointFrustrum.z));
    shadowvp.translate(-1.0, -1.0, 0.0);
    return shadowvp;
    
}
