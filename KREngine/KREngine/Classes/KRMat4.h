//
//  KRMat4.h
//  gldemo
//
//  Created by Kearwood Gilbert on 10-09-21.
//  Copyright (c) 2010 Kearwood Software. All rights reserved.
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
    Vector3 dot(const Vector3 &v) const;
};

#endif // KRMAT4_I