//
//  KRModel.h
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
#import <string>
#import "KRVector2.h"
#import "KRContext.h"
#import "KRBone.h"

#import "KREngine-common.h"

using std::vector;
using std::set;
using std::list;


#define MAX_VBO_SIZE 65535
#define KRENGINE_MAX_BONE_WEIGHTS_PER_VERTEX 4
#define KRENGINE_MAX_NAME_LENGTH 256
// MAX_VBO_SIZE must be divisible by 3 so triangles aren't split across VBO objects...

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#ifndef KRMODEL_I
#define KRMODEL_I

#import "KRMaterialManager.h"
#import "KRCamera.h"
#import "KRViewport.h"

class KRMaterial;
class KRNode;


class KRModel : public KRResource {
    
public:
    KRModel(KRContext &context, std::string name, KRDataBlock *data);
    KRModel(KRContext &context, std::string name);
    virtual ~KRModel();
    
    bool hasTransparency();
    
    typedef enum {
        KRENGINE_ATTRIB_VERTEX = 0,
        KRENGINE_ATTRIB_NORMAL,
        KRENGINE_ATTRIB_TANGENT,
        KRENGINE_ATTRIB_TEXUVA,
        KRENGINE_ATTRIB_TEXUVB,
        KRENGINE_ATTRIB_BONEINDEXES,
        KRENGINE_ATTRIB_BONEWEIGHTS,
        KRENGINE_NUM_ATTRIBUTES
    } vertex_attrib_t;
    
    typedef enum {
        KRENGINE_MODEL_FORMAT_TRIANGLES = 0,
        KRENGINE_MODEL_FORMAT_STRIP,
        KRENGINE_MODEL_FORMAT_INDEXED_TRIANGLES,
        KRENGINE_MODEL_FORMAT_INDEXED_STRIP
    } model_format_t;
    
#if TARGET_OS_IPHONE
    
    void render(KRCamera *pCamera, std::vector<KRLight *> &lights, const KRViewport &viewport, const KRMat4 &matModel, KRTexture *pLightMap, KRNode::RenderPass renderPass, const std::vector<KRBone *> &bones);
    
#endif
    
    std::string m_lodBaseName;
    
    virtual std::string getExtension();
    virtual bool save(const std::string& path);
    
    void LoadData(std::vector<KRVector3> vertices, std::vector<KRVector2> uva, std::vector<KRVector2> uvb, std::vector<KRVector3> normals, std::vector<KRVector3> tangents, std::vector<int> submesh_starts, std::vector<int> submesh_lengths, std::vector<std::string> material_names, std::vector<std::string> bone_names, std::vector<std::vector<int> > bone_indexes, std::vector<std::vector<float> > bone_weights, model_format_t model_format);
    void loadPack(KRDataBlock *data);
    
    
    void renderSubmesh(int iSubmesh);
    
    GLfloat getMaxDimension();
    
    KRVector3 getMinPoint() const;
    KRVector3 getMaxPoint() const;
    

    
    typedef struct {
        GLint start_vertex;
        GLsizei vertex_count;
        char szMaterialName[KRENGINE_MAX_NAME_LENGTH];
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
    
    vector<Submesh *> getSubmeshes();
    
    typedef struct {
        int32_t start_vertex;
        int32_t vertex_count;
        char szName[KRENGINE_MAX_NAME_LENGTH];
    } pack_material;
    
    typedef struct {
        char szName[KRENGINE_MAX_NAME_LENGTH];
    } pack_bone;

    int getLODCoverage() const;
    std::string getLODBaseName() const;
    
    
    static bool lod_sort_predicate(const KRModel *m1, const KRModel *m2);
    bool has_vertex_attribute(vertex_attrib_t attribute_type) const;
    
    int getSubmeshCount();
    int getVertexCount(int submesh);
    KRVector3 getVertexPosition(int index) const;
    KRVector3 getVertexNormal(int index) const;
    KRVector3 getVertexTangent(int index) const;
    KRVector2 getVertexUVA(int index) const;
    KRVector2 getVertexUVB(int index) const;
    int getBoneIndex(int index, int weight_index) const;
    float getBoneWeight(int index, int weight_index) const;
    
    void setVertexPosition(int index, const KRVector3 &v);
    void setVertexNormal(int index, const KRVector3 &v);
    void setVertexTangent(int index, const KRVector3 & v);
    void setVertexUVA(int index, const KRVector2 &v);
    void setVertexUVB(int index, const KRVector2 &v);
    void setBoneIndex(int index, int weight_index, int bone_index);
    void setBoneWeight(int index, int weight_index, float bone_weight);
    
    static size_t VertexSizeForAttributes(__int32_t vertex_attrib_flags);
    static size_t AttributeOffset(__int32_t vertex_attrib, __int32_t vertex_attrib_flags);
    
    int getBoneCount();
    char *getBoneName(int bone_index);

    
    model_format_t getModelFormat();
private:
    
    
    int m_lodCoverage; // This LOD level is activated when the bounding box of the model will cover less than this percent of the screen (100 = highest detail model)
    vector<KRMaterial *> m_materials;
    set<KRMaterial *> m_uniqueMaterials;
    
    bool m_hasTransparency;
    
    
    KRVector3 m_minPoint, m_maxPoint;
    
    KRDataBlock *m_pData;
    

    
    typedef struct {
        char szTag[16];
        int32_t model_format; // 0 == Triangle list, 1 == Triangle strips, 2 == Indexed triangle list, 3 == Indexed triangle strips, rest are reserved (model_format_t enum)
        int32_t vertex_attrib_flags;
        int32_t vertex_count;
        int32_t submesh_count;
        int32_t bone_count;
        float minx, miny, minz, maxx, maxy, maxz; // Axis aligned bounding box, in model's coordinate space
        unsigned char reserved[452]; // Pad out to 512 bytes
    } pack_header;
    
    vector<Submesh *> m_submeshes;
    int m_vertex_attribute_offset[KRENGINE_NUM_ATTRIBUTES];
    int m_vertex_size;
    void updateAttributeOffsets();
    
    
    void clearData();
    void clearBuffers();
    
    void setName(const std::string name);
    void optimize();
    
    
    
    pack_material *getSubmesh(int mesh_index);
    unsigned char *getVertexData() const;
    unsigned char *getVertexData(int index) const;
    pack_header *getHeader() const;
    pack_bone *getBone(int index);
};


#endif // KRMODEL_I