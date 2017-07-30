//
//  Quaternion.h
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

#ifndef KRAKEN_QUATERNION_H
#define KRAKEN_QUATERNION_H

#include "vector3.h"

namespace kraken {

class Quaternion {
public:
  union {
    struct {
      float w, x, y, z;
    };
    float c[4];
  };

  Quaternion();
  Quaternion(float w, float x, float y, float z);
  Quaternion(const Quaternion& p);
  Quaternion(const Vector3 &euler);
  Quaternion(const Vector3 &from_vector, const Vector3 &to_vector);
  ~Quaternion();
    
  Quaternion& operator =( const Quaternion& p );
  Quaternion operator +(const Quaternion &v) const;
  Quaternion operator -(const Quaternion &v) const;
  Quaternion operator +() const;
  Quaternion operator -() const;
    
  Quaternion operator *(const Quaternion &v);
  Quaternion operator *(float num) const;
  Quaternion operator /(float num) const;
    
  Quaternion& operator +=(const Quaternion& v);
  Quaternion& operator -=(const Quaternion& v);
  Quaternion& operator *=(const Quaternion& v);
  Quaternion& operator *=(const float& v);
  Quaternion& operator /=(const float& v);

  friend bool operator ==(Quaternion &v1, Quaternion &v2);
  friend bool operator !=(Quaternion &v1, Quaternion &v2);
  float& operator [](unsigned i);
  float operator [](unsigned i) const;
    
  void setEulerXYZ(const Vector3 &euler);
  void setEulerZYX(const Vector3 &euler);
  Vector3 eulerXYZ() const;
  Matrix4 rotationMatrix() const;
    
  void normalize();
  static Quaternion Normalize(const Quaternion &v1);
    
  void conjugate();
  static Quaternion Conjugate(const Quaternion &v1);
    
  static Quaternion FromAngleAxis(const Vector3 &axis, float angle);
  static Quaternion Lerp(const Quaternion &a, const Quaternion &b, float t);
  static Quaternion Slerp(const Quaternion &a, const Quaternion &b, float t);
  static float Dot(const Quaternion &v1, const Quaternion &v2);
};

} // namespace kraken

namespace std {
  template<>
  struct hash<kraken::Quaternion> {
  public:
    size_t operator()(const kraken::Quaternion &s) const
    {
      size_t h1 = hash<float>()(s.c[0]);
      size_t h2 = hash<float>()(s.c[1]);
      size_t h3 = hash<float>()(s.c[2]);
      size_t h4 = hash<float>()(s.c[3]);
      return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
    }
  };
} // namespace std

#endif // KRAKEN_QUATERNION_H
