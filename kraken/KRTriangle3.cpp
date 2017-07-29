//
//  KRTriangle.cpp
//  Kraken
//
//  Created by Kearwood Gilbert on 2/8/2014.
//  Copyright (c) 2014 Kearwood Software. All rights reserved.
//

#include "public/kraken.h"

using namespace kraken;

namespace {
  bool _intersectSphere(const Vector3 &start, const Vector3 &dir, const Vector3 &sphere_center, float sphere_radius, float &distance)
  {
    // dir must be normalized

    // From: http://archive.gamedev.net/archive/reference/articles/article1026.html

    // TODO - Move to another class?
    Vector3 Q = sphere_center - start;
    float c = Q.magnitude();
    float v = Vector3::Dot(Q, dir);
    float d = sphere_radius * sphere_radius - (c * c - v * v);



    if (d < 0.0) {
      // No intersection
      return false;
    }

    // Return the distance to the [first] intersecting point

    distance = v - sqrt(d);
    if (distance < 0.0f) {
      return false;
    }
    return true;

  }

  bool _sameSide(const Vector3 &p1, const Vector3 &p2, const Vector3 &a, const Vector3 &b)
  {
    // TODO - Move to Vector3 class?
    // From: http://stackoverflow.com/questions/995445/determine-if-a-3d-point-is-within-a-triangle

    Vector3 cp1 = Vector3::Cross(b - a, p1 - a);
    Vector3 cp2 = Vector3::Cross(b - a, p2 - a);
    if (Vector3::Dot(cp1, cp2) >= 0) return true;
    return false;
  }

  Vector3 _closestPointOnLine(const Vector3 &a, const Vector3 &b, const Vector3 &p)
  {
    // From: http://stackoverflow.com/questions/995445/determine-if-a-3d-point-is-within-a-triangle

    // Determine t (the length of the vector from ‘a’ to ‘p’)

    Vector3 c = p - a;
    Vector3 V = Vector3::Normalize(b - a);
    float d = (a - b).magnitude();
    float t = Vector3::Dot(V, c);

    // Check to see if ‘t’ is beyond the extents of the line segment

    if (t < 0) return a;
    if (t > d) return b;

    // Return the point between ‘a’ and ‘b’

    return a + V * t;
  }
} // anonymous namespace

namespace kraken {

KRTriangle3::KRTriangle3(const Vector3 &v1, const Vector3 &v2, const Vector3 &v3)
{
    vert[0] = v1;
    vert[1] = v2;
    vert[2] = v3;
}

KRTriangle3::KRTriangle3(const KRTriangle3 &tri)
{
    vert[0] = tri[0];
    vert[1] = tri[1];
    vert[3] = tri[3];
}


KRTriangle3::~KRTriangle3()
{
    
}

bool KRTriangle3::operator ==(const KRTriangle3& b) const
{
    return vert[0] == b[0] && vert[1] == b[1] && vert[2] == b[2];
}

bool KRTriangle3::operator !=(const KRTriangle3& b) const
{
    return vert[0] != b[0] || vert[1] != b[1] || vert[2] != b[2];
}

KRTriangle3& KRTriangle3::operator =(const KRTriangle3& b)
{
    
    vert[0] = b[0];
    vert[1] = b[1];
    vert[3] = b[3];
    return *this;
}

Vector3& KRTriangle3::operator[](unsigned int i)
{
    return vert[i];
}

Vector3 KRTriangle3::operator[](unsigned int i) const
{
    return vert[i];
}


bool KRTriangle3::rayCast(const Vector3 &start, const Vector3 &dir, Vector3 &hit_point) const
{
    // algorithm based on Dan Sunday's implementation at http://geomalgorithms.com/a06-_intersect-2.html
    const float SMALL_NUM = 0.00000001;     // anything that avoids division overflow
    Vector3   u, v, n;                    // triangle vectors
    Vector3   w0, w;                      // ray vectors
    float       r, a, b;                    // params to calc ray-plane intersect
    
    // get triangle edge vectors and plane normal
    u = vert[1] - vert[0];
    v = vert[2] - vert[0];
    n = Vector3::Cross(u, v); // cross product
    if (n == Vector3::Zero())             // triangle is degenerate
        return false;                  // do not deal with this case
    
    w0 = start - vert[0];
    a = -Vector3::Dot(n, w0);
    b = Vector3::Dot(n,dir);
    if (fabs(b) < SMALL_NUM) {     // ray is  parallel to triangle plane
        if (a == 0)
            return false; // ray lies in triangle plane
        else {
            return false; // ray disjoint from plane
        }
    }
    
    // get intersect point of ray with triangle plane
    r = a / b;
    if (r < 0.0)                    // ray goes away from triangle
        return false;                   // => no intersect
    // for a segment, also test if (r > 1.0) => no intersect
    
    
    Vector3 plane_hit_point = start + dir * r;            // intersect point of ray and plane
    
    // is plane_hit_point inside triangle?
    float    uu, uv, vv, wu, wv, D;
    uu = Vector3::Dot(u,u);
    uv = Vector3::Dot(u,v);
    vv = Vector3::Dot(v,v);
    w = plane_hit_point - vert[0];
    wu = Vector3::Dot(w,u);
    wv = Vector3::Dot(w,v);
    D = uv * uv - uu * vv;
    
    // get and test parametric coords
    float s, t;
    s = (uv * wv - vv * wu) / D;
    if (s < 0.0 || s > 1.0)         // plane_hit_point is outside triangle
        return false;
    t = (uv * wu - uu * wv) / D;
    if (t < 0.0 || (s + t) > 1.0)  // plane_hit_point is outside triangle
        return false;

    // plane_hit_point is inside the triangle
    hit_point = plane_hit_point;
    return true;
}

Vector3 KRTriangle3::calculateNormal() const
{
    Vector3 v1 = vert[1] - vert[0];
    Vector3 v2 = vert[2] - vert[0];
    
    return Vector3::Normalize(Vector3::Cross(v1, v2));
}

Vector3 KRTriangle3::closestPointOnTriangle(const Vector3 &p) const
{
    Vector3 a = vert[0];
    Vector3 b = vert[1];
    Vector3 c = vert[2];
    
    Vector3 Rab = _closestPointOnLine(a, b, p);
    Vector3 Rbc = _closestPointOnLine(b, c, p);
    Vector3 Rca = _closestPointOnLine(c, a, p);
    
    // return closest [Rab, Rbc, Rca] to p;
    float sd_Rab = (p - Rab).sqrMagnitude();
    float sd_Rbc = (p - Rbc).sqrMagnitude();
    float sd_Rca = (p - Rca).sqrMagnitude();
    
    if(sd_Rab < sd_Rbc && sd_Rab < sd_Rca) {
        return Rab;
    } else if(sd_Rbc < sd_Rab && sd_Rbc < sd_Rca) {
        return Rbc;
    } else {
        return Rca;
    }
}

bool KRTriangle3::sphereCast(const Vector3 &start, const Vector3 &dir, float radius, Vector3 &hit_point, float &hit_distance) const
{
    // Dir must be normalized
    const float SMALL_NUM = 0.001f;     // anything that avoids division overflow
    
    Vector3 tri_normal = calculateNormal();
    
    float d = Vector3::Dot(tri_normal, vert[0]);
    float e = Vector3::Dot(tri_normal, start) - radius;
    float cotangent_distance = e - d;
    
    Vector3 plane_intersect;
    float plane_intersect_distance;
    
    float denom = Vector3::Dot(tri_normal, dir);
    
    if(denom > -SMALL_NUM) {
        return false; // dir is co-planar with the triangle or going in the direction of the normal; no intersection
    }

    // Detect an embedded plane, caused by a sphere that is already intersecting the plane.
    if(cotangent_distance <= 0 && cotangent_distance >= -radius * 2.0f) {
        // Embedded plane - Sphere is already intersecting the plane.
        // Use the point closest to the origin of the sphere as the intersection
        plane_intersect = start - tri_normal * (cotangent_distance + radius);
        plane_intersect_distance = 0.0f;
    } else {
        // Sphere is not intersecting the plane
        // Determine the first point hit by the swept sphere on the triangle's plane
        
        plane_intersect_distance = -(cotangent_distance / denom);
        plane_intersect = start + dir * plane_intersect_distance - tri_normal * radius;
    }
    
    if(plane_intersect_distance < 0.0f) {
        return false;
    }
    
    if(containsPoint(plane_intersect)) {
        // Triangle contains point
        hit_point = plane_intersect;
        hit_distance = plane_intersect_distance;
        return true;
    } else {
        // Triangle does not contain point, cast ray back to sphere from closest point on triangle edge or vertice
        Vector3 closest_point = closestPointOnTriangle(plane_intersect);
        float reverse_hit_distance;
        if(_intersectSphere(closest_point, -dir, start, radius, reverse_hit_distance)) {
            // Reverse cast hit sphere
            hit_distance = reverse_hit_distance;
            hit_point = closest_point;
            return true;
        } else {
            // Reverse cast did not hit sphere
            return false;
        }
    }
}


bool KRTriangle3::containsPoint(const Vector3 &p) const
{
    /*
    // From: http://stackoverflow.com/questions/995445/determine-if-a-3d-point-is-within-a-triangle
    
    const float SMALL_NUM = 0.00000001f;     // anything that avoids division overflow
    // Vector3 A = vert[0], B = vert[1], C = vert[2];
    if (_sameSide(p, vert[0], vert[1], vert[2]) && _sameSide(p, vert[1], vert[0], vert[2]) && _sameSide(p, vert[2], vert[0], vert[1])) {
        Vector3 vc1 = Vector3::Cross(vert[0] - vert[1], vert[0] - vert[2]);
        if(fabs(Vector3::Dot(vert[0] - p, vc1)) <= SMALL_NUM) {
            return true;
        }
    }
    
    return false;
    */
    
    // From: http://blogs.msdn.com/b/rezanour/archive/2011/08/07/barycentric-coordinates-and-point-in-triangle-tests.aspx
    
    Vector3 A = vert[0];
    Vector3 B = vert[1];
    Vector3 C = vert[2];
    Vector3 P = p;
    
    // Prepare our barycentric variables
    Vector3 u = B - A;
    Vector3 v = C - A;
    Vector3 w = P - A;
    
    Vector3 vCrossW = Vector3::Cross(v, w);
    Vector3 vCrossU = Vector3::Cross(v, u);
    
    // Test sign of r
    if (Vector3::Dot(vCrossW, vCrossU) < 0)
        return false;
    
    Vector3 uCrossW = Vector3::Cross(u, w);
    Vector3 uCrossV = Vector3::Cross(u, v);
    
    // Test sign of t
    if (Vector3::Dot(uCrossW, uCrossV) < 0)
        return false;
    
    // At this point, we know that r and t and both > 0.
    // Therefore, as long as their sum is <= 1, each must be less <= 1
    float denom = uCrossV.magnitude();
    float r = vCrossW.magnitude() / denom;
    float t = uCrossW.magnitude() / denom;
    
    return (r + t <= 1);
}

} // namespace kraken
