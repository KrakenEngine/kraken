//
//  KRTriangle.h
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

#ifndef KRAKEN_TRIANGLE3_H
#define KRAKEN_TRIANGLE3_H

#include "vector3.h"

namespace kraken {

class Triangle3
{
public:
  Vector3 vert[3];

  Triangle3(const Triangle3 &tri);
  Triangle3(const Vector3 &v1, const Vector3 &v2, const Vector3 &v3);
  ~Triangle3();
    
  Vector3 calculateNormal() const;
    
  bool operator ==(const Triangle3& b) const;
  bool operator !=(const Triangle3& b) const;
  Triangle3& operator =(const Triangle3& b);
  Vector3& operator[](unsigned int i);
  Vector3 operator[](unsigned int i) const;
    
  bool rayCast(const Vector3 &start, const Vector3 &dir, Vector3 &hit_point) const;
  bool sphereCast(const Vector3 &start, const Vector3 &dir, float radius, Vector3 &hit_point, float &hit_distance) const;
    
  bool containsPoint(const Vector3 &p) const;
  Vector3 closestPointOnTriangle(const Vector3 &p) const;
};

} // namespace kraken

namespace std {
  template<>
  struct hash<kraken::Triangle3> {
  public:
    size_t operator()(const kraken::Triangle3 &s) const
    {
      size_t h1 = hash<kraken::Vector3>()(s.vert[0]);
      size_t h2 = hash<kraken::Vector3>()(s.vert[1]);
      size_t h3 = hash<kraken::Vector3>()(s.vert[2]);
      return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
  };
} // namespace std

#endif // KRAKEN_TRIANGLE3_H
