//
//  Vector2.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 12-03-22.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "public/kraken.h"

namespace kraken {

Vector2::Vector2() {
    x = 0.0;
    y = 0.0;
}

Vector2::Vector2(float X, float Y) {
    x = X;
    y = Y;
}

Vector2::Vector2(float v) {
    x = v;
    y = v;
}

Vector2::Vector2(float *v) {
    x = v[0];
    y = v[1];
}

Vector2::Vector2(const Vector2 &v) {
    x = v.x;
    y = v.y;
}


// Vector2 swizzle getters
Vector2 Vector2::yx() const
{
    return Vector2(y,x);
}

// Vector2 swizzle setters
void Vector2::yx(const Vector2 &v)
{
    y = v.x;
    x = v.y;
}

Vector2 Vector2::Min() {
    return Vector2(-std::numeric_limits<float>::max());
}

Vector2 Vector2::Max() {
    return Vector2(std::numeric_limits<float>::max());
}

Vector2 Vector2::Zero() {
    return Vector2(0.0f);
}

Vector2 Vector2::One() {
    return Vector2(1.0f);
}

Vector2::~Vector2() {

}

Vector2& Vector2::operator =(const Vector2& b) {
    x = b.x;
    y = b.y;
    return *this;
}

Vector2 Vector2::operator +(const Vector2& b) const {
    return Vector2(x + b.x, y + b.y);
}

Vector2 Vector2::operator -(const Vector2& b) const {
    return Vector2(x - b.x, y - b.y);
}

Vector2 Vector2::operator +() const {
    return *this;
}

Vector2 Vector2::operator -() const {
    return Vector2(-x, -y);
}

Vector2 Vector2::operator *(const float v) const {
    return Vector2(x * v, y * v);
}

Vector2 Vector2::operator /(const float v) const {
    float inv_v = 1.0f / v;
    return Vector2(x * inv_v, y * inv_v);
}

Vector2& Vector2::operator +=(const Vector2& b) {
    x += b.x;
    y += b.y;
    return *this;
}

Vector2& Vector2::operator -=(const Vector2& b) {
    x -= b.x;
    y -= b.y;
    return *this;
}



Vector2& Vector2::operator *=(const float v) {
    x *= v;
    y *= v;
    return *this;
}

Vector2& Vector2::operator /=(const float v) {
    float inv_v = 1.0f / v;
    x *= inv_v;
    y *= inv_v;
    return *this;
}


bool Vector2::operator ==(const Vector2& b) const {
    return x == b.x && y == b.y;
}

bool Vector2::operator !=(const Vector2& b) const {
    return x != b.x || y != b.y;
}

bool Vector2::operator >(const Vector2& b) const
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

bool Vector2::operator <(const Vector2& b) const
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

float& Vector2::operator[] (unsigned i) {
    switch(i) {
        case 0:
            return x;
        case 1:
        default:
            return y;
    }
}

float Vector2::operator[](unsigned i) const {
    switch(i) {
        case 0:
            return x;
        case 1:
        default:
            return y;
    }
}

void Vector2::normalize() {
    float inv_magnitude = 1.0f / sqrtf(x * x + y * y);
    x *= inv_magnitude;
    y *= inv_magnitude;
}

float Vector2::sqrMagnitude() const {
    return x * x + y * y;
}

float Vector2::magnitude() const {
    return sqrtf(x * x + y * y);
}


Vector2 Vector2::Normalize(const Vector2 &v) {
    float inv_magnitude = 1.0f / sqrtf(v.x * v.x + v.y * v.y);
    return Vector2(v.x * inv_magnitude, v.y * inv_magnitude);
}

float Vector2::Cross(const Vector2 &v1, const Vector2 &v2) {
    return v1.x * v2.y - v1.y * v2.x;
}

float Vector2::Dot(const Vector2 &v1, const Vector2 &v2) {
    return v1.x * v2.x + v1.y * v2.y;
}

} // namepsace kraken
