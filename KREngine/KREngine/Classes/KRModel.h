//
//  KRModel.h
//  gldemo
//
//  Copyright 2011 Kearwood Gilbert. All rights reserved.
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
#import <stdint.h>
#import <vector>
#import <string>

using std::vector;

#ifndef KRMODEL_I
#define KRMODEL_I

#import "KRMaterialManager.h"
#import "KRCamera.h"


class KRModel {
    
public:
    KRModel(std::string path, KRMaterialManager *pMaterialManager);
    ~KRModel();
    
    void render(KRCamera *pCamera, KRMaterialManager *pMaterialManager, bool bRenderShadowMap, KRMat4 &mvpMatrix, Vector3 &cameraPosition, Vector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers);

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