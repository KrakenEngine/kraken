//
//  KRBoundingVolume.h
//  KREngine
//
//  Created by Kearwood Gilbert on 11-09-29.
//  Copyright 2011 Kearwood Software. All rights reserved.
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
