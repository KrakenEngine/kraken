//
//  KRAABB.h
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-08-30.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
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
