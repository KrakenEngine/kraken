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

class KRAABB {
public:
    KRAABB(const KRVector3 &minPoint, const KRVector3 &maxPoint);
    ~KRAABB();
    
    KRVector3 center() const;
    KRVector3 size() const;
    bool intersects(const KRAABB& b) const;
    bool contains(const KRAABB &b) const;
    bool contains(const KRVector3 &v) const;
    bool visible(const KRMat4 &matViewProjection) const;
    
    KRAABB& operator =(const KRAABB& b);
    bool operator ==(const KRAABB& b) const;
    bool operator !=(const KRAABB& b) const;
    
    // Comparison operators are implemented to allow insertion into sorted containers such as std::set
    bool operator >(const KRAABB& b) const;
    bool operator <(const KRAABB& b) const;
    
    KRVector3 min;
    KRVector3 max;
    
    static KRAABB Infinite();
};


#endif /* defined(KRAABB_H) */
