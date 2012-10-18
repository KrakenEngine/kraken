//
//  KRVector2.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 12-03-22.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include <iostream>
#include <math.h>

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
    return KRVector2(x / v, y / v);
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
    x /= v;
    y /= v;
    return *this;
}


bool KRVector2::operator ==(const KRVector2& b) const {
    return x == b.x && y == b.y;
}

bool KRVector2::operator !=(const KRVector2& b) const {
    return x != b.x || y != b.y;
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
    float magnitude = sqrtf(x * x + y * y);
    x /= magnitude;
    y /= magnitude;
}

float KRVector2::sqrMagnitude() const {
    return x * x + y * y;
}

float KRVector2::magnitude() const {
    return sqrtf(x * x + y * y);
}


KRVector2 KRVector2::Normalize(const KRVector2 &v) {
    float magnitude = sqrtf(v.x * v.x + v.y * v.y);
    return KRVector2(v.x / magnitude, v.y / magnitude);
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
