//
//  KRVector3.h
//  Kraken
//
//  Copyright 2017 Kearwood Gilbert. All rights reserved.
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

#ifndef KRVECTOR3_H
#define KRVECTOR3_H

#include <functional> // for hash<>

#include "KRVector4.h"

namespace kraken {

class KRVector3 {

public:
    union {        
        struct {
            float x, y, z;
        };
        float c[3];
    };
    
    KRVector3();
    KRVector3(float X, float Y, float Z);
    KRVector3(float v);
    KRVector3(float *v);
    KRVector3(double *v);
    KRVector3(const KRVector3 &v);
    KRVector3(const KRVector4 &v);
    ~KRVector3();
    
    // KRVector2 swizzle getters
    KRVector2 xx() const;
    KRVector2 xy() const;
    KRVector2 xz() const;
    KRVector2 yx() const;
    KRVector2 yy() const;
    KRVector2 yz() const;
    KRVector2 zx() const;
    KRVector2 zy() const;
    KRVector2 zz() const;
    
    // KRVector2 swizzle setters
    void xy(const KRVector2 &v);
    void xz(const KRVector2 &v);
    void yx(const KRVector2 &v);
    void yz(const KRVector2 &v);
    void zx(const KRVector2 &v);
    void zy(const KRVector2 &v);
    
    KRVector3& operator =(const KRVector3& b);
    KRVector3& operator =(const KRVector4& b);
    KRVector3 operator +(const KRVector3& b) const;
    KRVector3 operator -(const KRVector3& b) const;
    KRVector3 operator +() const;
    KRVector3 operator -() const;
    KRVector3 operator *(const float v) const;
    KRVector3 operator /(const float v) const;
    
    KRVector3& operator +=(const KRVector3& b);
    KRVector3& operator -=(const KRVector3& b);
    KRVector3& operator *=(const float v);
    KRVector3& operator /=(const float v);
    
    bool operator ==(const KRVector3& b) const;
    bool operator !=(const KRVector3& b) const;
    
    // Comparison operators are implemented to allow insertion into sorted containers such as std::set
    bool operator >(const KRVector3& b) const;
    bool operator <(const KRVector3& b) const;
    
    float& operator[](unsigned i);
    float operator[](unsigned i) const;
    
    float sqrMagnitude() const; // calculate the square of the magnitude (useful for comparison of magnitudes without the cost of a sqrt() function)
    float magnitude() const;
    
    void scale(const KRVector3 &v);
    void normalize();
    static KRVector3 Normalize(const KRVector3 &v);
    
    static KRVector3 Cross(const KRVector3 &v1, const KRVector3 &v2);
    
    static float Dot(const KRVector3 &v1, const KRVector3 &v2);
    static KRVector3 Min();
    static KRVector3 Max();
    static const KRVector3 &Zero();
    static KRVector3 One();
    static KRVector3 Forward();
    static KRVector3 Backward();
    static KRVector3 Up();
    static KRVector3 Down();
    static KRVector3 Left();
    static KRVector3 Right();
    static KRVector3 Scale(const KRVector3 &v1, const KRVector3 &v2);
    static KRVector3 Lerp(const KRVector3 &v1, const KRVector3 &v2, float d);
    static KRVector3 Slerp(const KRVector3 &v1, const KRVector3 &v2, float d);
    static void OrthoNormalize(KRVector3 &normal, KRVector3 &tangent); // Gram-Schmidt Orthonormalization
};

} // namespace kraken

namespace std {
  template<>
  struct hash<kraken::KRVector3> {
  public:
    size_t operator()(const kraken::KRVector3 &s) const
    {
      size_t h1 = hash<float>()(s.x);
      size_t h2 = hash<float>()(s.y);
      size_t h3 = hash<float>()(s.z);
      return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
  };
}

#endif // KRVECTOR3_H
