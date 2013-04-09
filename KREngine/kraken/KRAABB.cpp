//
//  KRAABB.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-08-30.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KRAABB.h"
#include "KRMat4.h"
#include "KRVector2.h"
#include "assert.h"

KRAABB::KRAABB()
{
    min = KRVector3::Min();
    max = KRVector3::Max();
}

KRAABB::KRAABB(const KRVector3 &minPoint, const KRVector3 &maxPoint)
{
    min = minPoint;
    max = maxPoint;
}

KRAABB::KRAABB(const KRVector3 &corner1, const KRVector3 &corner2, const KRMat4 &modelMatrix)
{
    for(int iCorner=0; iCorner<8; iCorner++) {
        KRVector3 sourceCornerVertex = KRMat4::DotWDiv(modelMatrix, KRVector3(
                                                 (iCorner & 1) == 0 ? corner1.x : corner2.x,
                                                 (iCorner & 2) == 0 ? corner1.y : corner2.y,
                                                 (iCorner & 4) == 0 ? corner1.z : corner2.z));
        
        
        if(iCorner == 0) {
            min = sourceCornerVertex;
            max = sourceCornerVertex;
        } else {
            if(sourceCornerVertex.x < min.x) min.x = sourceCornerVertex.x;
            if(sourceCornerVertex.y < min.y) min.y = sourceCornerVertex.y;
            if(sourceCornerVertex.z < min.z) min.z = sourceCornerVertex.z;
            if(sourceCornerVertex.x > max.x) max.x = sourceCornerVertex.x;
            if(sourceCornerVertex.y > max.y) max.y = sourceCornerVertex.y;
            if(sourceCornerVertex.z > max.z) max.z = sourceCornerVertex.z;
        }
    }
}

KRAABB::~KRAABB()
{
    
}

KRAABB& KRAABB::operator =(const KRAABB& b)
{
    min = b.min;
    max = b.max;
    
    return *this;
}

bool KRAABB::operator ==(const KRAABB& b) const
{
    return min == b.min && max == b.max;
}

bool KRAABB::operator !=(const KRAABB& b) const
{
    return min != b.min || max != b.max;
}

KRVector3 KRAABB::center() const
{
    return (min + max) / 2.0f;
}

KRVector3 KRAABB::size() const
{
    return max - min;
}

float KRAABB::volume() const
{
    KRVector3 s = size();
    return s.x * s.y * s.z;
}

void KRAABB::scale(const KRVector3 &s)
{
    KRVector3 prev_center = center();
    KRVector3 prev_size = size();
    KRVector3 new_scale = KRVector3(prev_size.x * s.x, prev_size.y * s.y, prev_size.z * s.z) * 0.5f;
    min = prev_center - new_scale;
    max = prev_center + new_scale;
}

void KRAABB::scale(float s)
{
    scale(KRVector3(s));
}

bool KRAABB::operator >(const KRAABB& b) const
{
    // Comparison operators are implemented to allow insertion into sorted containers such as std::set
    if(min > b.min) {
        return true;
    } else if(min < b.min) {
        return false;
    } else if(max > b.max) {
        return true;
    } else {
        return false;
    }

}
bool KRAABB::operator <(const KRAABB& b) const
{
    // Comparison operators are implemented to allow insertion into sorted containers such as std::set
    
    if(min < b.min) {
        return true;
    } else if(min > b.min) {
        return false;
    } else if(max < b.max) {
        return true;
    } else {
        return false;
    }
}

bool KRAABB::intersects(const KRAABB& b) const
{
    // Return true if the two volumes intersect
    return min.x <= b.max.x && min.y <= b.max.y && min.z <= b.max.z && max.x >= b.min.x && max.y >= b.min.y && max.z >= b.max.z;
}

bool KRAABB::contains(const KRAABB &b) const
{
    // Return true if the passed KRAABB is entirely contained within this KRAABB
    return b.min.x >= min.x && b.min.y >= min.y && b.min.z >= min.z && b.max.x <= max.x && b.max.y <= max.y && b.max.z <= max.z;
}

bool KRAABB::contains(const KRVector3 &v) const
{
    return v.x >= min.x && v.x <= max.x && v.y >= min.y && v.y <= max.y && v.z >= min.z && v.z <= max.z;
}

KRAABB KRAABB::Infinite()
{
    return KRAABB(KRVector3::Min(), KRVector3::Max());
}

KRAABB KRAABB::Zero()
{
    return KRAABB(KRVector3::Zero(), KRVector3::Zero());
}

bool KRAABB::visible(const KRMat4 &matViewProjection) const
{
    // test if bounding box would be within the visible range of the clip space transformed by matViewProjection
    // This is used for view frustrum culling
    
    int outside_count[6] = {0, 0, 0, 0, 0, 0};
    
    for(int iCorner=0; iCorner<8; iCorner++) {
        KRVector3 sourceCornerVertex = KRVector3(
           (iCorner & 1) == 0 ? min.x : max.x,
           (iCorner & 2) == 0 ? min.y : max.y,
           (iCorner & 4) == 0 ? min.z : max.z);

        KRVector3 cornerVertex = KRMat4::Dot(matViewProjection, sourceCornerVertex);
        float cornerVertexW = KRMat4::DotW(matViewProjection, sourceCornerVertex);
        
        if(cornerVertex.x < -cornerVertexW) {
            outside_count[0]++;
        }
        if(cornerVertex.y < -cornerVertexW) {
            outside_count[1]++;
        }
        if(cornerVertex.z < -cornerVertexW) {
            outside_count[2]++;
        }
        if(cornerVertex.x > cornerVertexW) {
            outside_count[3]++;
        }
        if(cornerVertex.y > cornerVertexW) {
            outside_count[4]++;
        }
        if(cornerVertex.z > cornerVertexW) {
            outside_count[5]++;
        }
    }
    
    bool is_visible = true;
    for(int iFace=0; iFace < 6; iFace++) {
        if(outside_count[iFace] == 8) {
            is_visible = false;
        }
    }
    
    if(!is_visible) {
        //fprintf(stderr, "AABB culled:  %i%i%i%i%i%i out, (%f, %f, %f) - (%f, %f, %f)\n", outside_count[0], outside_count[1], outside_count[2], outside_count[3], outside_count[4], outside_count[5], min.x, min.y, min.z, max.x, max.y, max.z);
    } else {
        //fprintf(stderr, "AABB visible: %i%i%i%i%i%i out, (%f, %f, %f) - (%f, %f, %f)\n", outside_count[0], outside_count[1], outside_count[2], outside_count[3], outside_count[4], outside_count[5], min.x, min.y, min.z, max.x, max.y, max.z);
    }
    //is_visible = true;
    return is_visible;
}


float KRAABB::coverage(const KRMat4 &matVP, const KRVector2 viewportSize) const
{
    if(!visible(matVP)) {
        return 0.0f; // Culled out by view frustrum
    } else {
        KRVector2 screen_min;
        KRVector2 screen_max;
        // Loop through all corners and transform them to screen space
        for(int i=0; i<8; i++) {
            KRVector3 screen_pos = KRMat4::DotWDiv(matVP, KRVector3(i & 1 ? min.x : max.x, i & 2 ? min.y : max.y, i & 4 ? min.z : max.z));
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

float KRAABB::longest_radius() const
{
    float radius1 = (center() - min).magnitude();
    float radius2 = (max - center()).magnitude();
    return radius1 > radius2 ? radius1 : radius2;
}


bool KRAABB::intersectsLine(const KRVector3 &v1, const KRVector3 &v2) const
{
    KRVector3 dir = KRVector3::Normalize(v2 - v1);
    float length = (v2 - v1).magnitude();
    
    // EZ cases: if the ray starts inside the box, or ends inside
    // the box, then it definitely hits the box.
    // I'm using this code for ray tracing with an octree,
    // so I needed rays that start and end within an
    // octree node to COUNT as hits.
    // You could modify this test to (ray starts inside and ends outside)
    // to qualify as a hit if you wanted to NOT count totally internal rays
    if( contains( v1 ) || contains( v2 ) )
        return true ;
    
    // the algorithm says, find 3 t's,
    KRVector3 t ;
    
    // LARGEST t is the only one we need to test if it's on the face.
    for(int i = 0 ; i < 3 ; i++) {
        if( dir[i] > 0 ) { // CULL BACK FACE
            t[i] = ( min[i] - v1[i] ) / dir[i];
        } else {
            t[i] = ( max[i] - v1[i] ) / dir[i];
        }
    }
    
    int mi = 0;
    if(t[1] > t[mi]) mi = 1;
    if(t[2] > t[mi]) mi = 2;
    if(t[mi] >= 0 && t[mi] <= length) {
        KRVector3 pt = v1 + dir * t[mi];
        
        // check it's in the box in other 2 dimensions
        int o1 = ( mi + 1 ) % 3 ; // i=0: o1=1, o2=2, i=1: o1=2,o2=0 etc.
        int o2 = ( mi + 2 ) % 3 ;
        
        return pt[o1] >= min[o1] && pt[o1] <= max[o1] && pt[o2] >= min[o2] && pt[o2] <= max[o2];
    }
    
    return false ; // the ray did not hit the box.
}

bool KRAABB::intersectsRay(const KRVector3 &v1, const KRVector3 &dir) const
{
    // FINDME, TODO - Need to implement this
    return true;
}

void KRAABB::encapsulate(const KRAABB & b)
{
    if(b.min.x < min.x) min.x = b.min.x;
    if(b.min.y < min.y) min.y = b.min.y;
    if(b.min.z < min.z) min.z = b.min.z;
    
    if(b.max.x > max.x) max.x = b.max.x;
    if(b.max.y > max.y) max.y = b.max.y;
    if(b.max.z > max.z) max.z = b.max.z;
}

KRVector3 KRAABB::nearestPoint(const KRVector3 & v) const
{
    return KRVector3(KRCLAMP(v.x, min.x, max.x), KRCLAMP(v.y, min.y, max.y), KRCLAMP(v.z, min.z, max.z));
}
