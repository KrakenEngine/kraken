//
//  KRAABB.h
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-08-30.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

// Axis aligned bounding box

#ifndef KRAABB_H
#define KRAABB_H

#include <functional> // for hash<>

#include "Vector2.h"
#include "Vector3.h"

namespace kraken {

class KRMat4;

class KRAABB {
public:
  KRAABB(const Vector3 &minPoint, const Vector3 &maxPoint);
  KRAABB(const Vector3 &corner1, const Vector3 &corner2, const KRMat4 &modelMatrix);
  KRAABB();
  ~KRAABB();
    
  void scale(const Vector3 &s);
  void scale(float s);
    
  Vector3 center() const;
  Vector3 size() const;
  float volume() const;
  bool intersects(const KRAABB& b) const;
  bool contains(const KRAABB &b) const;
  bool contains(const Vector3 &v) const;
    
  bool intersectsLine(const Vector3 &v1, const Vector3 &v2) const;
  bool intersectsRay(const Vector3 &v1, const Vector3 &dir) const;
  bool intersectsSphere(const Vector3 &center, float radius) const;
  void encapsulate(const KRAABB & b);
    
  KRAABB& operator =(const KRAABB& b);
  bool operator ==(const KRAABB& b) const;
  bool operator !=(const KRAABB& b) const;
    
  // Comparison operators are implemented to allow insertion into sorted containers such as std::set
  bool operator >(const KRAABB& b) const;
  bool operator <(const KRAABB& b) const;
    
  Vector3 min;
  Vector3 max;
    
  static KRAABB Infinite();
  static KRAABB Zero();
    
  float longest_radius() const;
  Vector3 nearestPoint(const Vector3 & v) const;
};

} // namespace kraken

namespace std {
  template<>
  struct hash<kraken::KRAABB> {
  public:
    size_t operator()(const kraken::KRAABB &s) const
    {
      size_t h1 = hash<kraken::Vector3>()(s.min);
      size_t h2 = hash<kraken::Vector3>()(s.max);
      return h1 ^ ( h2 << 1 );
    }
  };
} // namespace std


#endif /* defined(KRAABB_H) */
