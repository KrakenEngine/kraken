//
//  KRMesh.h
//  Kraken Engine
//
//  Copyright 2026 Kearwood Gilbert. All rights reserved.
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

#pragma once

#include "KREngine-common.h"

#include "KRContext.h"
#include "nodes/KRBone.h"
#include "KRMeshManager.h"
#include "resources/material/KRMaterialBinding.h"

#include "KREngine-common.h"

#include "hydra.h"

using namespace kraken;

#define MAX_VBO_SIZE 65535
#define KRENGINE_MAX_BONE_WEIGHTS_PER_VERTEX 4
#define KRENGINE_MAX_NAME_LENGTH 256
// MAX_VBO_SIZE must be divisible by 3 so triangles aren't split across VBO objects...

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#include "resources/material/KRMaterialManager.h"
#include "nodes/KRCamera.h"
#include "KRViewport.h"

class KRMaterial;
class KRNode;
class KRRenderPass;

class KRMesh : public KRResource
{

public:
  KRMesh(KRContext& context, std::string name, mimir::Block* data);
  KRMesh(KRContext& context, std::string name);
  virtual ~KRMesh();

  kraken_stream_level getStreamLevel();
  virtual void getResourceBindings(std::list<KRResourceBinding*>& bindings) override;
  void preStream();
  void requestResidency(uint32_t usage, float lodCoverage) final;

  bool hasTransparency();

  struct PrimitiveDesc
  {
    std::string materialName;
    Topology format;
    std::vector<hydra::Vector3> vertices;
    std::vector<int> indexes;
    std::vector<hydra::Vector2> texcoord[8];
    std::vector<hydra::Vector4> color[8];
    std::vector<hydra::Vector3> normals;
    std::vector<hydra::Vector3> tangents;
    std::vector<std::vector<float> > bone_weights;
    std::vector<std::vector<int> > bone_indexes;
  };

  struct MeshDesc
  {
    std::vector<PrimitiveDesc> primitives;
    std::vector<std::string> bone_names;
    std::vector<hydra::Matrix4> bone_bind_poses;
  };

  void render(KRNode::RenderInfo& ri, const std::string& object_name, const hydra::Matrix4& matModel, KRTexture* pLightMap, const std::vector<KRBone*>& bones, float lod_coverage = 0.0f);

  std::string m_lodBaseName;

  virtual std::string getExtension() override;
  virtual bool save(const std::string& path) override;
  virtual bool save(mimir::Block& data) override;

  void LoadDesc(const MeshDesc& mi, bool calculate_normals, bool calculate_tangents);
  void loadPack(mimir::Block* data);

  void convertToIndexed(std::bitset<64> primitives = ~std::bitset<64>(0));
  void optimizeIndexes(std::bitset<64> primitives = ~std::bitset<64>(0));

  void renderNoMaterials(VkCommandBuffer& commandBuffer, const KRRenderPass* renderPass, const std::string& object_name, const std::string& material_name, float lodCoverage);

  float getMaxDimension();

  const hydra::AABB& getExtents() const;

  int getLODCoverage() const;
  std::string getLODBaseName() const;


  static bool lod_sort_predicate(const KRMesh* m1, const KRMesh* m2);
  
  int getPrimitiveCount() const;
  int getVertexCount(int primitive) const;
  int getIndexCount(int primitive) const;
  const VertexBufferLayout* getLayout(int primitive) const;

  int getVertexIndex(int primitive, int index) const;
  void setVertexIndex(int primitive, int index, int indexVal);
  hydra::Vector3 getVertexPosition(int primitive, int index) const;
  hydra::Vector3 getVertexNormal(int primitive, int index) const;
  hydra::Vector3 getVertexTangent(int primitive, int index) const;
  hydra::Vector2 getVertexTexCoord(int primitive, int set, int index) const;
  hydra::Vector4 getVertexColor(int primitive, int set, int index) const;

  void setVertexAttribute(int primitive, int vertexIndex, int attributeIndex, float val);
  void setVertexAttribute(int primitive, int vertexIndex, int attributeIndex, hydra::Vector2 val);
  void setVertexAttribute(int primitive, int vertexIndex, int attributeIndex, hydra::Vector3 val);
  void setVertexAttribute(int primitive, int vertexIndex, int attributeIndex, hydra::Vector4 val);
  void setVertexAttribute(int primitive, int vertexIndex, int attributeIndex, hydra::Matrix2 val);
  void setVertexAttribute(int primitive, int vertexIndex, int attributeIndex, hydra::Matrix4 val);
  void getVertexAttribute(int primitive, int vertexIndex, int attributeIndex, float* val) const;
  void getVertexAttribute(int primitive, int vertexIndex, int attributeIndex, hydra::Vector2* val) const;
  void getVertexAttribute(int primitive, int vertexIndex, int attributeIndex, hydra::Vector3* val) const;
  void getVertexAttribute(int primitive, int vertexIndex, int attributeIndex, hydra::Vector4* val) const;
  void getVertexAttribute(int primitive, int vertexIndex, int attributeIndex, hydra::Matrix2* val) const;
  void getVertexAttribute(int primitive, int vertexIndex, int attributeIndex, hydra::Matrix4* val) const;

  void setVertexPosition(int primitive, int index, const hydra::Vector3& v);
  void setVertexNormal(int primitive, int index, const hydra::Vector3& v);
  void setVertexTangent(int primitive, int index, const hydra::Vector3& v);
  void setVertexTexCoord(int primitive, int index, int set, const hydra::Vector2& v);
  void setVertexColor(int primitive, int index, int set, const hydra::Vector4& v);

  int getBoneIndex(int primitive, int index, int weight_index) const;
  void setBoneIndex(int primitive, int index, int weight_index, int bone_index);

  float getBoneWeight(int primitive, int index, int weight_index) const;
  void setBoneWeight(int primitive, int index, int weight_index, float weight);

  static VkFormat AttributeVulkanFormat(const VertexAttributeInfo& attribute);

  int getBoneCount();
  char* getBoneName(int bone_index);
  hydra::Matrix4 getBoneBindPose(int bone_index);

  bool lineCast(const hydra::Vector3& v0, const hydra::Vector3& v1, hydra::HitInfo& hitinfo) const;
  bool rayCast(const hydra::Vector3& v0, const hydra::Vector3& dir, hydra::HitInfo& hitinfo) const;
  bool sphereCast(const hydra::Matrix4& model_to_world, const hydra::Vector3& v0, const hydra::Vector3& v1, float radius, hydra::HitInfo& hitinfo) const;

  static int GetLODCoverage(const std::string& name);

protected:
  bool m_constant; // TRUE if this should be always loaded and should not be passed through the streamer

private:
  mimir::Block* m_pData;

  // Sub-blocks
  mimir::Block* m_pMetaData;
  vector<mimir::Block*> m_vertexBlocks;
  vector<mimir::Block*> m_indexBlocks;

  // KRMeshManager depends on the address of KRVBOData's being constant
  // after allocation, enforced by deleted copy constructors.
  // As std::vector requires copy constuctors, we wrap these in shared_ptr.
  vector<shared_ptr<KRMeshManager::KRVBOData>> vbo_data_blocks;

  void initSubBlocks();
  void getPrimitives();
  void getMaterials();
  void renderPrimitive(VkCommandBuffer& commandBuffer, int primitive, const KRRenderPass* renderPass, const std::string& object_name, const std::string& material_name, float lodCoverage);

  static bool rayCast(const hydra::Vector3& start, const hydra::Vector3& dir, const hydra::Triangle3& tri, const hydra::Vector3& tri_n0, const hydra::Vector3& tri_n1, const hydra::Vector3& tri_n2, hydra::HitInfo& hitinfo);
  static bool sphereCast(const hydra::Matrix4& model_to_world, const hydra::Vector3& v0, const hydra::Vector3& v1, float radius, const hydra::Triangle3& tri, hydra::HitInfo& hitinfo);

  int m_lodCoverage; // This LOD level is activated when the bounding box of the model will cover less than this percent of the screen (100 = highest detail model)
  vector<KRMaterialBinding> m_materials;
  set<KRMaterial*> m_uniqueMaterials;

  bool m_hasTransparency;

  struct pack_header
  {
    char szTag[16];
    int32_t primitiveCount;
    int32_t boneCount;
    hydra::AABB extents; // Axis aligned bounding box, in model's coordinate space
    unsigned char reserved[80]; // Pad out to 128 bytes
  };

  static_assert(sizeof(pack_header) == 128);

  struct pack_bone
  {
    char szName[KRENGINE_MAX_NAME_LENGTH];
    float bind_pose[16];
  };

  struct pack_primitive
  {
    char szMaterialName[KRENGINE_MAX_NAME_LENGTH];
    VertexBufferLayout layout;
    int64_t vertexOffset;
    int64_t vertexCount;
    int64_t indexOffset;
    int64_t indexCount;
  };

  void setName(const std::string name);

  std::byte* getVertexData(int primitive, int index) const;
  pack_header* getHeader() const;
  pack_primitive* getPrimitive(int index) const;
  pack_bone* getBone(int index);
  int getAttributeIndex(int primitive, VertexAttribute attribute, int index) const;

  void releaseData(bool includeMainDatablock = true);
};
