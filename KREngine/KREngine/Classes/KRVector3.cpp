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

//default constructor
KRVector3::KRVector3()
{
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
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

KRVector3::KRVector3(const KRVector3& p) {
    x = p.x;
    y = p.y;
    z = p.z;
}

KRVector3& KRVector3::operator = ( const KRVector3& p ) {
    x = p.x;
    y = p.y;
    z = p.z;
    
    return *this;
}

KRVector3::~KRVector3()
{
}

//calculate and return the magnitude of this vector
float KRVector3::GetMagnitude()
{
    return sqrtf(GetSqrMagnitude());
}

float KRVector3::GetSqrMagnitude()
{
    return x * x + y * y + z * z;
}

//multiply this vector by a scalar
KRVector3 KRVector3::operator*(float num) const
{
    return KRVector3(x * num, y * num, z * num);
}

//pass in a vector, pass in a scalar, return the product
/*
KRVector3 KRVector3::operator*(float num, KRVector3 const &vec)
{
    return KRVector3(vec.x * num, vec.y * num, vec.z * num);
}
 */

//add two vectors
KRVector3 KRVector3::operator+(const KRVector3 &vec) const
{
    return KRVector3(x + vec.x, y + vec.y, z + vec.z);
}

//subtract two vectors
KRVector3 KRVector3::operator-(const KRVector3 &vec) const
{
    return KRVector3(x - vec.x, y - vec.y, z - vec.z);
}

//normalize this vector
void KRVector3::normalize()
{
    float magnitude = sqrtf(x * x + y * y + z * z);
    x /= magnitude;
    y /= magnitude;
    z /= magnitude;
}

float KRVector3::Dot(const KRVector3 &v1, const KRVector3 &v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

//calculate and return dot product
float KRVector3::dot(const KRVector3 &vec) const
{
    return x * vec.x + y * vec.y + z * vec.z;
}

//calculate and return cross product
KRVector3 KRVector3::cross(const KRVector3 &vec) const
{
    return KRVector3(y * vec.z - z * vec.y,
                   z * vec.x - x * vec.z,
                   x * vec.y - y * vec.x);
}

bool operator== (KRVector3 &v1, KRVector3 &v2) {
    return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
}

bool operator!= (KRVector3 &v1, KRVector3 &v2) {
    return !(v1 == v2);
}