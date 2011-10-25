//
//  KRVector3.h
//  gldemo
//
//  Created by Kearwood Gilbert on 10-12-31.
//  Copyright 2010 Kearwood Software. All rights reserved.
//

#ifndef KRVECTOR3
#define KRVECTOR3
#include <math.h>

class Vector3
{

public:
    float x, y, z;
    
	//default constructor
    
	Vector3(float X, float Y, float Z);
    Vector3();
	~Vector3();
    
    Vector3(const Vector3& p);
    Vector3& operator = ( const Vector3& p );

    
	//calculate and return the magnitude of this vector
	float GetMagnitude();
    
	//multiply this vector by a scalar
	Vector3 operator*(float num) const;
    
	//pass in a vector, pass in a scalar, return the product
	//friend Vector3 operator*(float num, Vector3 const &vec);
    
	//add two vectors
	Vector3 operator+(const Vector3 &vec) const;
    
	//subtract two vectors
	Vector3 operator-(const Vector3 &vec) const;
    
	//normalize this vector
	void normalize();
	
	//calculate and return dot product
	float dot(const Vector3 &vec) const;
    
	//calculate and return cross product
	Vector3 cross(const Vector3 &vec) const;
};

#endif