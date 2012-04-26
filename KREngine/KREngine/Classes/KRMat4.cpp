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

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "KRMat4.h"

KRMat4::KRMat4() {
    // Default constructor - Initialize with an identity matrix
    static const GLfloat IDENTITY_MATRIX[] = {
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    };
    memcpy(m_mat, IDENTITY_MATRIX, sizeof(GLfloat) * 16);
    
}

KRMat4::KRMat4(GLfloat *pMat) {
    memcpy(m_mat, pMat, sizeof(GLfloat) * 16);
}

KRMat4::~KRMat4() {
    
}

GLfloat *KRMat4::getPointer() {
    return m_mat;
}

// Copy constructor
KRMat4::KRMat4(const KRMat4 &m) {
    memcpy(m_mat, m.m_mat, sizeof(GLfloat) * 16);
}

KRMat4& KRMat4::operator=(const KRMat4 &m) {
    if(this != &m) { // Prevent self-assignment.
        memcpy(m_mat, m.m_mat, sizeof(GLfloat) * 16);
    }
    return *this;
}

// Overload comparison operator
bool KRMat4::operator==(const KRMat4 &m) {
    return memcmp(m_mat, m.m_mat, sizeof(GLfloat) * 16) == 0;
}

// Overload compound multiply operator
KRMat4& KRMat4::operator*=(const KRMat4 &m) {
    GLfloat temp[16];
    
    int x,y;
    
    for (x=0; x < 4; x++)
    {
        for(y=0; y < 4; y++)
        {
            temp[y + (x*4)] = (m_mat[x*4] * m.m_mat[y]) +
            (m_mat[(x*4)+1] * m.m_mat[y+4]) +
            (m_mat[(x*4)+2] * m.m_mat[y+8]) +
            (m_mat[(x*4)+3] * m.m_mat[y+12]);
        }
    }
    
    memcpy(m_mat, temp, sizeof(GLfloat) << 4);
    return *this;
}

// Overload multiply operator
KRMat4 KRMat4::operator*(const KRMat4 &m) {
    KRMat4 ret = *this;
    ret *= m;
    return ret;
}


/* Generate a perspective view matrix using a field of view angle fov,
 * window aspect ratio, near and far clipping planes */
void KRMat4::perspective(GLfloat fov, GLfloat aspect, GLfloat nearz, GLfloat farz) {
   
    memset(m_mat, 0, sizeof(GLfloat) * 16);
    
    GLfloat range= tan(fov / 2.0f) * nearz; 
    m_mat[0] = (2 * nearz) / ((range * aspect) - (-range * aspect));
    m_mat[5] = (2 * nearz) / (2 * range);
    m_mat[10] = -(farz + nearz) / (farz - nearz);
    m_mat[11] = -1;
    m_mat[14] = -(2 * farz * nearz) / (farz - nearz);
    /*
    GLfloat range= atan(fov / 20.0f) * nearz; 
    GLfloat r = range * aspect;
    GLfloat t = range * 1.0;
    
    m_mat[0] = nearz / r;
    m_mat[5] = nearz / t;
    m_mat[10] = -(farz + nearz) / (farz - nearz);
    m_mat[11] = -(2.0 * farz * nearz) / (farz - nearz);
    m_mat[14] = -1.0;
    */
}

/* Perform translation operations on a matrix */
void KRMat4::translate(GLfloat x, GLfloat y, GLfloat z) {
    KRMat4 newMatrix; // Create new identity matrix
    
    newMatrix.m_mat[12] = x;
    newMatrix.m_mat[13] = y;
    newMatrix.m_mat[14] = z;
    
    *this *= newMatrix;
}

/* Rotate a matrix by an angle on a X, Y, or Z axis */
void KRMat4::rotate(GLfloat angle, AXIS axis) {
    const int cos1[3] = { 5, 0, 0 };
    const int cos2[3] = { 10, 10, 5 };
    const int sin1[3] = { 6, 2, 1 };
    const int sin2[3] = { 9, 8, 4 };
    
    KRMat4 newMatrix; // Create new identity matrix
    
    newMatrix.m_mat[cos1[axis]] = cos(angle);
    newMatrix.m_mat[sin1[axis]] = -sin(angle);
    newMatrix.m_mat[sin2[axis]] = -newMatrix.m_mat[sin1[axis]];
    newMatrix.m_mat[cos2[axis]] = newMatrix.m_mat[cos1[axis]];
    
    *this *= newMatrix;
}

/* Scale matrix by separate x, y, and z amounts */
void KRMat4::scale(GLfloat x, GLfloat y, GLfloat z) {
    KRMat4 newMatrix; // Create new identity matrix
    
    newMatrix.m_mat[0] = x;
    newMatrix.m_mat[5] = y;
    newMatrix.m_mat[10] = z;
    
    *this *= newMatrix;
}

/* Scale all dimensions equally */
void KRMat4::scale(GLfloat s) {
    scale(s,s,s);
}

 // Initialize with a bias matrix
void KRMat4::bias() {
    static const GLfloat BIAS_MATRIX[] = {
        0.5, 0.0, 0.0, 0.0, 
        0.0, 0.5, 0.0, 0.0,
        0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
    };
    memcpy(m_mat, BIAS_MATRIX, sizeof(GLfloat) * 16);
}


/* Generate an orthographic view matrix */
void KRMat4::ortho(GLfloat left, GLfloat right, GLfloat top, GLfloat bottom, GLfloat nearz, GLfloat farz) {
    memset(m_mat, 0, sizeof(GLfloat) * 16);
    m_mat[0] = 2.0f / (right - left);
    m_mat[5] = 2.0f / (bottom - top);
    m_mat[10] = -1.0f / (farz - nearz);
    m_mat[11] = -nearz / (farz - nearz);
    m_mat[15] = 1.0f;
}

/* Replace matrix with its inverse */
bool KRMat4::invert() {
    // Based on gluInvertMatrix implementation
    
    double inv[16], det;
    int i;
    
    inv[0] =   m_mat[5]*m_mat[10]*m_mat[15] - m_mat[5]*m_mat[11]*m_mat[14] - m_mat[9]*m_mat[6]*m_mat[15]
    + m_mat[9]*m_mat[7]*m_mat[14] + m_mat[13]*m_mat[6]*m_mat[11] - m_mat[13]*m_mat[7]*m_mat[10];
    inv[4] =  -m_mat[4]*m_mat[10]*m_mat[15] + m_mat[4]*m_mat[11]*m_mat[14] + m_mat[8]*m_mat[6]*m_mat[15]
    - m_mat[8]*m_mat[7]*m_mat[14] - m_mat[12]*m_mat[6]*m_mat[11] + m_mat[12]*m_mat[7]*m_mat[10];
    inv[8] =   m_mat[4]*m_mat[9]*m_mat[15] - m_mat[4]*m_mat[11]*m_mat[13] - m_mat[8]*m_mat[5]*m_mat[15]
    + m_mat[8]*m_mat[7]*m_mat[13] + m_mat[12]*m_mat[5]*m_mat[11] - m_mat[12]*m_mat[7]*m_mat[9];
    inv[12] = -m_mat[4]*m_mat[9]*m_mat[14] + m_mat[4]*m_mat[10]*m_mat[13] + m_mat[8]*m_mat[5]*m_mat[14]
    - m_mat[8]*m_mat[6]*m_mat[13] - m_mat[12]*m_mat[5]*m_mat[10] + m_mat[12]*m_mat[6]*m_mat[9];
    inv[1] =  -m_mat[1]*m_mat[10]*m_mat[15] + m_mat[1]*m_mat[11]*m_mat[14] + m_mat[9]*m_mat[2]*m_mat[15]
    - m_mat[9]*m_mat[3]*m_mat[14] - m_mat[13]*m_mat[2]*m_mat[11] + m_mat[13]*m_mat[3]*m_mat[10];
    inv[5] =   m_mat[0]*m_mat[10]*m_mat[15] - m_mat[0]*m_mat[11]*m_mat[14] - m_mat[8]*m_mat[2]*m_mat[15]
    + m_mat[8]*m_mat[3]*m_mat[14] + m_mat[12]*m_mat[2]*m_mat[11] - m_mat[12]*m_mat[3]*m_mat[10];
    inv[9] =  -m_mat[0]*m_mat[9]*m_mat[15] + m_mat[0]*m_mat[11]*m_mat[13] + m_mat[8]*m_mat[1]*m_mat[15]
    - m_mat[8]*m_mat[3]*m_mat[13] - m_mat[12]*m_mat[1]*m_mat[11] + m_mat[12]*m_mat[3]*m_mat[9];
    inv[13] =  m_mat[0]*m_mat[9]*m_mat[14] - m_mat[0]*m_mat[10]*m_mat[13] - m_mat[8]*m_mat[1]*m_mat[14]
    + m_mat[8]*m_mat[2]*m_mat[13] + m_mat[12]*m_mat[1]*m_mat[10] - m_mat[12]*m_mat[2]*m_mat[9];
    inv[2] =   m_mat[1]*m_mat[6]*m_mat[15] - m_mat[1]*m_mat[7]*m_mat[14] - m_mat[5]*m_mat[2]*m_mat[15]
    + m_mat[5]*m_mat[3]*m_mat[14] + m_mat[13]*m_mat[2]*m_mat[7] - m_mat[13]*m_mat[3]*m_mat[6];
    inv[6] =  -m_mat[0]*m_mat[6]*m_mat[15] + m_mat[0]*m_mat[7]*m_mat[14] + m_mat[4]*m_mat[2]*m_mat[15]
    - m_mat[4]*m_mat[3]*m_mat[14] - m_mat[12]*m_mat[2]*m_mat[7] + m_mat[12]*m_mat[3]*m_mat[6];
    inv[10] =  m_mat[0]*m_mat[5]*m_mat[15] - m_mat[0]*m_mat[7]*m_mat[13] - m_mat[4]*m_mat[1]*m_mat[15]
    + m_mat[4]*m_mat[3]*m_mat[13] + m_mat[12]*m_mat[1]*m_mat[7] - m_mat[12]*m_mat[3]*m_mat[5];
    inv[14] = -m_mat[0]*m_mat[5]*m_mat[14] + m_mat[0]*m_mat[6]*m_mat[13] + m_mat[4]*m_mat[1]*m_mat[14]
    - m_mat[4]*m_mat[2]*m_mat[13] - m_mat[12]*m_mat[1]*m_mat[6] + m_mat[12]*m_mat[2]*m_mat[5];
    inv[3] =  -m_mat[1]*m_mat[6]*m_mat[11] + m_mat[1]*m_mat[7]*m_mat[10] + m_mat[5]*m_mat[2]*m_mat[11]
    - m_mat[5]*m_mat[3]*m_mat[10] - m_mat[9]*m_mat[2]*m_mat[7] + m_mat[9]*m_mat[3]*m_mat[6];
    inv[7] =   m_mat[0]*m_mat[6]*m_mat[11] - m_mat[0]*m_mat[7]*m_mat[10] - m_mat[4]*m_mat[2]*m_mat[11]
    + m_mat[4]*m_mat[3]*m_mat[10] + m_mat[8]*m_mat[2]*m_mat[7] - m_mat[8]*m_mat[3]*m_mat[6];
    inv[11] = -m_mat[0]*m_mat[5]*m_mat[11] + m_mat[0]*m_mat[7]*m_mat[9] + m_mat[4]*m_mat[1]*m_mat[11]
    - m_mat[4]*m_mat[3]*m_mat[9] - m_mat[8]*m_mat[1]*m_mat[7] + m_mat[8]*m_mat[3]*m_mat[5];
    inv[15] =  m_mat[0]*m_mat[5]*m_mat[10] - m_mat[0]*m_mat[6]*m_mat[9] - m_mat[4]*m_mat[1]*m_mat[10]
    + m_mat[4]*m_mat[2]*m_mat[9] + m_mat[8]*m_mat[1]*m_mat[6] - m_mat[8]*m_mat[2]*m_mat[5];
    
    det = m_mat[0]*inv[0] + m_mat[1]*inv[4] + m_mat[2]*inv[8] + m_mat[3]*inv[12];
    
    if (det == 0) {
        return false;
    }
    
    det = 1.0 / det;
    
    for (i = 0; i < 16; i++) {
        m_mat[i] = inv[i] * det;
    }
    
    return true;
}

void KRMat4::transpose() {
    GLfloat trans[16];
    for(int x=0; x<4; x++) {
        for(int y=0; y<4; y++) {
            trans[x + y * 4] = m_mat[y + x * 4];
        }
    }
    memcpy(m_mat, trans, sizeof(GLfloat) * 16);
}

/* Dot Product */
KRVector3 KRMat4::dot(const KRVector3 &v) const {
    return KRVector3(
        v.x * (float)m_mat[0*4 + 0] + v.y * (float)m_mat[1*4 + 0] + v.z * (float)m_mat[2*4 + 0] + (float)m_mat[3*4 + 0],
        v.x * (float)m_mat[0*4 + 1] + v.y * (float)m_mat[1*4 + 1] + v.z * (float)m_mat[2*4 + 1] + (float)m_mat[3*4 + 1],
        v.x * (float)m_mat[0*4 + 2] + v.y * (float)m_mat[1*4 + 2] + v.z * (float)m_mat[2*4 + 2] + (float)m_mat[3*4 + 2]
    );
}
