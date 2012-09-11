//
//  KRAABB.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-08-30.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KRAABB.h"
#include "KRMat4.h"

KRAABB::KRAABB(const KRVector3 &minPoint, const KRVector3 &maxPoint)
{
    min = minPoint;
    max = maxPoint;
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

KRAABB KRAABB::Infinite()
{
    return KRAABB(KRVector3::Min(), KRVector3::Max());
}

bool KRAABB::visible(const KRMat4 &matViewProjection) const
{
    // test if bounding box would be within the visible range of the clip space transformed by matViewProjection
    // This is used for view frustrum culling
    
    KRVector3 minCorner, maxCorner;
    
    int outside_count[6] = {0, 0, 0, 0, 0, 0};
    
    for(int iCorner=0; iCorner<8; iCorner++) {
        KRVector3 cornerVertex = KRVector3(
           (iCorner & 1) == 0 ? min.x : max.x,
           (iCorner & 2) == 0 ? min.y : max.y,
           (iCorner & 4) == 0 ? min.z : max.z);
        
        cornerVertex = KRMat4::Dot(matViewProjection, cornerVertex);
        float cornerVertexW = KRMat4::DotW(matViewProjection, cornerVertex);
        
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
        fprintf(stderr, "AABB culled:  %i%i%i%i%i%i out, (%f, %f, %f) - (%f, %f, %f)\n", outside_count[0], outside_count[1], outside_count[2], outside_count[3], outside_count[4], outside_count[5], min.x, min.y, min.z, max.x, max.y, max.z);
    } else {
        fprintf(stderr, "AABB visible: %i%i%i%i%i%i out, (%f, %f, %f) - (%f, %f, %f)\n", outside_count[0], outside_count[1], outside_count[2], outside_count[3], outside_count[4], outside_count[5], min.x, min.y, min.z, max.x, max.y, max.z);
    }
    return is_visible;
}
