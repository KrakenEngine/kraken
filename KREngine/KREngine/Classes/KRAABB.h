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

class KRAABB {
public:
    KRAABB(const KRVector3 &minPoint, const KRVector3 &maxPoint);
    ~KRAABB();
    
    KRVector3 center();
    KRVector3 size();
    
    KRAABB& operator =(const KRAABB& b);
    bool operator ==(const KRAABB& b) const;
    bool operator !=(const KRAABB& b) const;
    
    // Comparison operators are implemented to allow insertion into sorted containers such as std::set
    bool operator >(const KRAABB& b) const;
    bool operator <(const KRAABB& b) const;
    
    KRVector3 min;
    KRVector3 max;
};


#endif /* defined(KRAABB_H) */
