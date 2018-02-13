//
//  KRAABB.h
//  Kraken
//
//  Copyright 2018 Kearwood Gilbert. All rights reserved.
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

// Axis aligned bounding box (AABB)

#ifndef KRAKEN_AABB_H
#define KRAKEN_AABB_H

#include <functional> // for hash<>

#include "vector2.h"
#include "vector3.h"

namespace kraken {

class Matrix4;

class AABB {
public:
  Vector3 min;
  Vector3 max;

  AABB(const Vector3 &minPoint, const Vector3 &maxPoint);
  AABB(const Vector3 &corner1, const Vector3 &corner2, const Matrix4 &modelMatrix);
  AABB();
  ~AABB();
    
  void scale(const Vector3 &s);
  void scale(float s);
    
  Vector3 center() const;
  Vector3 size() const;
  float volume() const;
  bool intersects(const AABB& b) const;
  bool contains(const AABB &b) const;
  bool contains(const Vector3 &v) const;
    
  bool intersectsLine(const Vector3 &v1, const Vector3 &v2) const;
  bool intersectsRay(const Vector3 &v1, const Vector3 &dir) const;
  bool intersectsSphere(const Vector3 &center, float radius) const;
  void encapsulate(const AABB & b);
    
  AABB& operator =(const AABB& b);
  bool operator ==(const AABB& b) const;
  bool operator !=(const AABB& b) const;
    
  // Comparison operators are implemented to allow insertion into sorted containers such as std::set
  bool operator >(const AABB& b) const;
  bool operator <(const AABB& b) const;
    
  static AABB Infinite();
  static AABB Zero();
    
  float longest_radius() const;
  Vector3 nearestPoint(const Vector3 & v) const;
};

} // namespace kraken

namespace std {
  template<>
  struct hash<kraken::AABB> {
  public:
    size_t operator()(const kraken::AABB &s) const
    {
      size_t h1 = hash<kraken::Vector3>()(s.min);
      size_t h2 = hash<kraken::Vector3>()(s.max);
      return h1 ^ ( h2 << 1 );
    }
  };
} // namespace std


#endif /* defined(KRAKEN_AABB_H) */
