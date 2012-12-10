//
//  KRVector2.h
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

#ifndef KRVECTOR2
#define KRVECTOR2

#import "KREngine-common.h"

class KRVector2 {
    
public:
    float x, y;
    
    KRVector2();
    KRVector2(float X, float Y);
    KRVector2(float v);
    KRVector2(float *v);
    KRVector2(const KRVector2 &v);
    ~KRVector2();
    
    KRVector2& operator =(const KRVector2& b);
    KRVector2 operator +(const KRVector2& b) const;
    KRVector2 operator -(const KRVector2& b) const;
    KRVector2 operator +() const;
    KRVector2 operator -() const;
    KRVector2 operator *(const float v) const;
    KRVector2 operator /(const float v) const;
    
    KRVector2& operator +=(const KRVector2& b);
    KRVector2& operator -=(const KRVector2& b);
    KRVector2& operator *=(const float v);
    KRVector2& operator /=(const float v);
    
    bool operator ==(const KRVector2& b) const;
    bool operator !=(const KRVector2& b) const;
    
    float& operator[](unsigned i);
    float operator[](unsigned i) const;
    
    float sqrMagnitude() const;
    float magnitude() const;

    void normalize();
    static KRVector2 Normalize(const KRVector2 &v);

    static float Cross(const KRVector2 &v1, const KRVector2 &v2);
    
    static float Dot(const KRVector2 &v1, const KRVector2 &v2);
    static KRVector2 Min();
    static KRVector2 Max();
    static KRVector2 Zero();
    static KRVector2 One();
    
    void setUniform(GLint location) const;
    
private:
    
    
};


#endif
