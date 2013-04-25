//
//  KRMat4.h
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


#include "KRVector3.h"
#include "KRVector4.h"

#include "KREngine-common.h"

#ifndef KRMAT4_I
#define KRMAT4_I


#define EMPTY_MATRIX4  { 0.0, 0.0, 0.0, 0.0,\
0.0, 0.0, 0.0, 0.0,\
0.0, 0.0, 0.0, 0.0,\
0.0, 0.0, 0.0, 0.0 }

#define IDENTITY_MATRIX4 { 1.0, 0.0, 0.0, 0.0,\
0.0, 1.0, 0.0, 0.0,\
0.0, 0.0, 1.0, 0.0,\
0.0, 0.0, 0.0, 1.0 }

typedef enum {
    X_AXIS,
    Y_AXIS,
    Z_AXIS
} AXIS;

class KRQuaternion;

class KRMat4 {
    
    
    public:
    
    float c[16];
    

    
    // Default constructor - Creates an identity matrix
    KRMat4();
    
    KRMat4(float *pMat);
    
    KRMat4(const KRVector3 &axis_x, const KRVector3 &axis_y, const KRVector3 &axis_z, const KRVector3 &trans);
    
    // Destructor
    ~KRMat4();
    
    // Copy constructor
    KRMat4(const KRMat4 &m);
    
    // Overload assignment operator
    KRMat4& operator=(const KRMat4 &m);
    
    // Overload comparison operator
    bool operator==(const KRMat4 &m);
    
    // Overload compound multiply operator
    KRMat4& operator*=(const KRMat4 &m);
    
    float& operator[](unsigned i);
    float operator[](unsigned i) const;
    
    // Overload multiply operator
    //KRMat4& operator*(const KRMat4 &m);
    KRMat4 operator*(const KRMat4 &m) const;
    
    float *getPointer();
    
    void perspective(float fov, float aspect, float nearz, float farz);
    void ortho(float left, float right, float top, float bottom, float nearz, float farz);
    void translate(float x, float y, float z);
    void translate(const KRVector3 &v);
    void scale(float x, float y, float z);
    void scale(const KRVector3 &v);
    void scale(float s);
    void rotate(float angle, AXIS axis);
    void rotate(const KRQuaternion &q);
    void bias();
    bool invert();
    void transpose();
    
    static KRVector3 DotNoTranslate(const KRMat4 &m, const KRVector3 &v); // Dot product without including translation; useful for transforming normals and tangents
    static KRMat4 Invert(const KRMat4 &m);
    static KRMat4 Transpose(const KRMat4 &m);
    static KRVector3 Dot(const KRMat4 &m, const KRVector3 &v);
    static KRVector4 Dot4(const KRMat4 &m, const KRVector4 &v);
    static float DotW(const KRMat4 &m, const KRVector3 &v);
    static KRVector3 DotWDiv(const KRMat4 &m, const KRVector3 &v);
    
    static KRMat4 LookAt(const KRVector3 &cameraPos, const KRVector3 &lookAtPos, const KRVector3 &upDirection);
    
    void setUniform(GLint location) const;
};

#endif // KRMAT4_I