//
//  KRVector3.cpp
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

#include "KRVector3.h"

#include <math.h>

//default constructor
KRVector3::KRVector3()
{
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
}

KRVector3::KRVector3(const KRVector3 &v) {
    x = v.x;
    y = v.y;
    z = v.z;
}

KRVector3 KRVector3::ZeroVector() {
    return KRVector3(0.0f, 0.0f, 0.0f);
}

KRVector3 KRVector3::OneVector() {
    return KRVector3(1.0f, 1.0f, 1.0f);
}

KRVector3 KRVector3::ForwardVector() {
    return KRVector3(0.0f, 0.0f, 1.0f);
}

KRVector3 KRVector3::BackwardVector() {
    return KRVector3(0.0f, 0.0f, -1.0f);
}

KRVector3 KRVector3::UpVector() {
    return KRVector3(0.0f, 1.0f, 0.0f);
}

KRVector3 KRVector3::DownVector() {
    return KRVector3(0.0f, -1.0f, 0.0f);
}

KRVector3 KRVector3::LeftVector() {
    return KRVector3(-1.0f, 0.0f, 0.0f);
}

KRVector3 KRVector3::RightVector() {
    return KRVector3(1.0f, 0.0f, 0.0f);
}

KRVector3 KRVector3::Lerp(const KRVector3 &v1, const KRVector3 &v2, float d) {
    return v1 + (v2 - v1) * d;
}

KRVector3 KRVector3::Slerp(const KRVector3 &v1, const KRVector3 &v2, float d) {
    // From: http://keithmaggio.wordpress.com/2011/02/15/math-magician-lerp-slerp-and-nlerp/
    // Dot product - the cosine of the angle between 2 vectors.
    float dot = KRVector3::Dot(v1, v2);     
    // Clamp it to be in the range of Acos()
    if(dot < -1.0f) dot = -1.0f;
    if(dot > 1.0f) dot = 1.0f;
    // Acos(dot) returns the angle between start and end,
    // And multiplying that by percent returns the angle between
    // start and the final result.
    float theta = acos(dot)*d;
    KRVector3 RelativeVec = v2 - v1*dot;
    RelativeVec.normalize();     // Orthonormal basis
    // The final result.
    return ((v1*cos(theta)) + (RelativeVec*sin(theta)));
}

void KRVector3::OrthoNormalize(KRVector3 &normal, KRVector3 &tangent) {
    // Gram-Schmidt Orthonormalization
    normal.normalize();
    KRVector3 proj = normal * Dot(tangent, normal);
    tangent = tangent - proj;
    tangent.normalize();
}

KRVector3::KRVector3(float v) {
    x = v;
    y = v;
    z = v;
}

KRVector3::KRVector3(float X, float Y, float Z)
{
    x = X;
    y = Y;
    z = Z;
}

KRVector3::~KRVector3()
{
}

KRVector3& KRVector3::operator =(const KRVector3& b) {
    x = b.x;
    y = b.y;
    z = b.z;
    return *this;
}
KRVector3 KRVector3::operator +(const KRVector3& b) const {
    return KRVector3(x + b.x, y + b.y, z + b.z);
}
KRVector3 KRVector3::operator -(const KRVector3& b) const {
    return KRVector3(x - b.x, y - b.y, z - b.z);
}
KRVector3 KRVector3::operator +() const {
    return *this;
}
KRVector3 KRVector3::operator -() const {
    return KRVector3(-x, -y, -z);
}

KRVector3 KRVector3::operator *(const float v) const {
    return KRVector3(x * v, y * v, z * v);
}

KRVector3 KRVector3::operator /(const float v) const {
    return KRVector3(x / v, y / v, z / v);
}

KRVector3& KRVector3::operator +=(const KRVector3& b) {
    x += b.x;
    y += b.y;
    z += b.z;
    
    return *this;
}

KRVector3& KRVector3::operator -=(const KRVector3& b) {
    x -= b.x;
    y -= b.y;
    z -= b.z;
    
    return *this;
}

KRVector3& KRVector3::operator *=(const float v) {
    x *= v;
    y *= v;
    z *= v;
    
    return *this;
}

KRVector3& KRVector3::operator /=(const float v) {
    x /= v;
    y /= v;
    z /= v;
    
    return *this;
}

bool KRVector3::operator ==(const KRVector3& b) const {
    return x == b.x && y == b.y && z == b.z;
    
}
bool KRVector3::operator !=(const KRVector3& b) const {
    return x != b.x || y != b.y || z != b.z;
}

float& KRVector3::operator[](unsigned i) {
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

float KRVector3::operator[](unsigned i) const {
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

float KRVector3::sqrMagnitude() const {
    // calculate the square of the magnitude (useful for comparison of magnitudes without the cost of a sqrt() function)
    return x * x + y * y + z * z;
}

float KRVector3::magnitude() const {
    return sqrtf(x * x + y * y + z * z);
}

void KRVector3::normalize() {
    float magnitude = sqrtf(x * x + y * y + z * z);
    x /= magnitude;
    y /= magnitude;
    z /= magnitude;
}
KRVector3 KRVector3::Normalize(const KRVector3 &v) {
    float magnitude = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    return KRVector3(v.x / magnitude, v.y / magnitude, v.z / magnitude);
}

KRVector3 KRVector3::Cross(const KRVector3 &v1, const KRVector3 &v2) {
    return KRVector3(v1.y * v2.z - v1.z * v2.y,
                     v1.z * v2.x - v1.x * v2.z,
                     v1.x * v2.y - v1.y * v2.x);
}

float KRVector3::Dot(const KRVector3 &v1, const KRVector3 &v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

