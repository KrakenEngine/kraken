//
//  KRVector4.h
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

#ifndef KRVECTOR4_H
#define KRVECTOR4_H

#include "KREngine-common.h"

class KRVector3;

class KRVector4 {
    
public:
    union {
        struct {
            float x, y, z, w;
        };
        float c[4];
    };
    
    KRVector4();
	KRVector4(float X, float Y, float Z, float W);
    KRVector4(float v);
    KRVector4(float *v);
    KRVector4(const KRVector4 &v);
    KRVector4(const KRVector3 &v, float W);
	~KRVector4();
    
    
    KRVector4& operator =(const KRVector4& b);
    KRVector4 operator +(const KRVector4& b) const;
    KRVector4 operator -(const KRVector4& b) const;
    KRVector4 operator +() const;
    KRVector4 operator -() const;
    KRVector4 operator *(const float v) const;
    KRVector4 operator /(const float v) const;
    
    KRVector4& operator +=(const KRVector4& b);
    KRVector4& operator -=(const KRVector4& b);
    KRVector4& operator *=(const float v);
    KRVector4& operator /=(const float v);
    
    bool operator ==(const KRVector4& b) const;
    bool operator !=(const KRVector4& b) const;
    
    // Comparison operators are implemented to allow insertion into sorted containers such as std::set
    bool operator >(const KRVector4& b) const;
    bool operator <(const KRVector4& b) const;
    
    float& operator[](unsigned i);
    float operator[](unsigned i) const;
    
    float sqrMagnitude() const; // calculate the square of the magnitude (useful for comparison of magnitudes without the cost of a sqrt() function)
    float magnitude() const;
    
    void normalize();
    static KRVector4 Normalize(const KRVector4 &v);

    
    static float Dot(const KRVector4 &v1, const KRVector4 &v2);
    static KRVector4 Min();
    static KRVector4 Max();
    static KRVector4 Zero();
    static KRVector4 One();
    static KRVector4 Forward();
    static KRVector4 Backward();
    static KRVector4 Up();
    static KRVector4 Down();
    static KRVector4 Left();
    static KRVector4 Right();
    static KRVector4 Lerp(const KRVector4 &v1, const KRVector4 &v2, float d);
    static KRVector4 Slerp(const KRVector4 &v1, const KRVector4 &v2, float d);
    static void OrthoNormalize(KRVector4 &normal, KRVector4 &tangent); // Gram-Schmidt Orthonormalization
    
    void setUniform(GLint location) const;
};

#endif