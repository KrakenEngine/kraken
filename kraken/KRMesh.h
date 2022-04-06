//
//  KRMesh.h
//  Kraken Engine
//
//  Copyright 2022 Kearwood Gilbert. All rights reserved.
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
#include "KREngine-common.h"

#include "KRContext.h"
#include "KRBone.h"
#include "KRMeshManager.h"

#include "KREngine-common.h"

#include "hydra.h"

using namespace kraken;

#define MAX_VBO_SIZE 65535
#define KRENGINE_MAX_BONE_WEIGHTS_PER_VERTEX 4
#define KRENGINE_MAX_NAME_LENGTH 256
// MAX_VBO_SIZE must be divisible by 3 so triangles aren't split across VBO objects...

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#ifndef KRMesh_I
#define KRMesh_I

#include "KRMaterialManager.h"
#include "KRCamera.h"
#include "KRViewport.h"

class KRMaterial;
class KRNode;

class KRMesh : public KRResource {

public:
    static void parseName(const std::string& name, std::string& lodBaseName, int& lodCoverage);

    KRMesh(KRContext &context, std::string name, KRDataBlock *data);
    KRMesh(KRContext &context, std::string name);
    virtual ~KRMesh();

    kraken_stream_level getStreamLevel();
    void preStream(float lodCoverage);

    bool hasTransparency();

    typedef enum {
        KRENGINE_ATTRIB_VERTEX = 0,
        KRENGINE_ATTRIB_NORMAL,
        KRENGINE_ATTRIB_TANGENT,
        KRENGINE_ATTRIB_TEXUVA,
        KRENGINE_ATTRIB_TEXUVB,
        KRENGINE_ATTRIB_BONEINDEXES,
        KRENGINE_ATTRIB_BONEWEIGHTS,
        KRENGINE_ATTRIB_VERTEX_SHORT,
        KRENGINE_ATTRIB_NORMAL_SHORT,
        KRENGINE_ATTRIB_TANGENT_SHORT,
        KRENGINE_ATTRIB_TEXUVA_SHORT,
        KRENGINE_ATTRIB_TEXUVB_SHORT,
        KRENGINE_NUM_ATTRIBUTES
    } vertex_attrib_t;

    typedef enum {
        KRENGINE_MODEL_FORMAT_TRIANGLES = 0,
        KRENGINE_MODEL_FORMAT_STRIP,
        KRENGINE_MODEL_FORMAT_INDEXED_TRIANGLES,
        KRENGINE_MODEL_FORMAT_INDEXED_STRIP
    } model_format_t;

    typedef struct {
        model_format_t format;
        std::vector<Vector3> vertices;
        std::vector<__uint16_t> vertex_indexes;
        std::vector<std::pair<int, int> > vertex_index_bases;
        std::vector<Vector2> uva;
        std::vector<Vector2> uvb;
        std::vector<Vector3> normals;
        std::vector<Vector3> tangents;
        std::vector<int> submesh_starts;
        std::vector<int> submesh_lengths;
        std::vector<std::string> material_names;
        std::vector<std::string> bone_names;
        std::vector<std::vector<int> > bone_indexes;
        std::vector<Matrix4> bone_bind_poses;
        std::vector<std::vector<float> > bone_weights;
    } mesh_info;

    void render(const KRNode::RenderInfo& ri, const std::string &object_name, const Matrix4 &matModel, KRTexture *pLightMap, const std::vector<KRBone *> &bones, const Vector3 &rim_color, float rim_power, float lod_coverage = 0.0f);

    std::string m_lodBaseName;

    virtual std::string getExtension();
    virtual bool save(const std::string& path);
    virtual bool save(KRDataBlock &data);

    void LoadData(const mesh_info &mi, bool calculate_normals, bool calculate_tangents);
    void loadPack(KRDataBlock *data);

    void convertToIndexed();
    void optimize();
    void optimizeIndexes();

    void renderSubmesh(VkCommandBuffer& commandBuffer, int iSubmesh, KRNode::RenderPass renderPass, const std::string &object_name, const std::string &material_name, float lodCoverage);

    float getMaxDimension();

    Vector3 getMinPoint() const;
    Vector3 getMaxPoint() const;

    class Submesh {
    public:
        Submesh() {};
        ~Submesh() {
            for(auto itr = vbo_data_blocks.begin(); itr != vbo_data_blocks.end(); itr++) {
                delete (*itr);
            }
            for(auto itr = vertex_data_blocks.begin(); itr != vertex_data_blocks.end(); itr++) {
                delete (*itr);
            }
            for(auto itr = index_data_blocks.begin(); itr != index_data_blocks.end(); itr++) {
                delete (*itr);
            }
        };

        GLint start_vertex;
        GLsizei vertex_count;
        char szMaterialName[KRENGINE_MAX_NAME_LENGTH];
        vector<KRDataBlock *> vertex_data_blocks;
        vector<KRDataBlock *> index_data_blocks;
        vector<KRMeshManager::KRVBOData *> vbo_data_blocks;
    };

    typedef struct {
        union {
            struct { // For Indexed triangles / strips
                uint16_t index_group;
                uint16_t index_group_offset;
            };
            int32_t start_vertex; // For non-indexed trigangles / strips
        };
        int32_t vertex_count;
        char szName[KRENGINE_MAX_NAME_LENGTH];
    } pack_material;

    typedef struct {
        char szName[KRENGINE_MAX_NAME_LENGTH];
        float bind_pose[16];
    } pack_bone;

    int getLODCoverage() const;
    std::string getLODBaseName() const;


    static bool lod_sort_predicate(const KRMesh *m1, const KRMesh *m2);
    bool has_vertex_attribute(vertex_attrib_t attribute_type) const;
    static bool has_vertex_attribute(int vertex_attrib_flags, vertex_attrib_t attribute_type);

    int getSubmeshCount() const;
    int getVertexCount(int submesh) const;
    int getTriangleVertexIndex(int submesh, int index) const;
    Vector3 getVertexPosition(int index) const;
    Vector3 getVertexNormal(int index) const;
    Vector3 getVertexTangent(int index) const;
    Vector2 getVertexUVA(int index) const;
    Vector2 getVertexUVB(int index) const;
    int getBoneIndex(int index, int weight_index) const;
    float getBoneWeight(int index, int weight_index) const;

    void setVertexPosition(int index, const Vector3 &v);
    void setVertexNormal(int index, const Vector3 &v);
    void setVertexTangent(int index, const Vector3 & v);
    void setVertexUVA(int index, const Vector2 &v);
    void setVertexUVB(int index, const Vector2 &v);
    void setBoneIndex(int index, int weight_index, int bone_index);
    void setBoneWeight(int index, int weight_index, float bone_weight);

    static size_t VertexSizeForAttributes(__int32_t vertex_attrib_flags);
    static size_t AttributeOffset(__int32_t vertex_attrib, __int32_t vertex_attrib_flags);
    static VkFormat AttributeVulkanFormat(__int32 vertex_attrib);

    int getBoneCount();
    char *getBoneName(int bone_index);
    Matrix4 getBoneBindPose(int bone_index);


    model_format_t getModelFormat() const;

    bool lineCast(const Vector3 &v0, const Vector3 &v1, HitInfo &hitinfo) const;
    bool rayCast(const Vector3 &v0, const Vector3 &dir, HitInfo &hitinfo) const;
    bool sphereCast(const Matrix4 &model_to_world, const Vector3 &v0, const Vector3 &v1, float radius, HitInfo &hitinfo) const;

    static int GetLODCoverage(const std::string &name);

protected:
    bool m_constant; // TRUE if this should be always loaded and should not be passed through the streamer

private:
    KRDataBlock *m_pData;
    KRDataBlock *m_pMetaData;
    KRDataBlock *m_pIndexBaseData;

    void getSubmeshes();
    void getMaterials();

    static bool rayCast(const Vector3 &start, const Vector3 &dir, const Triangle3 &tri, const Vector3 &tri_n0, const Vector3 &tri_n1, const Vector3 &tri_n2, HitInfo &hitinfo);
    static bool sphereCast(const Matrix4 &model_to_world, const Vector3 &v0, const Vector3 &v1, float radius, const Triangle3 &tri, HitInfo &hitinfo);

    int m_lodCoverage; // This LOD level is activated when the bounding box of the model will cover less than this percent of the screen (100 = highest detail model)
    vector<KRMaterial *> m_materials;
    set<KRMaterial *> m_uniqueMaterials;

    bool m_hasTransparency;


    Vector3 m_minPoint, m_maxPoint;




    typedef struct {
        char szTag[16];
        int32_t model_format; // 0 == Triangle list, 1 == Triangle strips, 2 == Indexed triangle list, 3 == Indexed triangle strips, rest are reserved (model_format_t enum)
        int32_t vertex_attrib_flags;
        int32_t vertex_count;
        int32_t submesh_count;
        int32_t bone_count;
        float minx, miny, minz, maxx, maxy, maxz; // Axis aligned bounding box, in model's coordinate space
        int32_t index_count;
        int32_t index_base_count;
        unsigned char reserved[444]; // Pad out to 512 bytes
    } pack_header;

    vector<Submesh *> m_submeshes;
    int m_vertex_attribute_offset[KRENGINE_NUM_ATTRIBUTES];
    int m_vertex_size;
    void updateAttributeOffsets();

    void setName(const std::string name);



    pack_material *getSubmesh(int mesh_index) const;
    unsigned char *getVertexData() const;
    size_t getVertexDataOffset() const;
    unsigned char *getVertexData(int index) const;
    __uint16_t *getIndexData() const;
    size_t getIndexDataOffset() const;
    __uint32_t *getIndexBaseData() const;
    pack_header *getHeader() const;
    pack_bone *getBone(int index);


    void getIndexedRange(int index_group, int &start_index_offset, int &start_vertex_offset, int &index_count, int &vertex_count) const;

    void releaseData();

    void createDataBlocks(KRMeshManager::KRVBOData::vbo_type t);


};


#endif // KRMesh_I