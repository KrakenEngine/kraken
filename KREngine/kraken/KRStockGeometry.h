//
//  KRStockGeometry.h
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-09-19.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KRSTOCKGEOMETRY_H
#define KRSTOCKGEOMETRY_H

#include "KRMesh.h"

static const GLfloat KRENGINE_VBO_3D_CUBE[] = {
    1.0, 1.0, 1.0,
    -1.0, 1.0, 1.0,
    1.0,-1.0, 1.0,
    -1.0,-1.0, 1.0,
    -1.0,-1.0,-1.0,
    -1.0, 1.0, 1.0,
    -1.0, 1.0,-1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0,-1.0,
    1.0,-1.0, 1.0,
    1.0,-1.0,-1.0,
    -1.0,-1.0,-1.0,
    1.0, 1.0,-1.0,
    -1.0, 1.0,-1.0
};

static int KRENGINE_VBO_3D_CUBE_SIZE = sizeof(GLfloat) * 3 * 14;
static const __int32_t KRENGINE_VBO_3D_CUBE_ATTRIBS = (1 << KRMesh::KRENGINE_ATTRIB_VERTEX);

static const GLfloat KRENGINE_VERTICES_2D_SQUARE[] = {
    -1.0f, -1.0f,
    1.0f, -1.0f,
    -1.0f,  1.0f,
    1.0f,  1.0f,
};

static const GLfloat KRENGINE_VERTICES_2D_SQUARE_UV[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f,  1.0f,
    1.0f,  1.0f,
};

static const GLfloat KRENGINE_VBO_2D_SQUARE[] = {
    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
    1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
    1.0f,  1.0f, 0.0f, 1.0f, 1.0f
};

static const int KRENGINE_VBO_2D_SQUARE_SIZE = sizeof(GLfloat) * 5 * 4;
static const __int32_t KRENGINE_VBO_2D_SQUARE_ATTRIBS = (1 << KRMesh::KRENGINE_ATTRIB_VERTEX) | (1 << KRMesh::KRENGINE_ATTRIB_TEXUVA);


#endif
