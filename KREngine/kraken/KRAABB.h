//
//  KRAABB.h
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-08-30.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

// Axis aligned bounding box

#ifndef KRAABB_H
#define KRAABB_H

#include "KRVector3.h"

class KRMat4;
class KRVector2;

class KRAABB {
public:
    KRAABB(const KRVector3 &minPoint, const KRVector3 &maxPoint);
    KRAABB(const KRVector3 &corner1, const KRVector3 &corner2, const KRMat4 &modelMatrix);
    KRAABB();
    ~KRAABB();
    
    void scale(const KRVector3 &s);
    void scale(float s);
    
    KRVector3 center() const;
    KRVector3 size() const;
    float volume() const;
    bool intersects(const KRAABB& b) const;
    bool contains(const KRAABB &b) const;
    bool contains(const KRVector3 &v) const;
    
    bool intersectsLine(const KRVector3 &v1, const KRVector3 &v2) const;
    bool intersectsRay(const KRVector3 &v1, const KRVector3 &dir) const;
    void encapsulate(const KRAABB & b);
    
    KRAABB& operator =(const KRAABB& b);
    bool operator ==(const KRAABB& b) const;
    bool operator !=(const KRAABB& b) const;
    
    // Comparison operators are implemented to allow insertion into sorted containers such as std::set
    bool operator >(const KRAABB& b) const;
    bool operator <(const KRAABB& b) const;
    
    KRVector3 min;
    KRVector3 max;
    
    static KRAABB Infinite();
    static KRAABB Zero();
    
    float longest_radius() const;
    KRVector3 nearestPoint(const KRVector3 & v) const;
};


#endif /* defined(KRAABB_H) */
