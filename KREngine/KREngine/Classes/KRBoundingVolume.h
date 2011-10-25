//
//  KRBoundingVolume.h
//  KREngine
//
//  Copyright 2011 Kearwood Gilbert. All rights reserved.
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


#ifndef KRBOUNDINGVOLUME_H
#define KRBOUNDINGVOLUME_H


#import "KRVector3.h"
#import "KRMat4.h"
#import "KRScene.h"

class KRScene;

class KRBoundingVolume {
public:
    KRBoundingVolume(const Vector3 *pVertices);
    KRBoundingVolume(const Vector3 &corner1, const Vector3 &corner2, const KRMat4 modelMatrix);
    KRBoundingVolume(const KRMat4 &matView, GLfloat fov, GLfloat aspect, GLfloat nearz, GLfloat farz);
    ~KRBoundingVolume();
    
    KRBoundingVolume(const KRBoundingVolume& p);
    KRBoundingVolume& operator = ( const KRBoundingVolume& p );
    
    KRBoundingVolume get_union(const KRBoundingVolume &p) const;
    bool test_intersect(const KRBoundingVolume &p) const;
    
    KRMat4 calcShadowProj(KRScene *pScene, GLfloat sun_yaw, GLfloat sun_pitch) const;
private:
    Vector3 m_vertices[8];
};

#endif
