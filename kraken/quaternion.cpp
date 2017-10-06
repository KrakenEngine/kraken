//
//  Quaternion.cpp
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

#include "KRHelpers.h"

namespace kraken {

Quaternion::Quaternion() {
    c[0] = 1.0;
    c[1] = 0.0;
    c[2] = 0.0;
    c[3] = 0.0;
}

Quaternion::Quaternion(float w, float x, float y, float z) {
    c[0] = w;
    c[1] = x;
    c[2] = y;
    c[3] = z;
}

Quaternion::Quaternion(const Quaternion& p) {
    c[0] = p[0];
    c[1] = p[1];
    c[2] = p[2];
    c[3] = p[3];
}

Quaternion& Quaternion::operator =( const Quaternion& p ) {
    c[0] = p[0];
    c[1] = p[1];
    c[2] = p[2];
    c[3] = p[3];
    return *this;
}

Quaternion::Quaternion(const Vector3 &euler) {
    setEulerZYX(euler);
}

Quaternion::Quaternion(const Vector3 &from_vector, const Vector3 &to_vector) {
    
    Vector3 a = Vector3::Cross(from_vector, to_vector);
    c[0] = a[0];
    c[1] = a[1];
    c[2] = a[2];
    c[3] = sqrt(from_vector.sqrMagnitude() * to_vector.sqrMagnitude()) + Vector3::Dot(from_vector, to_vector);
    normalize();
}

Quaternion::~Quaternion() {
    
}

void Quaternion::setEulerXYZ(const Vector3 &euler)
{
    *this = Quaternion::FromAngleAxis(Vector3(1.0f, 0.0f, 0.0f), euler.x)
        * Quaternion::FromAngleAxis(Vector3(0.0f, 1.0f, 0.0f), euler.y)
        * Quaternion::FromAngleAxis(Vector3(0.0f, 0.0f, 1.0f), euler.z);
}

void Quaternion::setEulerZYX(const Vector3 &euler) {
    // ZYX Order!
    float c1 = cos(euler[0] * 0.5f);
    float c2 = cos(euler[1] * 0.5f);
    float c3 = cos(euler[2] * 0.5f);
    float s1 = sin(euler[0] * 0.5f);
    float s2 = sin(euler[1] * 0.5f);
    float s3 = sin(euler[2] * 0.5f);
    
    c[0] = c1 * c2 * c3 + s1 * s2 * s3;
    c[1] = s1 * c2 * c3 - c1 * s2 * s3;
    c[2] = c1 * s2 * c3 + s1 * c2 * s3;
    c[3] = c1 * c2 * s3 - s1 * s2 * c3;
}

float Quaternion::operator [](unsigned i) const {
    return c[i];
}

float &Quaternion::operator [](unsigned i) {
    return c[i];
}

Vector3 Quaternion::eulerXYZ() const {
    double a2 = 2 * (c[0] * c[2] - c[1] * c[3]);
    if(a2 <= -0.99999) {
        return Vector3(
           2.0 * atan2(c[1], c[0]),
           -PI * 0.5f,
           0
        );
    } else if(a2 >= 0.99999) {
        return Vector3(
           2.0 * atan2(c[1], c[0]),
           PI * 0.5f,
           0
        );
    } else {
        return Vector3(
             atan2(2 * (c[0] * c[1] + c[2] * c[3]), (1 - 2 * (c[1] * c[1] + c[2] * c[2]))),
             asin(a2),
             atan2(2 * (c[0] * c[3] + c[1] * c[2]), (1 - 2 * (c[2] * c[2] + c[3] * c[3])))
         );
    }
    

}

bool operator ==(Quaternion &v1, Quaternion &v2) {
    return
        v1[0] == v2[0]
        && v1[1] == v2[1]
        && v1[2] == v2[2]
        && v1[3] == v2[3];
}

bool operator !=(Quaternion &v1, Quaternion &v2) {
    return
        v1[0] != v2[0]
        || v1[1] != v2[1]
        || v1[2] != v2[2]
        || v1[3] != v2[3];
}

Quaternion Quaternion::operator *(const Quaternion &v) {
    float t0 = (c[3]-c[2])*(v[2]-v[3]);
    float t1 = (c[0]+c[1])*(v[0]+v[1]);
    float t2 = (c[0]-c[1])*(v[2]+v[3]);
    float t3 = (c[3]+c[2])*(v[0]-v[1]);
    float t4 = (c[3]-c[1])*(v[1]-v[2]);
    float t5 = (c[3]+c[1])*(v[1]+v[2]);
    float t6 = (c[0]+c[2])*(v[0]-v[3]);
    float t7 = (c[0]-c[2])*(v[0]+v[3]);
    float t8 = t5+t6+t7;
    float t9 = (t4+t8)/2;
    
    return Quaternion(
        t0+t9-t5,
        t1+t9-t8,
        t2+t9-t7,
        t3+t9-t6
    );
}

Quaternion Quaternion::operator *(float v) const {
    return Quaternion(c[0] * v, c[1] * v, c[2] * v, c[3] * v);
}

Quaternion Quaternion::operator /(float num) const {
    float inv_num = 1.0f / num;
    return Quaternion(c[0] * inv_num, c[1] * inv_num, c[2] * inv_num, c[3] * inv_num);
}

Quaternion Quaternion::operator +(const Quaternion &v) const {
    return Quaternion(c[0] + v[0], c[1] + v[1], c[2] + v[2], c[3] + v[3]);
}

Quaternion Quaternion::operator -(const Quaternion &v) const {
    return Quaternion(c[0] - v[0], c[1] - v[1], c[2] - v[2], c[3] - v[3]);
}

Quaternion& Quaternion::operator +=(const Quaternion& v) {
    c[0] += v[0];
    c[1] += v[1];
    c[2] += v[2];
    c[3] += v[3];
    return *this;
}

Quaternion& Quaternion::operator -=(const Quaternion& v) {
    c[0] -= v[0];
    c[1] -= v[1];
    c[2] -= v[2];
    c[3] -= v[3];
    return *this;
}

Quaternion& Quaternion::operator *=(const Quaternion& v) {
    float t0 = (c[3]-c[2])*(v[2]-v[3]);
    float t1 = (c[0]+c[1])*(v[0]+v[1]);
    float t2 = (c[0]-c[1])*(v[2]+v[3]);
    float t3 = (c[3]+c[2])*(v[0]-v[1]);
    float t4 = (c[3]-c[1])*(v[1]-v[2]);
    float t5 = (c[3]+c[1])*(v[1]+v[2]);
    float t6 = (c[0]+c[2])*(v[0]-v[3]);
    float t7 = (c[0]-c[2])*(v[0]+v[3]);
    float t8 = t5+t6+t7;
    float t9 = (t4+t8)/2;
    
    c[0] = t0+t9-t5;
    c[1] = t1+t9-t8;
    c[2] = t2+t9-t7;
    c[3] = t3+t9-t6;
    
    return *this;
}

Quaternion& Quaternion::operator *=(const float& v) {
    c[0] *= v;
    c[1] *= v;
    c[2] *= v;
    c[3] *= v;
    return *this;
}

Quaternion& Quaternion::operator /=(const float& v) {
    float inv_v = 1.0f / v;
    c[0] *= inv_v;
    c[1] *= inv_v;
    c[2] *= inv_v;
    c[3] *= inv_v;
    return *this;
}

Quaternion Quaternion::operator +() const {
    return *this;
}

Quaternion Quaternion::operator -() const {
    return Quaternion(-c[0], -c[1], -c[2], -c[3]);
}

Quaternion Normalize(const Quaternion &v1) {
    float inv_magnitude = 1.0f / sqrtf(v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2] + v1[3] * v1[3]);
    return Quaternion(
        v1[0] * inv_magnitude,
        v1[1] * inv_magnitude,
        v1[2] * inv_magnitude,
        v1[3] * inv_magnitude
    );
}

void Quaternion::normalize() {
    float inv_magnitude = 1.0f / sqrtf(c[0] * c[0] + c[1] * c[1] + c[2] * c[2] + c[3] * c[3]);
    c[0] *= inv_magnitude;
    c[1] *= inv_magnitude;
    c[2] *= inv_magnitude;
    c[3] *= inv_magnitude;
}

Quaternion Conjugate(const Quaternion &v1) {
    return Quaternion(v1[0], -v1[1], -v1[2], -v1[3]);
}

void Quaternion::conjugate() {
    c[1] = -c[1];
    c[2] = -c[2];
    c[3] = -c[3];
}

Matrix4 Quaternion::rotationMatrix() const {
    Matrix4 matRotate;
    
    /*
    Vector3 euler = eulerXYZ();
    
    matRotate.rotate(euler.x, X_AXIS);
    matRotate.rotate(euler.y, Y_AXIS);
    matRotate.rotate(euler.z, Z_AXIS);
     */
    
    // FINDME - Determine why the more optimal routine commented below wasn't working
    
    
    matRotate.c[0] = 1.0 - 2.0 * (c[2] * c[2] + c[3] * c[3]);
    matRotate.c[1] = 2.0 * (c[1] * c[2] - c[0] * c[3]);
    matRotate.c[2] = 2.0 * (c[0] * c[2] + c[1] * c[3]);
    
    matRotate.c[4] = 2.0 * (c[1] * c[2] + c[0] * c[3]);
    matRotate.c[5] = 1.0 - 2.0 * (c[1] * c[1] + c[3] * c[3]);
    matRotate.c[6] = 2.0 * (c[2] * c[3] - c[0] * c[1]);
    
    matRotate.c[8] = 2.0 * (c[1] * c[3] - c[0] * c[2]);
    matRotate.c[9] = 2.0 * (c[0] * c[1] + c[2] * c[3]);
    matRotate.c[10] = 1.0 - 2.0 * (c[1] * c[1] + c[2] * c[2]);
    
    return matRotate;
}


Quaternion Quaternion::FromAngleAxis(const Vector3 &axis, float angle)
{
    float ha = angle * 0.5f;
    float sha = sin(ha);
    return Quaternion(cos(ha), axis.x * sha, axis.y * sha, axis.z * sha);
}

float Quaternion::Dot(const Quaternion &v1, const Quaternion &v2)
{
    return v1.c[0] * v2.c[0] + v1.c[1] * v2.c[1] + v1.c[2] * v2.c[2] + v1.c[3] * v2.c[3];
}

Quaternion Quaternion::Lerp(const Quaternion &a, const Quaternion &b, float t)
{
    if (t <= 0.0f) {
        return a;
    } else if (t >= 1.0f) {
        return b;
    }
    
    return a * (1.0f - t) + b * t;
}

Quaternion Quaternion::Slerp(const Quaternion &a, const Quaternion &b, float t)
{
    if (t <= 0.0f) {
        return a;
    }
    
    if (t >= 1.0f) {
        return b;
    }
    
    float coshalftheta = Dot(a, b);
    Quaternion c = a;
    
    // Angle is greater than 180. We can negate the angle/quat to get the
    // shorter rotation to reach the same destination.
    if ( coshalftheta < 0.0f ) {
        coshalftheta = -coshalftheta;
        c = -c;
    }
    
    if ( coshalftheta > (1.0f - std::numeric_limits<float>::epsilon())) {
        // Angle is tiny - save some computation by lerping instead.
        return Lerp(c, b, t);
    }
    
    float halftheta = acos(coshalftheta);
    
    return (c * sin((1.0f - t) * halftheta) + b * sin(t * halftheta)) / sin(halftheta);
}

} // namespace kraken