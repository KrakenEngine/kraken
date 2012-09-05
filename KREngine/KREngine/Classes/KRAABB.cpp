//
//  KRAABB.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-08-30.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KRAABB.h"

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