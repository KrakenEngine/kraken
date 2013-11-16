//
//  KRMat4.cpp
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

#include "KREngine-common.h"

#include "KRMat4.h"
#include "KRQuaternion.h"

KRMat4::KRMat4() {
    // Default constructor - Initialize with an identity matrix
    static const float IDENTITY_MATRIX[] = {
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    };
    memcpy(c, IDENTITY_MATRIX, sizeof(float) * 16);
    
}

KRMat4::KRMat4(float *pMat) {
    memcpy(c, pMat, sizeof(float) * 16);
}

KRMat4::KRMat4(const KRVector3 &axis_x, const KRVector3 &axis_y, const KRVector3 &axis_z, const KRVector3 &trans)
{
    c[0]  = axis_x.x;  c[1]  = axis_x.y;   c[2]  = axis_x.z;   c[3]  = 0.0f;
    c[4]  = axis_y.x;  c[5]  = axis_y.y;   c[6]  = axis_y.z;   c[7]  = 0.0f;
    c[8]  = axis_z.x;  c[9]  = axis_z.y;   c[10] = axis_z.z;   c[11] = 0.0f;
    c[12] = trans.x;   c[13] = trans.y;    c[14] = trans.z;    c[15] = 1.0f;
}

KRMat4::~KRMat4() {
    
}

float *KRMat4::getPointer() {
    return c;
}

// Copy constructor
KRMat4::KRMat4(const KRMat4 &m) {
    memcpy(c, m.c, sizeof(float) * 16);
}

KRMat4& KRMat4::operator=(const KRMat4 &m) {
    if(this != &m) { // Prevent self-assignment.
        memcpy(c, m.c, sizeof(float) * 16);
    }
    return *this;
}

float& KRMat4::operator[](unsigned i) {
    return c[i];
}

float KRMat4::operator[](unsigned i) const {
    return c[i];
}

// Overload comparison operator
bool KRMat4::operator==(const KRMat4 &m) const {
    return memcmp(c, m.c, sizeof(float) * 16) == 0;
}

// Overload compound multiply operator
KRMat4& KRMat4::operator*=(const KRMat4 &m) {
    float temp[16];
    
    int x,y;
    
    for (x=0; x < 4; x++)
    {
        for(y=0; y < 4; y++)
        {
            temp[y + (x*4)] = (c[x*4] * m.c[y]) +
            (c[(x*4)+1] * m.c[y+4]) +
            (c[(x*4)+2] * m.c[y+8]) +
            (c[(x*4)+3] * m.c[y+12]);
        }
    }
    
    memcpy(c, temp, sizeof(float) << 4);
    return *this;
}

// Overload multiply operator
KRMat4 KRMat4::operator*(const KRMat4 &m) const {
    KRMat4 ret = *this;
    ret *= m;
    return ret;
}


/* Generate a perspective view matrix using a field of view angle fov,
 * window aspect ratio, near and far clipping planes */
void KRMat4::perspective(float fov, float aspect, float nearz, float farz) {
   
    memset(c, 0, sizeof(float) * 16);
    
    float range= tan(fov * 0.5) * nearz;
    c[0] = (2 * nearz) / ((range * aspect) - (-range * aspect));
    c[5] = (2 * nearz) / (2 * range);
    c[10] = -(farz + nearz) / (farz - nearz);
    c[11] = -1;
    c[14] = -(2 * farz * nearz) / (farz - nearz);
    /*
    float range= atan(fov / 20.0f) * nearz; 
    float r = range * aspect;
    float t = range * 1.0;
    
    c[0] = nearz / r;
    c[5] = nearz / t;
    c[10] = -(farz + nearz) / (farz - nearz);
    c[11] = -(2.0 * farz * nearz) / (farz - nearz);
    c[14] = -1.0;
    */
}

/* Perform translation operations on a matrix */
void KRMat4::translate(float x, float y, float z) {
    KRMat4 newMatrix; // Create new identity matrix
    
    newMatrix.c[12] = x;
    newMatrix.c[13] = y;
    newMatrix.c[14] = z;
    
    *this *= newMatrix;
}

void KRMat4::translate(const KRVector3 &v)
{
    translate(v.x, v.y, v.z);
}

/* Rotate a matrix by an angle on a X, Y, or Z axis */
void KRMat4::rotate(float angle, AXIS axis) {
    const int cos1[3] = { 5, 0, 0 }; // cos(angle)
    const int cos2[3] = { 10, 10, 5 }; // cos(angle)
    const int sin1[3] = { 9, 2, 4 }; // -sin(angle)
    const int sin2[3] = { 6, 8, 1 }; // sin(angle)
    
    /*
     X_AXIS:
     
     1,    0,    0,    0
     0,    cos(angle), -sin(angle), 0
     0,    sin(angle), cos(angle), 0
     0,    0,    0,    1
     
     Y_AXIS:
     
     cos(angle), 0, -sin(angle), 0
     0, 1, 0, 0
     sin(angle), 0, cos(angle), 0
     0, 0, 0, 1
     
     Z_AXIS:
     
     cos(angle), -sin(angle), 0, 0
     sin(angle), cos(angle), 0, 0
     0, 0, 1, 0
     0, 0, 0, 1
     
     */
    
    KRMat4 newMatrix; // Create new identity matrix
    
    newMatrix.c[cos1[axis]] = cos(angle);
    newMatrix.c[sin1[axis]] = -sin(angle);
    newMatrix.c[sin2[axis]] = -newMatrix.c[sin1[axis]];
    newMatrix.c[cos2[axis]] = newMatrix.c[cos1[axis]];
    
    *this *= newMatrix;
}

void KRMat4::rotate(const KRQuaternion &q)
{
    *this *= q.rotationMatrix();
}

/* Scale matrix by separate x, y, and z amounts */
void KRMat4::scale(float x, float y, float z) {
    KRMat4 newMatrix; // Create new identity matrix
    
    newMatrix.c[0] = x;
    newMatrix.c[5] = y;
    newMatrix.c[10] = z;
    
    *this *= newMatrix;
}

void KRMat4::scale(const KRVector3 &v) {
    scale(v.x, v.y, v.z);
}

/* Scale all dimensions equally */
void KRMat4::scale(float s) {
    scale(s,s,s);
}

 // Initialize with a bias matrix
void KRMat4::bias() {
    static const float BIAS_MATRIX[] = {
        0.5, 0.0, 0.0, 0.0, 
        0.0, 0.5, 0.0, 0.0,
        0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
    };
    memcpy(c, BIAS_MATRIX, sizeof(float) * 16);
}


/* Generate an orthographic view matrix */
void KRMat4::ortho(float left, float right, float top, float bottom, float nearz, float farz) {
    memset(c, 0, sizeof(float) * 16);
    c[0] = 2.0f / (right - left);
    c[5] = 2.0f / (bottom - top);
    c[10] = -1.0f / (farz - nearz);
    c[11] = -nearz / (farz - nearz);
    c[15] = 1.0f;
}

/* Replace matrix with its inverse */
bool KRMat4::invert() {
    // Based on gluInvertMatrix implementation
    
    float inv[16], det;
    int i;
    
    inv[0] =   c[5]*c[10]*c[15] - c[5]*c[11]*c[14] - c[9]*c[6]*c[15]
    + c[9]*c[7]*c[14] + c[13]*c[6]*c[11] - c[13]*c[7]*c[10];
    inv[4] =  -c[4]*c[10]*c[15] + c[4]*c[11]*c[14] + c[8]*c[6]*c[15]
    - c[8]*c[7]*c[14] - c[12]*c[6]*c[11] + c[12]*c[7]*c[10];
    inv[8] =   c[4]*c[9]*c[15] - c[4]*c[11]*c[13] - c[8]*c[5]*c[15]
    + c[8]*c[7]*c[13] + c[12]*c[5]*c[11] - c[12]*c[7]*c[9];
    inv[12] = -c[4]*c[9]*c[14] + c[4]*c[10]*c[13] + c[8]*c[5]*c[14]
    - c[8]*c[6]*c[13] - c[12]*c[5]*c[10] + c[12]*c[6]*c[9];
    inv[1] =  -c[1]*c[10]*c[15] + c[1]*c[11]*c[14] + c[9]*c[2]*c[15]
    - c[9]*c[3]*c[14] - c[13]*c[2]*c[11] + c[13]*c[3]*c[10];
    inv[5] =   c[0]*c[10]*c[15] - c[0]*c[11]*c[14] - c[8]*c[2]*c[15]
    + c[8]*c[3]*c[14] + c[12]*c[2]*c[11] - c[12]*c[3]*c[10];
    inv[9] =  -c[0]*c[9]*c[15] + c[0]*c[11]*c[13] + c[8]*c[1]*c[15]
    - c[8]*c[3]*c[13] - c[12]*c[1]*c[11] + c[12]*c[3]*c[9];
    inv[13] =  c[0]*c[9]*c[14] - c[0]*c[10]*c[13] - c[8]*c[1]*c[14]
    + c[8]*c[2]*c[13] + c[12]*c[1]*c[10] - c[12]*c[2]*c[9];
    inv[2] =   c[1]*c[6]*c[15] - c[1]*c[7]*c[14] - c[5]*c[2]*c[15]
    + c[5]*c[3]*c[14] + c[13]*c[2]*c[7] - c[13]*c[3]*c[6];
    inv[6] =  -c[0]*c[6]*c[15] + c[0]*c[7]*c[14] + c[4]*c[2]*c[15]
    - c[4]*c[3]*c[14] - c[12]*c[2]*c[7] + c[12]*c[3]*c[6];
    inv[10] =  c[0]*c[5]*c[15] - c[0]*c[7]*c[13] - c[4]*c[1]*c[15]
    + c[4]*c[3]*c[13] + c[12]*c[1]*c[7] - c[12]*c[3]*c[5];
    inv[14] = -c[0]*c[5]*c[14] + c[0]*c[6]*c[13] + c[4]*c[1]*c[14]
    - c[4]*c[2]*c[13] - c[12]*c[1]*c[6] + c[12]*c[2]*c[5];
    inv[3] =  -c[1]*c[6]*c[11] + c[1]*c[7]*c[10] + c[5]*c[2]*c[11]
    - c[5]*c[3]*c[10] - c[9]*c[2]*c[7] + c[9]*c[3]*c[6];
    inv[7] =   c[0]*c[6]*c[11] - c[0]*c[7]*c[10] - c[4]*c[2]*c[11]
    + c[4]*c[3]*c[10] + c[8]*c[2]*c[7] - c[8]*c[3]*c[6];
    inv[11] = -c[0]*c[5]*c[11] + c[0]*c[7]*c[9] + c[4]*c[1]*c[11]
    - c[4]*c[3]*c[9] - c[8]*c[1]*c[7] + c[8]*c[3]*c[5];
    inv[15] =  c[0]*c[5]*c[10] - c[0]*c[6]*c[9] - c[4]*c[1]*c[10]
    + c[4]*c[2]*c[9] + c[8]*c[1]*c[6] - c[8]*c[2]*c[5];
    
    det = c[0]*inv[0] + c[1]*inv[4] + c[2]*inv[8] + c[3]*inv[12];
    
    if (det == 0) {
        return false;
    }
    
    det = 1.0 / det;
    
    for (i = 0; i < 16; i++) {
        c[i] = inv[i] * det;
    }
    
    return true;
}

void KRMat4::transpose() {
    float trans[16];
    for(int x=0; x<4; x++) {
        for(int y=0; y<4; y++) {
            trans[x + y * 4] = c[y + x * 4];
        }
    }
    memcpy(c, trans, sizeof(float) * 16);
}

/* Dot Product, returning KRVector3 */
KRVector3 KRMat4::Dot(const KRMat4 &m, const KRVector3 &v) {
    return KRVector3(
        v.c[0] * m.c[0] + v.c[1] * m.c[4] + v.c[2] * m.c[8]  + m.c[12],
        v.c[0] * m.c[1] + v.c[1] * m.c[5] + v.c[2] * m.c[9]  + m.c[13],
        v.c[0] * m.c[2] + v.c[1] * m.c[6] + v.c[2] * m.c[10] + m.c[14]
    );
}

KRVector4 KRMat4::Dot4(const KRMat4 &m, const KRVector4 &v) {
#ifdef KRAKEN_USE_ARM_NEON

    KRVector4 d;
    asm volatile (
                  "vld1.32                {d0, d1}, [%1]                  \n\t"   //Q0 = v
                  "vld1.32                {d18, d19}, [%0]!               \n\t"   //Q1 = m
                  "vld1.32                {d20, d21}, [%0]!               \n\t"   //Q2 = m+4
                  "vld1.32                {d22, d23}, [%0]!               \n\t"   //Q3 = m+8
                  "vld1.32                {d24, d25}, [%0]!               \n\t"   //Q4 = m+12
                  
                  "vmul.f32               q13, q9, d0[0]                  \n\t"   //Q5 = Q1*Q0[0]
                  "vmla.f32               q13, q10, d0[1]                 \n\t"   //Q5 += Q1*Q0[1]
                  "vmla.f32               q13, q11, d1[0]                 \n\t"   //Q5 += Q2*Q0[2]
                  "vmla.f32               q13, q12, d1[1]                 \n\t"   //Q5 += Q3*Q0[3]
                  
                  "vst1.32                {d26, d27}, [%2]                \n\t"   //Q4 = m+12
                  : /* no output registers */
                  : "r"(m.c), "r"(v.c), "r"(d.c)
                  : "q0", "q9", "q10","q11", "q12", "q13", "memory"
                  );
    return d;
#else
    return KRVector4(
        v.c[0] * m.c[0] + v.c[1] * m.c[4] + v.c[2] * m.c[8]  + m.c[12],
        v.c[0] * m.c[1] + v.c[1] * m.c[5] + v.c[2] * m.c[9]  + m.c[13],
        v.c[0] * m.c[2] + v.c[1] * m.c[6] + v.c[2] * m.c[10] + m.c[14],
        v.c[0] * m.c[3] + v.c[1] * m.c[7] + v.c[2] * m.c[11] + m.c[15]
    );
#endif
}

// Dot product without including translation; useful for transforming normals and tangents
KRVector3 KRMat4::DotNoTranslate(const KRMat4 &m, const KRVector3 &v)
{
    return KRVector3(
         v.x * m.c[0] + v.y * m.c[4] + v.z * m.c[8],
         v.x * m.c[1] + v.y * m.c[5] + v.z * m.c[9],
         v.x * m.c[2] + v.y * m.c[6] + v.z * m.c[10]
    );
}

/* Dot Product, returning w component as if it were a KRVector4 (This will be deprecated once KRVector4 is implemented instead*/
float KRMat4::DotW(const KRMat4 &m, const KRVector3 &v) {
    return v.x * m.c[0*4 + 3] + v.y * m.c[1*4 + 3] + v.z * m.c[2*4 + 3] + m.c[3*4 + 3];
}

/* Dot Product followed by W-divide */
KRVector3 KRMat4::DotWDiv(const KRMat4 &m, const KRVector3 &v) {
    KRVector4 r = Dot4(m, KRVector4(v, 1.0f));
    return KRVector3(r) / r.w;
}

KRMat4 KRMat4::LookAt(const KRVector3 &cameraPos, const KRVector3 &lookAtPos, const KRVector3 &upDirection)
{
    KRMat4 matLookat;
    KRVector3 lookat_z_axis = lookAtPos - cameraPos;
    lookat_z_axis.normalize();
    KRVector3 lookat_x_axis = KRVector3::Cross(upDirection, lookat_z_axis);
    lookat_x_axis.normalize();
    KRVector3 lookat_y_axis = KRVector3::Cross(lookat_z_axis, lookat_x_axis);
    
    matLookat.getPointer()[0] = lookat_x_axis.x;
    matLookat.getPointer()[1] = lookat_y_axis.x;
    matLookat.getPointer()[2] = lookat_z_axis.x;
    
    matLookat.getPointer()[4] = lookat_x_axis.y;
    matLookat.getPointer()[5] = lookat_y_axis.y;
    matLookat.getPointer()[6] = lookat_z_axis.y;
    
    matLookat.getPointer()[8] = lookat_x_axis.z;
    matLookat.getPointer()[9] = lookat_y_axis.z;
    matLookat.getPointer()[10] = lookat_z_axis.z;
    
    matLookat.getPointer()[12] = -KRVector3::Dot(lookat_x_axis, cameraPos);
    matLookat.getPointer()[13] = -KRVector3::Dot(lookat_y_axis, cameraPos);
    matLookat.getPointer()[14] = -KRVector3::Dot(lookat_z_axis, cameraPos);
    
    return matLookat;
}

KRMat4 KRMat4::Invert(const KRMat4 &m)
{
    KRMat4 matInvert = m;
    matInvert.invert();
    return matInvert;
}

KRMat4 KRMat4::Transpose(const KRMat4 &m)
{
    KRMat4 matTranspose = m;
    matTranspose.transpose();
    return matTranspose;
}

void KRMat4::setUniform(GLint location) const
{
    if(location != -1) GLDEBUG(glUniformMatrix4fv(location, 1, GL_FALSE, c));
}

KRMat4 KRMat4::Translation(const KRVector3 &v)
{
    KRMat4 m;
    m[12] = v.x;
    m[13] = v.y;
    m[14] = v.z;
//    m.translate(v);
    return m;
}

KRMat4 KRMat4::Rotation(const KRVector3 &v)
{
    KRMat4 m;
    m.rotate(v.x, X_AXIS);
    m.rotate(v.y, Y_AXIS);
    m.rotate(v.z, Z_AXIS);
    return m;
}

KRMat4 KRMat4::Scaling(const KRVector3 &v)
{
    KRMat4 m;
    m.scale(v);
    return m;
}

