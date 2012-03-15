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

#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

#import "KRVector3.h"

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


class KRMat4 {
    
    GLfloat m_mat[16];
    
public:
    
    // Default constructor - Creates an identity matrix
    KRMat4();
    
    KRMat4(GLfloat *pMat);
    
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
    
    // Overload multiply operator
    //KRMat4& operator*(const KRMat4 &m);
    KRMat4 operator*(const KRMat4 &m);
    
    GLfloat *getPointer();
    
    void perspective(GLfloat fov, GLfloat aspect, GLfloat nearz, GLfloat farz);
    void ortho(GLfloat left, GLfloat right, GLfloat top, GLfloat bottom, GLfloat nearz, GLfloat farz);
    void translate(GLfloat x, GLfloat y, GLfloat z);
    void scale(GLfloat x, GLfloat y, GLfloat z);
    void scale(GLfloat s);
    void rotate(GLfloat angle, AXIS axis);
    void bias();
    bool invert();
    KRVector3 dot(const KRVector3 &v) const;
};

#endif // KRMAT4_I