//
//  KRVector2.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 12-03-22.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KREngine-common.h"

#include "KRVector2.h"

KRVector2::KRVector2() {
    x = 0.0;
    y = 0.0;
}

KRVector2::KRVector2(float X, float Y) {
    x = X;
    y = Y;
}

KRVector2::KRVector2(float v) {
    x = v;
    y = v;
}

KRVector2::KRVector2(float *v) {
    x = v[0];
    y = v[1];
}

KRVector2::KRVector2(const KRVector2 &v) {
    x = v.x;
    y = v.y;
}

KRVector2 KRVector2::Min() {
    return KRVector2(-std::numeric_limits<float>::max());
}

KRVector2 KRVector2::Max() {
    return KRVector2(std::numeric_limits<float>::max());
}

KRVector2 KRVector2::Zero() {
    return KRVector2(0.0f);
}

KRVector2 KRVector2::One() {
    return KRVector2(1.0f);
}

KRVector2::~KRVector2() {

}

KRVector2& KRVector2::operator =(const KRVector2& b) {
    x = b.x;
    y = b.y;
    return *this;
}

KRVector2 KRVector2::operator +(const KRVector2& b) const {
    return KRVector2(x + b.x, y + b.y);
}

KRVector2 KRVector2::operator -(const KRVector2& b) const {
    return KRVector2(x - b.x, y - b.y);
}

KRVector2 KRVector2::operator +() const {
    return *this;
}

KRVector2 KRVector2::operator -() const {
    return KRVector2(-x, -y);
}

KRVector2 KRVector2::operator *(const float v) const {
    return KRVector2(x * v, y * v);
}

KRVector2 KRVector2::operator /(const float v) const {
    float inv_v = 1.0f / v;
    return KRVector2(x * inv_v, y * inv_v);
}

KRVector2& KRVector2::operator +=(const KRVector2& b) {
    x += b.x;
    y += b.y;
    return *this;
}

KRVector2& KRVector2::operator -=(const KRVector2& b) {
    x -= b.x;
    y -= b.y;
    return *this;
}



KRVector2& KRVector2::operator *=(const float v) {
    x *= v;
    y *= v;
    return *this;
}

KRVector2& KRVector2::operator /=(const float v) {
    float inv_v = 1.0f / v;
    x *= inv_v;
    y *= inv_v;
    return *this;
}


bool KRVector2::operator ==(const KRVector2& b) const {
    return x == b.x && y == b.y;
}

bool KRVector2::operator !=(const KRVector2& b) const {
    return x != b.x || y != b.y;
}

bool KRVector2::operator >(const KRVector2& b) const
{
    // Comparison operators are implemented to allow insertion into sorted containers such as std::set
    if(x > b.x) {
        return true;
    } else if(x < b.x) {
        return false;
    } else if(y > b.y) {
        return true;
    } else {
        return false;
    }
}

bool KRVector2::operator <(const KRVector2& b) const
{
    // Comparison operators are implemented to allow insertion into sorted containers such as std::set
    if(x < b.x) {
        return true;
    } else if(x > b.x) {
        return false;
    } else if(y < b.y) {
        return true;
    } else {
        return false;
    }
}

float& KRVector2::operator[] (unsigned i) {
    switch(i) {
        case 0:
            return x;
        case 1:
        default:
            return y;
    }
}

float KRVector2::operator[](unsigned i) const {
    switch(i) {
        case 0:
            return x;
        case 1:
        default:
            return y;
    }
}

void KRVector2::normalize() {
    float inv_magnitude = 1.0f / sqrtf(x * x + y * y);
    x *= inv_magnitude;
    y *= inv_magnitude;
}

float KRVector2::sqrMagnitude() const {
    return x * x + y * y;
}

float KRVector2::magnitude() const {
    return sqrtf(x * x + y * y);
}


KRVector2 KRVector2::Normalize(const KRVector2 &v) {
    float inv_magnitude = 1.0f / sqrtf(v.x * v.x + v.y * v.y);
    return KRVector2(v.x * inv_magnitude, v.y * inv_magnitude);
}

float KRVector2::Cross(const KRVector2 &v1, const KRVector2 &v2) {
    return v1.x * v2.y - v1.y * v2.x;
}

float KRVector2::Dot(const KRVector2 &v1, const KRVector2 &v2) {
    return v1.x * v2.x + v1.y * v2.y;
}

void KRVector2::setUniform(GLint location) const
{
    if(location != -1) GLDEBUG(glUniform2f(location, x, y));
}
