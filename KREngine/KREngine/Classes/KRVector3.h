//
//  KRVector3.h
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

#ifndef KRVECTOR3
#define KRVECTOR3
#include <math.h>

class KRVector3
{

public:
    float x, y, z;
    
	//default constructor
    
	KRVector3(float X = 0, float Y = 0, float Z = 0);
    KRVector3();
	~KRVector3();
    
    KRVector3(const KRVector3& p);
    KRVector3& operator = ( const KRVector3& p );

    
	//calculate and return the magnitude of this vector
	float GetMagnitude();
    
	//multiply this vector by a scalar
	KRVector3 operator*(float num) const;
    
	//pass in a vector, pass in a scalar, return the product
	//friend KRVector3 operator*(float num, KRVector3 const &vec);
    
	//add two vectors
	KRVector3 operator+(const KRVector3 &vec) const;
    
	//subtract two vectors
	KRVector3 operator-(const KRVector3 &vec) const;
    
	//normalize this vector
	void normalize();
	
	//calculate and return dot product
	float dot(const KRVector3 &vec) const;
    
	//calculate and return cross product
	KRVector3 cross(const KRVector3 &vec) const;
};

#endif