//
//  KRMesh.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-03-15.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import <stdint.h>
#import <vector>
#import <string>

#define MAX_VBO_SIZE 65535
// MAX_VBO_SIZE must be divisible by 3 so triangles aren't split across VBO objects...

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#ifndef KREngine_KRMesh_h
#define KREngine_KRMesh_h



using std::vector;

class KRMesh {
public:
    KRMesh();
    ~KRMesh();
    
    void loadPack(std::string path);
    void renderSubmesh(int iSubmesh);
    
    GLfloat getMaxDimension();
    GLfloat getMinX();
    GLfloat getMaxX();
    GLfloat getMinY();
    GLfloat getMaxY();
    GLfloat getMinZ();
    GLfloat getMaxZ();
    
    typedef struct {
        GLint start_vertex;
        GLsizei vertex_count;
        char szMaterialName[64];
    } Submesh;
    
    vector<Submesh *> getSubmeshes();
    
protected:
    GLfloat m_minx, m_miny, m_minz, m_maxx, m_maxy, m_maxz;

    int m_fdPackFile;
    void *m_pPackData;
    int m_iPackFileSize;
    
    typedef struct {
        char szTag[16];
        float minx, miny, minz, maxx, maxy, maxz;
        int32_t vertex_count;
        int32_t submesh_count;
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
    

    
    GLsizei m_cBuffers;
    GLuint *m_pBuffers;
    
    vector<Submesh *> m_submeshes;
};


#endif
