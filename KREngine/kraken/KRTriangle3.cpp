//
//  KRTriangle.cpp
//  Kraken
//
//  Created by Kearwood Gilbert on 2/8/2014.
//  Copyright (c) 2014 Kearwood Software. All rights reserved.
//

#include "KRTriangle3.h"
#include "KRVector3.h"

KRTriangle3::KRTriangle3(const KRVector3 &v1, const KRVector3 &v2, const KRVector3 &v3)
{
    m_c[0] = v1;
    m_c[1] = v2;
    m_c[2] = v3;
}

KRTriangle3::KRTriangle3(const KRTriangle3 &tri)
{
    m_c[0] = tri[0];
    m_c[1] = tri[1];
    m_c[3] = tri[3];
}


KRTriangle3::~KRTriangle3()
{
    
}

bool KRTriangle3::operator ==(const KRTriangle3& b) const
{
    return m_c[0] == b[0] && m_c[1] == b[1] && m_c[2] == b[2];
}

bool KRTriangle3::operator !=(const KRTriangle3& b) const
{
    return m_c[0] != b[0] || m_c[1] != b[1] || m_c[2] != b[2];
}

KRTriangle3& KRTriangle3::operator =(const KRTriangle3& b)
{
    
    m_c[0] = b[0];
    m_c[1] = b[1];
    m_c[3] = b[3];
    return *this;
}

KRVector3& KRTriangle3::operator[](unsigned i)
{
    return m_c[i];
}

KRVector3 KRTriangle3::operator[](unsigned i) const
{
    return m_c[i];
}


bool KRTriangle3::rayCast(const KRVector3 &start, const KRVector3 &dir, KRVector3 &hit_point) const
{
    // algorithm based on Dan Sunday's implementation at http://geomalgorithms.com/a06-_intersect-2.html
    const float SMALL_NUM = 0.00000001;     // anything that avoids division overflow
    KRVector3   u, v, n;                    // triangle vectors
    KRVector3   w0, w;                      // ray vectors
    float       r, a, b;                    // params to calc ray-plane intersect
    
    // get triangle edge vectors and plane normal
    u = m_c[1] - m_c[0];
    v = m_c[2] - m_c[0];
    n = KRVector3::Cross(u, v); // cross product
    if (n == KRVector3::Zero())             // triangle is degenerate
        return false;                  // do not deal with this case
    
    w0 = start - m_c[0];
    a = -KRVector3::Dot(n, w0);
    b = KRVector3::Dot(n,dir);
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
    
    
    KRVector3 plane_hit_point = start + dir * r;            // intersect point of ray and plane
    
    // is plane_hit_point inside triangle?
    float    uu, uv, vv, wu, wv, D;
    uu = KRVector3::Dot(u,u);
    uv = KRVector3::Dot(u,v);
    vv = KRVector3::Dot(v,v);
    w = plane_hit_point - m_c[0];
    wu = KRVector3::Dot(w,u);
    wv = KRVector3::Dot(w,v);
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

KRVector3 KRTriangle3::calculateNormal() const
{
    KRVector3 v1 = m_c[1] - m_c[0];
    KRVector3 v2 = m_c[2] - m_c[0];
    
    return KRVector3::Normalize(KRVector3::Cross(v1, v2));
}

bool _intersectSphere(const KRVector3 &rO, const KRVector3 &rV, const KRVector3 &sO, float sR, float &distance)
{
    // From: http://archive.gamedev.net/archive/reference/articles/article1026.html
    
    // TODO - Move to another class?
    KRVector3 Q = sO - rO;
    float c = Q.magnitude();
    float v = KRVector3::Dot(Q, rV);
    float d = sR * sR - (c * c - v * v);
    
    // If there was no intersection, return -1
    
    if(d < 0.0) return false;
    
    // Return the distance to the [first] intersecting point
    
    distance = v - sqrt(d);
    return true;
}

bool _sameSide(const KRVector3 &p1, const KRVector3 &p2, const KRVector3 &a, const KRVector3 &b)
{
    // TODO - Move to KRVector3 class?
    // From: http://stackoverflow.com/questions/995445/determine-if-a-3d-point-is-within-a-triangle
    
    KRVector3 cp1 = KRVector3::Cross(b - a, p1 - a);
    KRVector3 cp2 = KRVector3::Cross(b - a, p2 - a);
    if(KRVector3::Dot(cp1, cp2) >= 0) return true;
    return false;
}

KRVector3 _closestPointOnLine(const KRVector3 &a, const KRVector3 &b, const KRVector3 &p)
{
    // From: http://stackoverflow.com/questions/995445/determine-if-a-3d-point-is-within-a-triangle
    
    // Determine t (the length of the vector from ‘a’ to ‘p’)
    
    KRVector3 c = p - a;
    KRVector3 V = KRVector3::Normalize(b - a);
    float d = (a - b).magnitude();
    float t = KRVector3::Dot(V, c);
    
    // Check to see if ‘t’ is beyond the extents of the line segment
    
    if (t < 0) return a;
    if (t > d) return b;
    
    // Return the point between ‘a’ and ‘b’
    
    return a + V * t;
}

KRVector3 KRTriangle3::closestPointOnTriangle(const KRVector3 &p) const
{
    KRVector3 a = m_c[0];
    KRVector3 b = m_c[1];
    KRVector3 c = m_c[2];
    
    KRVector3 Rab = _closestPointOnLine(a, b, p);
    KRVector3 Rbc = _closestPointOnLine(b, c, p);
    KRVector3 Rca = _closestPointOnLine(c, a, p);
    
    // return closest [Rab, Rbc, Rca] to p;
    float sd_Rab = (p - Rab).sqrMagnitude();
    float sd_Rbc = (p - Rbc).sqrMagnitude();
    float sd_Rca = (p - Rca).sqrMagnitude();
    
    if(sd_Rab < sd_Rbc && sd_Rab < sd_Rca) {
        return Rab;
    } else if(sd_Rbc < sd_Rab && sd_Rbc < sd_Rca) {
        return sd_Rbc;
    } else {
        return sd_Rca;
    }
}

bool KRTriangle3::sphereCast(const KRVector3 &start, const KRVector3 &dir, float radius, KRVector3 &hit_point, float &hit_distance) const
{
    // Dir must be normalized
    const float SMALL_NUM = 0.00000001f;     // anything that avoids division overflow
    
    KRVector3 tri_normal = calculateNormal();
    
    float d = KRVector3::Dot(tri_normal, m_c[0]);
    float e = KRVector3::Dot(tri_normal, start) - radius;
    float cotangent_distance = e - d;
    
    KRVector3 plane_intersect;
    float plane_intersect_distance;
    
    // Detect an embedded plane, caused by a sphere that is already intersecting the plane.
    if(cotangent_distance <= 0 && cotangent_distance >= -radius * 2.0f) {
        // Embedded plane - Sphere is already intersecting the plane.
        // Use the point closest to the origin of the sphere as the intersection
        plane_intersect = start - tri_normal * (cotangent_distance + radius);
        plane_intersect_distance = 0.0f;
    } else {
        // Sphere is not intersecting the plane
        // Determine the first point hit by the swept sphere on the triangle's plane

        float denom = KRVector3::Dot(tri_normal, dir);
        
        if(fabs(denom) < SMALL_NUM) {
            return false; // dir is co-planar with the triangle; no intersection
        }
        
        plane_intersect_distance = -(cotangent_distance / denom);
        plane_intersect = start + dir * plane_intersect_distance;
    }
    
    if(containsPoint(plane_intersect)) {
        // Triangle contains point
        hit_point = plane_intersect;
        hit_distance = plane_intersect_distance;
        return true;
    } else {
        // Triangle does not contain point, cast ray back to sphere from closest point on triangle edge or vertice
        KRVector3 closest_point = closestPointOnTriangle(hit_point);
        float reverse_hit_distance;
        if(_intersectSphere(closest_point, -dir, start, radius, reverse_hit_distance)) {
            // Reverse cast hit sphere
            hit_distance = reverse_hit_distance;
            hit_point = closest_point - dir * reverse_hit_distance;
            return true;
        } else {
            // Reverse cast did not hit sphere
            return false;
        }
    }
}


bool KRTriangle3::containsPoint(const KRVector3 &p) const
{
    // From: http://stackoverflow.com/questions/995445/determine-if-a-3d-point-is-within-a-triangle
    
    const float SMALL_NUM = 0.00000001f;     // anything that avoids division overflow
    // KRVector3 A = m_c[0], B = m_c[1], C = m_c[2];
    if (_sameSide(p, m_c[0], m_c[1], m_c[2]) && _sameSide(p, m_c[1], m_c[0], m_c[2]) && _sameSide(p, m_c[2], m_c[0], m_c[1])) {
        KRVector3 vc1 = KRVector3::Cross(m_c[0] - m_c[1], m_c[0] - m_c[2]);
        if(fabs(KRVector3::Dot(m_c[0] - p, vc1)) <= SMALL_NUM) {
            return true;
        }
    }
    
    return false;
}



