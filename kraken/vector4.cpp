//
//  Vector4.cpp
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

#include "public/kraken.h"

namespace kraken {

const Vector4 Vector4_ZERO(0.0f, 0.0f, 0.0f, 0.0f);

//default constructor
Vector4::Vector4()
{
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
    w = 0.0f;
}

Vector4::Vector4(const Vector4 &v) {
    x = v.x;
    y = v.y;
    z = v.z;
    w = v.w;
}

Vector4::Vector4(const Vector3 &v, float W) {
    x = v.x;
    y = v.y;
    z = v.z;
    w = W;
}

Vector4::Vector4(float *v) {
    x = v[0];
    y = v[1];
    z = v[2];
    w = v[3];
}

Vector4 Vector4::Min() {
    return Vector4(-std::numeric_limits<float>::max());
}

Vector4 Vector4::Max() {
    return Vector4(std::numeric_limits<float>::max());
}

const Vector4 &Vector4::Zero() {
    return Vector4_ZERO;
}

Vector4 Vector4::One() {
    return Vector4(1.0f, 1.0f, 1.0f, 1.0f);
}

Vector4 Vector4::Forward() {
    return Vector4(0.0f, 0.0f, 1.0f, 1.0f);
}

Vector4 Vector4::Backward() {
    return Vector4(0.0f, 0.0f, -1.0f, 1.0f);
}

Vector4 Vector4::Up() {
    return Vector4(0.0f, 1.0f, 0.0f, 1.0f);
}

Vector4 Vector4::Down() {
    return Vector4(0.0f, -1.0f, 0.0f, 1.0f);
}

Vector4 Vector4::Left() {
    return Vector4(-1.0f, 0.0f, 0.0f, 1.0f);
}

Vector4 Vector4::Right() {
    return Vector4(1.0f, 0.0f, 0.0f, 1.0f);
}

Vector4 Vector4::Lerp(const Vector4 &v1, const Vector4 &v2, float d) {
    return v1 + (v2 - v1) * d;
}

Vector4 Vector4::Slerp(const Vector4 &v1, const Vector4 &v2, float d) {
    // From: http://keithmaggio.wordpress.com/2011/02/15/math-magician-lerp-slerp-and-nlerp/
    // Dot product - the cosine of the angle between 2 vectors.
    float dot = Vector4::Dot(v1, v2);
    // Clamp it to be in the range of Acos()
    if(dot < -1.0f) dot = -1.0f;
    if(dot > 1.0f) dot = 1.0f;
    // Acos(dot) returns the angle between start and end,
    // And multiplying that by percent returns the angle between
    // start and the final result.
    float theta = acos(dot)*d;
    Vector4 RelativeVec = v2 - v1*dot;
    RelativeVec.normalize();     // Orthonormal basis
    // The final result.
    return ((v1*cos(theta)) + (RelativeVec*sin(theta)));
}

void Vector4::OrthoNormalize(Vector4 &normal, Vector4 &tangent) {
    // Gram-Schmidt Orthonormalization
    normal.normalize();
    Vector4 proj = normal * Dot(tangent, normal);
    tangent = tangent - proj;
    tangent.normalize();
}

Vector4::Vector4(float v) {
    x = v;
    y = v;
    z = v;
    w = v;
}

Vector4::Vector4(float X, float Y, float Z, float W)
{
    x = X;
    y = Y;
    z = Z;
    w = W;
}

Vector4::~Vector4()
{
}

Vector4& Vector4::operator =(const Vector4& b) {
    x = b.x;
    y = b.y;
    z = b.z;
    w = b.w;
    return *this;
}
Vector4 Vector4::operator +(const Vector4& b) const {
    return Vector4(x + b.x, y + b.y, z + b.z, w + b.w);
}
Vector4 Vector4::operator -(const Vector4& b) const {
    return Vector4(x - b.x, y - b.y, z - b.z, w - b.w);
}
Vector4 Vector4::operator +() const {
    return *this;
}
Vector4 Vector4::operator -() const {
    return Vector4(-x, -y, -z, -w);
}

Vector4 Vector4::operator *(const float v) const {
    return Vector4(x * v, y * v, z * v, w * v);
}

Vector4 Vector4::operator /(const float v) const {
    return Vector4(x / v, y / v, z / v, w/ v);
}

Vector4& Vector4::operator +=(const Vector4& b) {
    x += b.x;
    y += b.y;
    z += b.z;
    w += b.w;
    
    return *this;
}

Vector4& Vector4::operator -=(const Vector4& b) {
    x -= b.x;
    y -= b.y;
    z -= b.z;
    w -= b.w;
    
    return *this;
}

Vector4& Vector4::operator *=(const float v) {
    x *= v;
    y *= v;
    z *= v;
    w *= v;
    
    return *this;
}

Vector4& Vector4::operator /=(const float v) {
    float inv_v = 1.0f / v;
    x *= inv_v;
    y *= inv_v;
    z *= inv_v;
    w *= inv_v;
    
    return *this;
}

bool Vector4::operator ==(const Vector4& b) const {
    return x == b.x && y == b.y && z == b.z && w == b.w;
    
}
bool Vector4::operator !=(const Vector4& b) const {
    return x != b.x || y != b.y || z != b.z || w != b.w;
}

float& Vector4::operator[](unsigned i) {
    switch(i) {
        case 0:
            return x;
        case 1:
            return y;
        case 2:
            return z;
        default:
        case 3:
            return w;
    }
}

float Vector4::operator[](unsigned i) const {
    switch(i) {
        case 0:
            return x;
        case 1:
            return y;
        case 2:
            return z;
        default:
        case 3:
            return w;
    }
}

float Vector4::sqrMagnitude() const {
    // calculate the square of the magnitude (useful for comparison of magnitudes without the cost of a sqrt() function)
    return x * x + y * y + z * z + w * w;
}

float Vector4::magnitude() const {
    return sqrtf(x * x + y * y + z * z + w * w);
}

void Vector4::normalize() {
    float inv_magnitude = 1.0f / sqrtf(x * x + y * y + z * z + w * w);
    x *= inv_magnitude;
    y *= inv_magnitude;
    z *= inv_magnitude;
    w *= inv_magnitude;
}
Vector4 Vector4::Normalize(const Vector4 &v) {
    float inv_magnitude = 1.0f / sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
    return Vector4(v.x * inv_magnitude, v.y * inv_magnitude, v.z * inv_magnitude, v.w * inv_magnitude);
}


float Vector4::Dot(const Vector4 &v1, const Vector4 &v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
}

bool Vector4::operator >(const Vector4& b) const
{
    // Comparison operators are implemented to allow insertion into sorted containers such as std::set
    if(x != b.x) return x > b.x;
    if(y != b.y) return y > b.y;
    if(z != b.z) return z > b.z;
    if(w != b.w) return w > b.w;
    return false;
}

bool Vector4::operator <(const Vector4& b) const
{
    // Comparison operators are implemented to allow insertion into sorted containers such as std::set
    if(x != b.x) return x < b.x;
    if(y != b.y) return y < b.y;
    if(z != b.z) return z < b.z;
    if(w != b.w) return w < b.w;
    return false;
}

} // namespace kraken
