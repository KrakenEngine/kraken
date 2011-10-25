//
//  KRMat4.cpp
//  gldemo
//
//  Created by Kearwood Gilbert on 10-09-21.
//  Copyright (c) 2010 Kearwood Software. All rights reserved.
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
KRMat4& KRMat4::operator*(const KRMat4 &m) {
    KRMat4 result = *this;
    result *= m;
    return result;
}



/* Generate a perspective view matrix using a field of view angle fov,
 * window aspect ratio, near and far clipping planes */
void KRMat4::perspective(GLfloat fov, GLfloat aspect, GLfloat nearz, GLfloat farz) {
    GLfloat range;
    
    range = tan(fov * 0.00872664625) * nearz; /* 0.00872664625 = PI/360 */
    memset(m_mat, 0, sizeof(GLfloat) * 16);
    m_mat[0] = (2 * nearz) / ((range * aspect) - (-range * aspect));
    m_mat[5] = (2 * nearz) / (2 * range);
    m_mat[10] = -(farz + nearz) / (farz - nearz);
    m_mat[11] = -1;
    m_mat[14] = -(2 * farz * nearz) / (farz - nearz);
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
    // const GLfloat d2r = 0.0174532925199; /* PI / 180 */
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

void KRMat4::scale(GLfloat x, GLfloat y, GLfloat z) {
    KRMat4 newMatrix; // Create new identity matrix
    
    newMatrix.m_mat[0] = x;
    newMatrix.m_mat[5] = y;
    newMatrix.m_mat[10] = z;
    
    *this *= newMatrix;
}

void KRMat4::scale(GLfloat s) {
    scale(s,s,s);
}
