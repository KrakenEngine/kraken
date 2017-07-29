//
//  Vector3.h
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

#ifndef KRAKEN_VECTOR3_H
#define KRAKEN_VECTOR3_H

#include <functional> // for hash<>

#include "Vector2.h"
#include "KRVector4.h"

namespace kraken {

class Vector3 {

public:
    union {        
        struct {
            float x, y, z;
        };
        float c[3];
    };
    
    Vector3();
    Vector3(float X, float Y, float Z);
    Vector3(float v);
    Vector3(float *v);
    Vector3(double *v);
    Vector3(const Vector3 &v);
    Vector3(const KRVector4 &v);
    ~Vector3();
    
    // Vector2 swizzle getters
    Vector2 xx() const;
    Vector2 xy() const;
    Vector2 xz() const;
    Vector2 yx() const;
    Vector2 yy() const;
    Vector2 yz() const;
    Vector2 zx() const;
    Vector2 zy() const;
    Vector2 zz() const;
    
    // Vector2 swizzle setters
    void xy(const Vector2 &v);
    void xz(const Vector2 &v);
    void yx(const Vector2 &v);
    void yz(const Vector2 &v);
    void zx(const Vector2 &v);
    void zy(const Vector2 &v);
    
    Vector3& operator =(const Vector3& b);
    Vector3& operator =(const KRVector4& b);
    Vector3 operator +(const Vector3& b) const;
    Vector3 operator -(const Vector3& b) const;
    Vector3 operator +() const;
    Vector3 operator -() const;
    Vector3 operator *(const float v) const;
    Vector3 operator /(const float v) const;
    
    Vector3& operator +=(const Vector3& b);
    Vector3& operator -=(const Vector3& b);
    Vector3& operator *=(const float v);
    Vector3& operator /=(const float v);
    
    bool operator ==(const Vector3& b) const;
    bool operator !=(const Vector3& b) const;
    
    // Comparison operators are implemented to allow insertion into sorted containers such as std::set
    bool operator >(const Vector3& b) const;
    bool operator <(const Vector3& b) const;
    
    float& operator[](unsigned i);
    float operator[](unsigned i) const;
    
    float sqrMagnitude() const; // calculate the square of the magnitude (useful for comparison of magnitudes without the cost of a sqrt() function)
    float magnitude() const;
    
    void scale(const Vector3 &v);
    void normalize();
    static Vector3 Normalize(const Vector3 &v);
    
    static Vector3 Cross(const Vector3 &v1, const Vector3 &v2);
    
    static float Dot(const Vector3 &v1, const Vector3 &v2);
    static Vector3 Min();
    static Vector3 Max();
    static const Vector3 &Zero();
    static Vector3 One();
    static Vector3 Forward();
    static Vector3 Backward();
    static Vector3 Up();
    static Vector3 Down();
    static Vector3 Left();
    static Vector3 Right();
    static Vector3 Scale(const Vector3 &v1, const Vector3 &v2);
    static Vector3 Lerp(const Vector3 &v1, const Vector3 &v2, float d);
    static Vector3 Slerp(const Vector3 &v1, const Vector3 &v2, float d);
    static void OrthoNormalize(Vector3 &normal, Vector3 &tangent); // Gram-Schmidt Orthonormalization
};

} // namespace kraken

namespace std {
  template<>
  struct hash<kraken::Vector3> {
  public:
    size_t operator()(const kraken::Vector3 &s) const
    {
      size_t h1 = hash<float>()(s.x);
      size_t h2 = hash<float>()(s.y);
      size_t h3 = hash<float>()(s.z);
      return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
  };
}

#endif // KRAKEN_VECTOR3_H
