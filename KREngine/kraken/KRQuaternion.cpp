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

#include "KRQuaternion.h"
#include "KRVector3.h"

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

KRQuaternion::KRQuaternion(const KRVector3 &euler) {
    setEulerZYX(euler);
}

KRQuaternion::KRQuaternion(const KRVector3 &from_vector, const KRVector3 &to_vector) {
    
    KRVector3 a = KRVector3::Cross(from_vector, to_vector);
    m_val[0] = a[0];
    m_val[1] = a[1];
    m_val[2] = a[2];
    m_val[3] = sqrt(from_vector.sqrMagnitude() * to_vector.sqrMagnitude()) + KRVector3::Dot(from_vector, to_vector);
    normalize();
}

KRQuaternion::~KRQuaternion() {
    
}

void KRQuaternion::setEulerZYX(const KRVector3 &euler) {
    // ZYX Order!
    m_val[0] = cos(euler[0] / 2.0) * cos(euler[1] / 2.0) * cos(euler[2] / 2.0) + sin(euler[0] / 2.0) * sin(euler[1] / 2.0) * sin(euler[2] / 2.0);
    m_val[1] = sin(euler[0] / 2.0) * cos(euler[1] / 2.0) * cos(euler[2] / 2.0) - cos(euler[0] / 2.0) * sin(euler[1] / 2.0) * sin(euler[2] / 2.0);
    m_val[2] = cos(euler[0] / 2.0) * sin(euler[1] / 2.0) * cos(euler[2] / 2.0) + sin(euler[0] / 2.0) * cos(euler[1] / 2.0) * sin(euler[2] / 2.0);
    m_val[3] = cos(euler[0] / 2.0) * cos(euler[1] / 2.0) * sin(euler[2] / 2.0) - sin(euler[0] / 2.0) * sin(euler[1] / 2.0) * cos(euler[2] / 2.0);
}

float KRQuaternion::operator [](unsigned i) const {
    return m_val[i];
}

float &KRQuaternion::operator [](unsigned i) {
    return m_val[i];
}

KRVector3 KRQuaternion::eulerXYZ() const {
    double a2 = 2 * (m_val[0] * m_val[2] - m_val[1] * m_val[3]);
    if(a2 <= -0.99999) {
        return KRVector3(
           2.0 * atan2(m_val[1], m_val[0]),
           -PI / 2.0,
           0
        );
    } else if(a2 >= 0.99999) {
        return KRVector3(
           2.0 * atan2(m_val[1], m_val[0]),
           PI / 2.0,
           0
        );
    } else {
        return KRVector3(
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
    return KRQuaternion(m_val[0] / num, m_val[1] / num, m_val[2] / num, m_val[3] / num);
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
    m_val[0] += v[0];
    m_val[1] += v[1];
    m_val[2] += v[2];
    m_val[3] += v[3];
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
    m_val[0] /= v;
    m_val[1] /= v;
    m_val[2] /= v;
    m_val[3] /= v;
    return *this;
}

KRQuaternion KRQuaternion::operator +() const {
    return *this;
}

KRQuaternion KRQuaternion::operator -() const {
    return KRQuaternion(-m_val[0], -m_val[1], -m_val[2], -m_val[3]);
}

KRQuaternion Normalize(const KRQuaternion &v1) {
    float magnitude = sqrtf(v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2] + v1[3] * v1[3]);
    return KRQuaternion(
        v1[0] / magnitude,
        v1[1] / magnitude,
        v1[2] / magnitude,
        v1[3] / magnitude
    );
}

void KRQuaternion::normalize() {
    float magnitude = sqrtf(m_val[0] * m_val[0] + m_val[1] * m_val[1] + m_val[2] * m_val[2] + m_val[3] * m_val[3]);
    m_val[0] /= magnitude;
    m_val[1] /= magnitude;
    m_val[2] /= magnitude;
    m_val[3] /= magnitude;
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
    
    matRotate.getPointer()[0] = 1.0 - 2.0 * (m_val[2] * m_val[2] + m_val[3] * m_val[3]);
    matRotate.getPointer()[1] = 2.0 * (m_val[1] * m_val[2] - m_val[0] * m_val[3]);
    matRotate.getPointer()[2] = 2.0 * (m_val[0] * m_val[2] + m_val[1] * m_val[3]);
    
    matRotate.getPointer()[4] = 2.0 * (m_val[1] * m_val[2] + m_val[0] * m_val[3]);
    matRotate.getPointer()[5] = 1.0 - 2.0 * (m_val[1] * m_val[1] + m_val[3] * m_val[3]);
    matRotate.getPointer()[6] = 2.0 * (m_val[2] * m_val[3] - m_val[0] * m_val[1]);
    
    matRotate.getPointer()[8] = 2.0 * (m_val[1] * m_val[3] - m_val[0] * m_val[2]);
    matRotate.getPointer()[9] = 2.0 * (m_val[0] * m_val[1] + m_val[2] * m_val[3]);
    matRotate.getPointer()[10] = 1.0 - 2.0 * (m_val[1] * m_val[1] + m_val[2] * m_val[2]);
    
    return matRotate;
}

