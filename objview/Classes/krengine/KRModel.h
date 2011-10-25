//
//  KRModel.h
//  gldemo
//
//  Created by Kearwood Gilbert on 10-09-22.
//  Copyright (c) 2010 Kearwood Software. All rights reserved.
//

#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import <stdint.h>
#import <vector>

using std::vector;

#ifndef KRMODEL_I
#define KRMODEL_I

#import "KRMaterialManager.h"



class KRModel {
    
public:
    KRModel(std::string path, KRMaterialManager *pMaterialManager);
    ~KRModel();
    
    void render(GLuint program, GLuint iVertexAttrib, GLuint iNormalAttrib, GLuint iTangentAttrib, GLuint iTexCoordAttrib, KRMaterialManager *pMaterialManager);

    GLfloat getMaxDimension();
    GLfloat getMinX();
    GLfloat getMaxX();
    GLfloat getMinY();
    GLfloat getMaxY();
    GLfloat getMinZ();
    GLfloat getMaxZ();
    
private:
    
    void loadWavefront(std::string path, KRMaterialManager *pMaterialManager);
    void loadPack(std::string path, KRMaterialManager *pMaterialManager);
    
    int m_fdPackFile;
    void *m_pPackFile;
    int m_iPackFileSize;
    
    typedef struct {
        char szTag[16];
        float minx, miny, minz, maxx, maxy, maxz;
        int32_t vertex_count;
        int32_t material_count;
    } pack_header;
    
    typedef struct {
        int32_t start_vertex;
        int32_t vertex_count;
        char szName[64];
    } pack_material;
    
    typedef struct {
        GLfloat x;
        GLfloat y;
        GLfloat z;
    } Vertex3D, Vector3D;
    
    typedef struct {
        GLfloat u;
        GLfloat v;
    } TexCoord;
    
    typedef struct {
        Vertex3D vertex;
        Vector3D normal;
        Vector3D tangent;
        TexCoord texcoord;
    } VertexData;
    
    VertexData *m_pVertexData;
    
    typedef struct {
        GLint start_vertex;
        GLsizei vertex_count;
        KRMaterial *pMaterial;
    } Material;
    
    GLsizei m_cBuffers;
    GLuint *m_pBuffers;
    
    vector<Material *> m_materials;
    
    GLfloat m_minx, m_miny, m_minz, m_maxx, m_maxy, m_maxz;

};


#endif // KRMODEL_I