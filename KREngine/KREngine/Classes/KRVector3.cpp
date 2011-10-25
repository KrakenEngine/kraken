//
//  KRVector3.cpp
//  gldemo
//
//  Created by Kearwood Gilbert on 10-12-31.
//  Copyright 2010 Kearwood Software. All rights reserved.
//

#include "KRVector3.h"

//default constructor
Vector3::Vector3()
{
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
}

Vector3::Vector3(float X = 0, float Y = 0, float Z = 0)
{
    x = X;
    y = Y;
    z = Z;
}

Vector3::Vector3(const Vector3& p) {
    x = p.x;
    y = p.y;
    z = p.z;
}

Vector3& Vector3::operator = ( const Vector3& p ) {
    x = p.x;
    y = p.y;
    z = p.z;
    
    return *this;
}

Vector3::~Vector3()
{
}

//calculate and return the magnitude of this vector
float Vector3::GetMagnitude()
{
    return sqrtf(x * x + y * y + z * z);
}

//multiply this vector by a scalar
Vector3 Vector3::operator*(float num) const
{
    return Vector3(x * num, y * num, z * num);
}

//pass in a vector, pass in a scalar, return the product
/*
Vector3 Vector3::operator*(float num, Vector3 const &vec)
{
    return Vector3(vec.x * num, vec.y * num, vec.z * num);
}
 */

//add two vectors
Vector3 Vector3::operator+(const Vector3 &vec) const
{
    return Vector3(x + vec.x, y + vec.y, z + vec.z);
}

//subtract two vectors
Vector3 Vector3::operator-(const Vector3 &vec) const
{
    return Vector3(x - vec.x, y - vec.y, z - vec.z);
}

//normalize this vector
void Vector3::normalize()
{
    float magnitude = sqrtf(x * x + y * y + z * z);
    x /= magnitude;
    y /= magnitude;
    z /= magnitude;
}

//calculate and return dot product
float Vector3::dot(const Vector3 &vec) const
{
    return x * vec.x + y * vec.y + z * vec.z;
}

//calculate and return cross product
Vector3 Vector3::cross(const Vector3 &vec) const
{
    return Vector3(y * vec.z - z * vec.y,
                   z * vec.x - x * vec.z,
                   x * vec.y - y * vec.x);
}
