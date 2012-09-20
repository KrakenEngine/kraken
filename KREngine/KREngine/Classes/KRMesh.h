//
//  KRMesh.h
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

#import <stdint.h>
#import <vector>
#import <set>
#import <list>
#import <string>
#import "KRMesh.h"
#import "KRVector2.h"
#import "KRVector3.h"
#import "KRResource.h"
#import "KRDataBlock.h"

#import "KREngine-common.h"

using std::vector;
using std::set;
using std::list;


#define MAX_VBO_SIZE 65535
// MAX_VBO_SIZE must be divisible by 3 so triangles aren't split across VBO objects...

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#ifndef KREngine_KRMesh_h
#define KREngine_KRMesh_h

using std::vector;

class KRMesh : public KRResource {
public:    
    KRMesh(KRContext &context, std::string name);
    virtual ~KRMesh();
    
    virtual std::string getExtension();
    virtual bool save(const std::string& path);
    
    void LoadData(std::vector<KRVector3> vertices, std::vector<KRVector2> uva, std::vector<KRVector2> uvb, std::vector<KRVector3> normals, std::vector<KRVector3> tangents, std::vector<int> submesh_starts, std::vector<int> submesh_lengths, std::vector<std::string> material_names);
    void loadPack(KRDataBlock *data);
    
    
    void renderSubmesh(int iSubmesh);
    
    GLfloat getMaxDimension();
    
    KRVector3 getMinPoint() const;
    KRVector3 getMaxPoint() const;
    
    typedef struct {
        GLint start_vertex;
        GLsizei vertex_count;
        char szMaterialName[256];
    } Submesh;
    
    typedef struct {
        GLfloat x;
        GLfloat y;
        GLfloat z;
    } KRVector3D;
    
    typedef struct {
        GLfloat u;
        GLfloat v;
    } TexCoord;
    
    typedef struct {
        KRVector3D vertex;
        KRVector3D normal;
        KRVector3D tangent;
        TexCoord uva;
        TexCoord uvb;
    } VertexData;
    
    VertexData *getVertexData();
    
    vector<Submesh *> getSubmeshes();
    
    typedef struct {
        int32_t start_vertex;
        int32_t vertex_count;
        char szName[256];
    } pack_material;
    
protected:
    KRVector3 m_minPoint, m_maxPoint;

    KRDataBlock *m_pData;
    
    typedef struct {
        char szTag[16];
        float minx, miny, minz, maxx, maxy, maxz;
        int32_t vertex_count;
        int32_t submesh_count;
    } pack_header;
    
    vector<Submesh *> m_submeshes;
    
    void clearData();
    void clearBuffers();
};


#endif
