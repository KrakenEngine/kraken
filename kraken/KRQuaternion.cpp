//
//  KRQuaternion.cpp
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

KRQuaternion::KRQuaternion() {
    m_val[0] = 1.0;
    m_val[1] = 0.0;
    m_val[2] = 0.0;
    m_val[3] = 0.0;
}

KRQuaternion::KRQuaternion(float w, float x, float y, float z) {
    m_val[0] = w;
    m_val[1] = x;
    m_val[2] = y;
    m_val[3] = z;
}

KRQuaternion::KRQuaternion(const KRQuaternion& p) {
    m_val[0] = p[0];
    m_val[1] = p[1];
    m_val[2] = p[2];
    m_val[3] = p[3];
}

KRQuaternion& KRQuaternion::operator =( const KRQuaternion& p ) {
    m_val[0] = p[0];
    m_val[1] = p[1];
    m_val[2] = p[2];
    m_val[3] = p[3];
    return *this;
}

KRQuaternion::KRQuaternion(const Vector3 &euler) {
    setEulerZYX(euler);
}

KRQuaternion::KRQuaternion(const Vector3 &from_vector, const Vector3 &to_vector) {
    
    Vector3 a = Vector3::Cross(from_vector, to_vector);
    m_val[0] = a[0];
    m_val[1] = a[1];
    m_val[2] = a[2];
    m_val[3] = sqrt(from_vector.sqrMagnitude() * to_vector.sqrMagnitude()) + Vector3::Dot(from_vector, to_vector);
    normalize();
}

KRQuaternion::~KRQuaternion() {
    
}

void KRQuaternion::setEulerXYZ(const Vector3 &euler)
{
    *this = KRQuaternion::FromAngleAxis(Vector3(1.0f, 0.0f, 0.0f), euler.x)
        * KRQuaternion::FromAngleAxis(Vector3(0.0f, 1.0f, 0.0f), euler.y)
        * KRQuaternion::FromAngleAxis(Vector3(0.0f, 0.0f, 1.0f), euler.z);
}

void KRQuaternion::setEulerZYX(const Vector3 &euler) {
    // ZYX Order!
    float c1 = cos(euler[0] * 0.5f);
    float c2 = cos(euler[1] * 0.5f);
    float c3 = cos(euler[2] * 0.5f);
    float s1 = sin(euler[0] * 0.5f);
    float s2 = sin(euler[1] * 0.5f);
    float s3 = sin(euler[2] * 0.5f);
    
    m_val[0] = c1 * c2 * c3 + s1 * s2 * s3;
    m_val[1] = s1 * c2 * c3 - c1 * s2 * s3;
    m_val[2] = c1 * s2 * c3 + s1 * c2 * s3;
    m_val[3] = c1 * c2 * s3 - s1 * s2 * c3;
}

float KRQuaternion::operator [](unsigned i) const {
    return m_val[i];
}

float &KRQuaternion::operator [](unsigned i) {
    return m_val[i];
}

Vector3 KRQuaternion::eulerXYZ() const {
    double a2 = 2 * (m_val[0] * m_val[2] - m_val[1] * m_val[3]);
    if(a2 <= -0.99999) {
        return Vector3(
           2.0 * atan2(m_val[1], m_val[0]),
           -PI * 0.5f,
           0
        );
    } else if(a2 >= 0.99999) {
        return Vector3(
           2.0 * atan2(m_val[1], m_val[0]),
           PI * 0.5f,
           0
        );
    } else {
        return Vector3(
             atan2(2 * (m_val[0] * m_val[1] + m_val[2] * m_val[3]), (1 - 2 * (m_val[1] * m_val[1] + m_val[2] * m_val[2]))),
             asin(a2),
             atan2(2 * (m_val[0] * m_val[3] + m_val[1] * m_val[2]), (1 - 2 * (m_val[2] * m_val[2] + m_val[3] * m_val[3])))
         );
    }
    

}

bool operator ==(KRQuaternion &v1, KRQuaternion &v2) {
    return
        v1[0] == v2[0]
        && v1[1] == v2[1]
        && v1[2] == v2[2]
        && v1[3] == v2[3];
}

bool operator !=(KRQuaternion &v1, KRQuaternion &v2) {
    return
        v1[0] != v2[0]
        || v1[1] != v2[1]
        || v1[2] != v2[2]
        || v1[3] != v2[3];
}

KRQuaternion KRQuaternion::operator *(const KRQuaternion &v) {
    float t0 = (m_val[3]-m_val[2])*(v[2]-v[3]);
    float t1 = (m_val[0]+m_val[1])*(v[0]+v[1]);
    float t2 = (m_val[0]-m_val[1])*(v[2]+v[3]);
    float t3 = (m_val[3]+m_val[2])*(v[0]-v[1]);
    float t4 = (m_val[3]-m_val[1])*(v[1]-v[2]);
    float t5 = (m_val[3]+m_val[1])*(v[1]+v[2]);
    float t6 = (m_val[0]+m_val[2])*(v[0]-v[3]);
    float t7 = (m_val[0]-m_val[2])*(v[0]+v[3]);
    float t8 = t5+t6+t7;
    float t9 = (t4+t8)/2;
    
    return KRQuaternion(
        t0+t9-t5,
        t1+t9-t8,
        t2+t9-t7,
        t3+t9-t6
    );
}

KRQuaternion KRQuaternion::operator *(float v) const {
    return KRQuaternion(m_val[0] * v, m_val[1] * v, m_val[2] * v, m_val[3] * v);
}

KRQuaternion KRQuaternion::operator /(float num) const {
    float inv_num = 1.0f / num;
    return KRQuaternion(m_val[0] * inv_num, m_val[1] * inv_num, m_val[2] * inv_num, m_val[3] * inv_num);
}

KRQuaternion KRQuaternion::operator +(const KRQuaternion &v) const {
    return KRQuaternion(m_val[0] + v[0], m_val[1] + v[1], m_val[2] + v[2], m_val[3] + v[3]);
}

KRQuaternion KRQuaternion::operator -(const KRQuaternion &v) const {
    return KRQuaternion(m_val[0] - v[0], m_val[1] - v[1], m_val[2] - v[2], m_val[3] - v[3]);
}

KRQuaternion& KRQuaternion::operator +=(const KRQuaternion& v) {
    m_val[0] += v[0];
    m_val[1] += v[1];
    m_val[2] += v[2];
    m_val[3] += v[3];
    return *this;
}

KRQuaternion& KRQuaternion::operator -=(const KRQuaternion& v) {
    m_val[0] -= v[0];
    m_val[1] -= v[1];
    m_val[2] -= v[2];
    m_val[3] -= v[3];
    return *this;
}

KRQuaternion& KRQuaternion::operator *=(const KRQuaternion& v) {
    float t0 = (m_val[3]-m_val[2])*(v[2]-v[3]);
    float t1 = (m_val[0]+m_val[1])*(v[0]+v[1]);
    float t2 = (m_val[0]-m_val[1])*(v[2]+v[3]);
    float t3 = (m_val[3]+m_val[2])*(v[0]-v[1]);
    float t4 = (m_val[3]-m_val[1])*(v[1]-v[2]);
    float t5 = (m_val[3]+m_val[1])*(v[1]+v[2]);
    float t6 = (m_val[0]+m_val[2])*(v[0]-v[3]);
    float t7 = (m_val[0]-m_val[2])*(v[0]+v[3]);
    float t8 = t5+t6+t7;
    float t9 = (t4+t8)/2;
    
    m_val[0] = t0+t9-t5;
    m_val[1] = t1+t9-t8;
    m_val[2] = t2+t9-t7;
    m_val[3] = t3+t9-t6;
    
    return *this;
}

KRQuaternion& KRQuaternion::operator *=(const float& v) {
    m_val[0] *= v;
    m_val[1] *= v;
    m_val[2] *= v;
    m_val[3] *= v;
    return *this;
}

KRQuaternion& KRQuaternion::operator /=(const float& v) {
    float inv_v = 1.0f / v;
    m_val[0] *= inv_v;
    m_val[1] *= inv_v;
    m_val[2] *= inv_v;
    m_val[3] *= inv_v;
    return *this;
}

KRQuaternion KRQuaternion::operator +() const {
    return *this;
}

KRQuaternion KRQuaternion::operator -() const {
    return KRQuaternion(-m_val[0], -m_val[1], -m_val[2], -m_val[3]);
}

KRQuaternion Normalize(const KRQuaternion &v1) {
    float inv_magnitude = 1.0f / sqrtf(v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2] + v1[3] * v1[3]);
    return KRQuaternion(
        v1[0] * inv_magnitude,
        v1[1] * inv_magnitude,
        v1[2] * inv_magnitude,
        v1[3] * inv_magnitude
    );
}

void KRQuaternion::normalize() {
    float inv_magnitude = 1.0f / sqrtf(m_val[0] * m_val[0] + m_val[1] * m_val[1] + m_val[2] * m_val[2] + m_val[3] * m_val[3]);
    m_val[0] *= inv_magnitude;
    m_val[1] *= inv_magnitude;
    m_val[2] *= inv_magnitude;
    m_val[3] *= inv_magnitude;
}

KRQuaternion Conjugate(const KRQuaternion &v1) {
    return KRQuaternion(v1[0], -v1[1], -v1[2], -v1[3]);
}

void KRQuaternion::conjugate() {
    m_val[1] = -m_val[1];
    m_val[2] = -m_val[2];
    m_val[3] = -m_val[3];
}

KRMat4 KRQuaternion::rotationMatrix() const {
    KRMat4 matRotate;
    
    /*
    Vector3 euler = eulerXYZ();
    
    matRotate.rotate(euler.x, X_AXIS);
    matRotate.rotate(euler.y, Y_AXIS);
    matRotate.rotate(euler.z, Z_AXIS);
     */
    
    // FINDME - Determine why the more optimal routine commented below wasn't working
    
    
    matRotate.c[0] = 1.0 - 2.0 * (m_val[2] * m_val[2] + m_val[3] * m_val[3]);
    matRotate.c[1] = 2.0 * (m_val[1] * m_val[2] - m_val[0] * m_val[3]);
    matRotate.c[2] = 2.0 * (m_val[0] * m_val[2] + m_val[1] * m_val[3]);
    
    matRotate.c[4] = 2.0 * (m_val[1] * m_val[2] + m_val[0] * m_val[3]);
    matRotate.c[5] = 1.0 - 2.0 * (m_val[1] * m_val[1] + m_val[3] * m_val[3]);
    matRotate.c[6] = 2.0 * (m_val[2] * m_val[3] - m_val[0] * m_val[1]);
    
    matRotate.c[8] = 2.0 * (m_val[1] * m_val[3] - m_val[0] * m_val[2]);
    matRotate.c[9] = 2.0 * (m_val[0] * m_val[1] + m_val[2] * m_val[3]);
    matRotate.c[10] = 1.0 - 2.0 * (m_val[1] * m_val[1] + m_val[2] * m_val[2]);
    
    return matRotate;
}


KRQuaternion KRQuaternion::FromAngleAxis(const Vector3 &axis, float angle)
{
    float ha = angle * 0.5f;
    float sha = sin(ha);
    return KRQuaternion(cos(ha), axis.x * sha, axis.y * sha, axis.z * sha);
}

float KRQuaternion::Dot(const KRQuaternion &v1, const KRQuaternion &v2)
{
    return v1.m_val[0] * v2.m_val[0] + v1.m_val[1] * v2.m_val[1] + v1.m_val[2] * v2.m_val[2] + v1.m_val[3] * v2.m_val[3];
}

KRQuaternion KRQuaternion::Lerp(const KRQuaternion &a, const KRQuaternion &b, float t)
{
    if (t <= 0.0f) {
        return a;
    } else if (t >= 1.0f) {
        return b;
    }
    
    return a * (1.0f - t) + b * t;
}

KRQuaternion KRQuaternion::Slerp(const KRQuaternion &a, const KRQuaternion &b, float t)
{
    if (t <= 0.0f) {
        return a;
    }
    
    if (t >= 1.0f) {
        return b;
    }
    
    float coshalftheta = Dot(a, b);
    KRQuaternion c = a;
    
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
