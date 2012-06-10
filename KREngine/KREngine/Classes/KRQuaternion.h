//
//  KRQuaternion.h
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

#ifndef KRQUATERNION
#define KRQUATERNION
#include <math.h>

#import "KREngine-common.h"

class KRVector3;

class KRQuaternion {
public:
    KRQuaternion();
    KRQuaternion(float w, float x, float y, float z);
    KRQuaternion(const KRQuaternion& p);
    KRQuaternion(const KRVector3 &euler);
    ~KRQuaternion();
    
    KRQuaternion& operator =( const KRQuaternion& p );
	KRQuaternion operator +(const KRQuaternion &v) const;
	KRQuaternion operator -(const KRQuaternion &v) const;
    KRQuaternion operator +() const;
    KRQuaternion operator -() const;
    
    KRQuaternion operator *(const KRQuaternion &v);
	KRQuaternion operator *(float num) const;
    KRQuaternion operator /(float num) const;
    
    KRQuaternion& operator +=(const KRQuaternion& v);
    KRQuaternion& operator -=(const KRQuaternion& v);
    KRQuaternion& operator *=(const KRQuaternion& v);
    KRQuaternion& operator *=(const float& v);
    KRQuaternion& operator /=(const float& v);

    friend bool operator ==(KRVector3 &v1, KRVector3 &v2);
    friend bool operator !=(KRVector3 &v1, KRVector3 &v2);
    float& operator [](unsigned i);
    float operator [](unsigned i) const;
    
    
    void setEuler(const KRVector3 &euler);
    KRVector3 euler() const;
    
    void normalize();
    static KRQuaternion Normalize(const KRQuaternion &v1);
    
    void conjugate();
    static KRQuaternion Conjugate(const KRQuaternion &v1);
private:
    float m_val[4];
};

#endif