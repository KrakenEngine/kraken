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

#import "KREngine-common.h"

class KRVector3 {

public:
    float x, y, z;
    
    KRVector3();
	KRVector3(float X, float Y, float Z);
    KRVector3(float v);
    KRVector3(const KRVector3 &v);
	~KRVector3();
    
    
    KRVector3& operator =(const KRVector3& b);
    KRVector3 operator +(const KRVector3& b) const;
    KRVector3 operator -(const KRVector3& b) const;
    KRVector3 operator +() const;
    KRVector3 operator -() const;
    KRVector3 operator *(const float v) const;
    KRVector3 operator /(const float v) const;
    
    KRVector3& operator +=(const KRVector3& b);
    KRVector3& operator -=(const KRVector3& b);
    KRVector3& operator *=(const float v);
    KRVector3& operator /=(const float v);
    
    bool operator ==(const KRVector3& b) const;
    bool operator !=(const KRVector3& b) const;
    
    float& operator[](unsigned i);
    float operator[](unsigned i) const;
    
    float sqrMagnitude() const; // calculate the square of the magnitude (useful for comparison of magnitudes without the cost of a sqrt() function)
    float magnitude() const;
    
    void normalize();
    static KRVector3 Normalize(const KRVector3 &v);
    
    static KRVector3 Cross(const KRVector3 &v1, const KRVector3 &v2);
    
    static float Dot(const KRVector3 &v1, const KRVector3 &v2);
    
    static KRVector3 ZeroVector();
    static KRVector3 OneVector();
    static KRVector3 ForwardVector();
    static KRVector3 BackwardVector();
    static KRVector3 UpVector();
    static KRVector3 DownVector();
    static KRVector3 LeftVector();
    static KRVector3 RightVector();
    static KRVector3 Lerp(const KRVector3 &v1, const KRVector3 &v2, float d);
    static KRVector3 Slerp(const KRVector3 &v1, const KRVector3 &v2, float d);
    static void OrthoNormalize(KRVector3 &normal, KRVector3 &tangent); // Gram-Schmidt Orthonormalization
};

#endif