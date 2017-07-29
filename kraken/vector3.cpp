//
//  Vector3.cpp
//  KREngine
//
//  Copyright 2012 Kearwood Gilbert. All rights reserved.
//  
//  Redistribution and use in source and binary forms, with or without modification, are
//  permitted provided that the following conditions are met:
//  
//  1. Redistributions of source code must retain the above copyright notice, this list of
//  conditions and the following disclaimer.
//  
//  2. Redistributions in binary form must reproduce the above copyright notice, this list
//  of conditions and the following disclaimer in the documentation and/or other materials
//  provided with the distribution.
//  
//  THIS SOFTWARE IS PROVIDED BY KEARWOOD GILBERT ''AS IS'' AND ANY EXPRESS OR IMPLIED
//  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL KEARWOOD GILBERT OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//  
//  The views and conclusions contained in the software and documentation are those of the
//  authors and should not be interpreted as representing official policies, either expressed
//  or implied, of Kearwood Gilbert.
//

#include "KREngine-common.h"
#include "public/kraken.h"

const Vector3 Vector3_ZERO(0.0f, 0.0f, 0.0f);

//default constructor
Vector3::Vector3()
{
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
}

Vector3::Vector3(const Vector3 &v) {
    x = v.x;
    y = v.y;
    z = v.z;
}

Vector3::Vector3(const KRVector4 &v) {
    x = v.x;
    y = v.y;
    z = v.z;
}

Vector3::Vector3(float *v) {
    x = v[0];
    y = v[1];
    z = v[2];
}

Vector3::Vector3(double *v) {
    x = (float)v[0];
    y = (float)v[1];
    z = (float)v[2];
}

Vector2 Vector3::xx() const
{
    return Vector2(x,x);
}

Vector2 Vector3::xy() const
{
    return Vector2(x,y);
}

Vector2 Vector3::xz() const
{
    return Vector2(x,z);
}

Vector2 Vector3::yx() const
{
    return Vector2(y,x);
}

Vector2 Vector3::yy() const
{
    return Vector2(y,y);
}

Vector2 Vector3::yz() const
{
    return Vector2(y,z);
}

Vector2 Vector3::zx() const
{
    return Vector2(z,x);
}

Vector2 Vector3::zy() const
{
    return Vector2(z,y);
}

Vector2 Vector3::zz() const
{
    return Vector2(z,z);
}

void Vector3::xy(const Vector2 &v)
{
    x = v.x;
    y = v.y;
}

void Vector3::xz(const Vector2 &v)
{
    x = v.x;
    z = v.y;
}

void Vector3::yx(const Vector2 &v)
{
    y = v.x;
    x = v.y;
}

void Vector3::yz(const Vector2 &v)
{
    y = v.x;
    z = v.y;
}

void Vector3::zx(const Vector2 &v)
{
    z = v.x;
    x = v.y;
}

void Vector3::zy(const Vector2 &v)
{
    z = v.x;
    y = v.y;
}

Vector3 Vector3::Min() {
    return Vector3(-std::numeric_limits<float>::max());
}

Vector3 Vector3::Max() {
    return Vector3(std::numeric_limits<float>::max());
}

const Vector3 &Vector3::Zero() {
    return Vector3_ZERO;
}

Vector3 Vector3::One() {
    return Vector3(1.0f, 1.0f, 1.0f);
}

Vector3 Vector3::Forward() {
    return Vector3(0.0f, 0.0f, 1.0f);
}

Vector3 Vector3::Backward() {
    return Vector3(0.0f, 0.0f, -1.0f);
}

Vector3 Vector3::Up() {
    return Vector3(0.0f, 1.0f, 0.0f);
}

Vector3 Vector3::Down() {
    return Vector3(0.0f, -1.0f, 0.0f);
}

Vector3 Vector3::Left() {
    return Vector3(-1.0f, 0.0f, 0.0f);
}

Vector3 Vector3::Right() {
    return Vector3(1.0f, 0.0f, 0.0f);
}


void Vector3::scale(const Vector3 &v)
{
    x *= v.x;
    y *= v.y;
    z *= v.z;
}

Vector3 Vector3::Scale(const Vector3 &v1, const Vector3 &v2)
{
    return Vector3(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
}

Vector3 Vector3::Lerp(const Vector3 &v1, const Vector3 &v2, float d) {
    return v1 + (v2 - v1) * d;
}

Vector3 Vector3::Slerp(const Vector3 &v1, const Vector3 &v2, float d) {
    // From: http://keithmaggio.wordpress.com/2011/02/15/math-magician-lerp-slerp-and-nlerp/
    // Dot product - the cosine of the angle between 2 vectors.
    float dot = Vector3::Dot(v1, v2);     
    // Clamp it to be in the range of Acos()
    if(dot < -1.0f) dot = -1.0f;
    if(dot > 1.0f) dot = 1.0f;
    // Acos(dot) returns the angle between start and end,
    // And multiplying that by percent returns the angle between
    // start and the final result.
    float theta = acos(dot)*d;
    Vector3 RelativeVec = v2 - v1*dot;
    RelativeVec.normalize();     // Orthonormal basis
    // The final result.
    return ((v1*cos(theta)) + (RelativeVec*sin(theta)));
}

void Vector3::OrthoNormalize(Vector3 &normal, Vector3 &tangent) {
    // Gram-Schmidt Orthonormalization
    normal.normalize();
    Vector3 proj = normal * Dot(tangent, normal);
    tangent = tangent - proj;
    tangent.normalize();
}

Vector3::Vector3(float v) {
    x = v;
    y = v;
    z = v;
}

Vector3::Vector3(float X, float Y, float Z)
{
    x = X;
    y = Y;
    z = Z;
}

Vector3::~Vector3()
{
}

Vector3& Vector3::operator =(const Vector3& b) {
    x = b.x;
    y = b.y;
    z = b.z;
    return *this;
}

Vector3& Vector3::operator =(const KRVector4 &b) {
    x = b.x;
    y = b.y;
    z = b.z;
    return *this;
}

Vector3 Vector3::operator +(const Vector3& b) const {
    return Vector3(x + b.x, y + b.y, z + b.z);
}
Vector3 Vector3::operator -(const Vector3& b) const {
    return Vector3(x - b.x, y - b.y, z - b.z);
}
Vector3 Vector3::operator +() const {
    return *this;
}
Vector3 Vector3::operator -() const {
    return Vector3(-x, -y, -z);
}

Vector3 Vector3::operator *(const float v) const {
    return Vector3(x * v, y * v, z * v);
}

Vector3 Vector3::operator /(const float v) const {
    float inv_v = 1.0f / v;
    return Vector3(x * inv_v, y * inv_v, z * inv_v);
}

Vector3& Vector3::operator +=(const Vector3& b) {
    x += b.x;
    y += b.y;
    z += b.z;
    
    return *this;
}

Vector3& Vector3::operator -=(const Vector3& b) {
    x -= b.x;
    y -= b.y;
    z -= b.z;
    
    return *this;
}

Vector3& Vector3::operator *=(const float v) {
    x *= v;
    y *= v;
    z *= v;
    
    return *this;
}

Vector3& Vector3::operator /=(const float v) {
    float inv_v = 1.0f / v;
    x *= inv_v;
    y *= inv_v;
    z *= inv_v;
    
    return *this;
}

bool Vector3::operator ==(const Vector3& b) const {
    return x == b.x && y == b.y && z == b.z;
    
}
bool Vector3::operator !=(const Vector3& b) const {
    return x != b.x || y != b.y || z != b.z;
}

float& Vector3::operator[](unsigned i) {
    switch(i) {
        case 0:
            return x;
        case 1:
            return y;
        default:
        case 2:
            return z;
    }
}

float Vector3::operator[](unsigned i) const {
    switch(i) {
        case 0:
            return x;
        case 1:
            return y;
        case 2:
        default:
            return z;
    }
}

float Vector3::sqrMagnitude() const {
    // calculate the square of the magnitude (useful for comparison of magnitudes without the cost of a sqrt() function)
    return x * x + y * y + z * z;
}

float Vector3::magnitude() const {
    return sqrtf(x * x + y * y + z * z);
}

void Vector3::normalize() {
    float inv_magnitude = 1.0f / sqrtf(x * x + y * y + z * z);
    x *= inv_magnitude;
    y *= inv_magnitude;
    z *= inv_magnitude;
}
Vector3 Vector3::Normalize(const Vector3 &v) {
    float inv_magnitude = 1.0f / sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    return Vector3(v.x * inv_magnitude, v.y * inv_magnitude, v.z * inv_magnitude);
}

Vector3 Vector3::Cross(const Vector3 &v1, const Vector3 &v2) {
    return Vector3(v1.y * v2.z - v1.z * v2.y,
                     v1.z * v2.x - v1.x * v2.z,
                     v1.x * v2.y - v1.y * v2.x);
}

float Vector3::Dot(const Vector3 &v1, const Vector3 &v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

bool Vector3::operator >(const Vector3& b) const
{
    // Comparison operators are implemented to allow insertion into sorted containers such as std::set
    if(x > b.x) {
        return true;
    } else if(x < b.x) {
        return false;
    } else if(y > b.y) {
        return true;
    } else if(y < b.y) {
        return false;
    } else if(z > b.z) {
        return true;
    } else {
        return false;
    }
}

bool Vector3::operator <(const Vector3& b) const
{
    // Comparison operators are implemented to allow insertion into sorted containers such as std::set
    if(x < b.x) {
        return true;
    } else if(x > b.x) {
        return false;
    } else if(y < b.y) {
        return true;
    } else if(y > b.y) {
        return false;
    } else if(z < b.z) {
        return true;
    } else {
        return false;
    }
}
