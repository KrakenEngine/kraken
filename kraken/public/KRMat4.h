//
//  KRMat4.h
//  Kraken
//
//  Copyright 2017 Kearwood Gilbert. All rights reserved.
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


#include "Vector3.h"
#include "Vector4.h"

#ifndef KRMAT4_H
#define KRMAT4_H

namespace kraken {

typedef enum {
    X_AXIS,
    Y_AXIS,
    Z_AXIS
} AXIS;

class KRQuaternion;

class KRMat4 {
    public:
    
    float c[16]; // Matrix components, in column-major order
    
    // Default constructor - Creates an identity matrix
    KRMat4();
    
    KRMat4(float *pMat);
    
    KRMat4(const Vector3 &axis_x, const Vector3 &axis_y, const Vector3 &axis_z, const Vector3 &trans);
    
    // Destructor
    ~KRMat4();
    
    // Copy constructor
    KRMat4(const KRMat4 &m);
    
    // Overload assignment operator
    KRMat4& operator=(const KRMat4 &m);
    
    // Overload comparison operator
    bool operator==(const KRMat4 &m) const;
    
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
    void translate(const Vector3 &v);
    void scale(float x, float y, float z);
    void scale(const Vector3 &v);
    void scale(float s);
    void rotate(float angle, AXIS axis);
    void rotate(const KRQuaternion &q);
    void bias();
    bool invert();
    void transpose();
    
    static Vector3 DotNoTranslate(const KRMat4 &m, const Vector3 &v); // Dot product without including translation; useful for transforming normals and tangents
    static KRMat4 Invert(const KRMat4 &m);
    static KRMat4 Transpose(const KRMat4 &m);
    static Vector3 Dot(const KRMat4 &m, const Vector3 &v);
    static Vector4 Dot4(const KRMat4 &m, const Vector4 &v);
    static float DotW(const KRMat4 &m, const Vector3 &v);
    static Vector3 DotWDiv(const KRMat4 &m, const Vector3 &v);
    
    static KRMat4 LookAt(const Vector3 &cameraPos, const Vector3 &lookAtPos, const Vector3 &upDirection);
    
    static KRMat4 Translation(const Vector3 &v);
    static KRMat4 Rotation(const Vector3 &v);
    static KRMat4 Scaling(const Vector3 &v);
};

} // namespace kraken

#endif // KRMAT4_H