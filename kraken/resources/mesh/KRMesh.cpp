//
//  KRMesh.cpp
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

#include <type_traits>
#include <algorithm>
#include <cmath>

#include "KREngine-common.h"

#include "KRMesh.h"

#include "KRPipeline.h"
#include "KRPipelineManager.h"
#include "KRContext.h"
#include "KRRenderPass.h"
#include "../3rdparty/forsyth/forsyth.h"

using namespace mimir;
using namespace hydra;


KRMesh::KRMesh(KRContext& context, std::string name) : KRResource(context, name)
{
  setName(name);

  m_hasTransparency = false;
  m_pData = NULL;
  m_pMetaData = NULL;
  m_constant = false;
}

KRMesh::KRMesh(KRContext& context, std::string name, Block* data) : KRResource(context, name)
{
  setName(name);

  m_hasTransparency = false;
  m_pData = NULL;
  m_pMetaData = NULL;
  m_constant = false;

  loadPack(data);
}

void KRMesh::setName(const std::string name)
{
  m_lodCoverage = 100;
  m_lodBaseName = name;
}

int KRMesh::GetLODCoverage(const std::string& name)
{
  int lod_coverage = 100;
  size_t last_underscore_pos = name.find_last_of('_');
  if (last_underscore_pos != std::string::npos) {
    // Found an underscore
    std::string suffix = name.substr(last_underscore_pos + 1);
    if (suffix.find("lod") == 0) {
      std::string lod_level_string = suffix.substr(3);
      char* end = NULL;
      int c = (int)strtol(lod_level_string.c_str(), &end, 10);
      if (c >= 0 && c <= 100 && *end == '\0') {
        lod_coverage = c;
        //m_lodBaseName = name.substr(0, last_underscore_pos);
      }
    }
  }
  return lod_coverage;
}



KRMesh::~KRMesh()
{
  releaseData();
}

void KRMesh::releaseData(bool includeMainDatablock /* = true*/)
{
  m_hasTransparency = false;
  
  vbo_data_blocks.clear();

  for (auto itr = m_vertexBlocks.begin(); itr != m_vertexBlocks.end(); itr++) {
    if (*itr != nullptr) {
      delete (*itr);
    }
  }

  for (auto itr = m_indexBlocks.begin(); itr != m_indexBlocks.end(); itr++) {
    if (*itr != nullptr) {
      delete (*itr);
    }
  }

  if (m_pMetaData) {
    m_pMetaData->unlock();
    delete m_pMetaData;
    m_pMetaData = NULL;
  }

  if (m_pData && includeMainDatablock) {
    delete m_pData;
    m_pData = NULL;
  }
}

std::string KRMesh::getExtension()
{
  return "krmesh";
}

bool KRMesh::save(const std::string& path)
{
  return m_pData->save(path);
}

bool KRMesh::save(Block& data)
{
  data.append(*m_pData);
  return true;
}

void KRMesh::loadPack(Block* data)
{
  releaseData();

  m_pData = data;

  initSubBlocks();
}

void KRMesh::getMaterials()
{
  if (m_materials.size() != 0) {
    return;
  }

  int primitiveCount = getHeader()->submesh_count;
  for (int i = 0; i < primitiveCount; i++) {
    const char* szMaterialName = getPrimitive(i)->szMaterialName;
    m_materials.push_back(KRMaterialBinding(szMaterialName));
  }
}

void KRMesh::requestResidency(uint32_t usage, float lodCoverage)
{
  KRResource::requestResidency(usage, lodCoverage);

  for (shared_ptr<KRMeshManager::KRVBOData>& vbo : vbo_data_blocks) {
    vbo->requestResidency(lodCoverage);
  }
}

void KRMesh::preStream()
{
  getPrimitives();
  getMaterials();

  m_hasTransparency = false;
  for(KRMaterialBinding& material : m_materials) {
    if (material.isBound() && material.get()->isTransparent()) {
      m_hasTransparency = true;
      break;
    }
  }
}

void KRMesh::getResourceBindings(std::list<KRResourceBinding*>& bindings)
{
  KRResource::getResourceBindings(bindings);

  for (KRResourceBinding& binding : m_materials) {
    bindings.push_back(&binding);
  }
}

kraken_stream_level KRMesh::getStreamLevel()
{
  kraken_stream_level stream_level = kraken_stream_level::STREAM_LEVEL_IN_HQ;
  getPrimitives();
  getMaterials();

  for (KRMaterialBinding& material : m_materials) {
    if (material.isBound()) {
      stream_level = std::min(stream_level, material.get()->getStreamLevel());
    }
  }
  bool all_vbo_data_loaded = true;
  bool vbo_data_loaded = false;
  for (const shared_ptr<KRMeshManager::KRVBOData>& vbo_data : vbo_data_blocks) {
    if (vbo_data->isVBOReady()) {
      vbo_data_loaded = true;
    } else {
      all_vbo_data_loaded = false;
    }
  }

  if (!vbo_data_loaded || !all_vbo_data_loaded) {
    stream_level = kraken_stream_level::STREAM_LEVEL_OUT;
  }

  return stream_level;
}


void KRMesh::render(KRNode::RenderInfo& ri, const std::string& object_name, const Matrix4& matModel, KRTexture* pLightMap, const std::vector<KRBone*>& bones, float lod_coverage)
{
  //fprintf(stderr, "Rendering model: %s\n", m_name.c_str());
  if (ri.renderPass->getType() != RenderPassType::RENDER_PASS_ADDITIVE_PARTICLES && ri.renderPass->getType() != RenderPassType::RENDER_PASS_PARTICLE_OCCLUSION && ri.renderPass->getType() != RenderPassType::RENDER_PASS_VOLUMETRIC_EFFECTS_ADDITIVE) {
    if (getStreamLevel() > kraken_stream_level::STREAM_LEVEL_OUT) {
      getPrimitives();
      getMaterials();

      int cSubmeshes = (int)getHeader()->submesh_count;
      if (ri.renderPass->getType() == RenderPassType::RENDER_PASS_SHADOWMAP) {
        for (int iSubmesh = 0; iSubmesh < cSubmeshes; iSubmesh++) {
          KRMaterial* pMaterial = m_materials[iSubmesh].get();
          if (pMaterial && !pMaterial->isTransparent()) {
            // Exclude transparent and semi-transparent meshes from shadow maps
            renderSubmesh(ri.commandBuffer, iSubmesh, ri.renderPass, object_name, pMaterial->getName(), lod_coverage);
          }
        }
      } else {
        for (int iSubmesh = 0; iSubmesh < cSubmeshes; iSubmesh++) {
          const pack_primitive* primitive = getPrimitive(iSubmesh);
          KRMaterial* pMaterial = m_materials[iSubmesh].get();

          if (pMaterial) {
            if ((!pMaterial->isTransparent() && ri.renderPass->getType() != RenderPassType::RENDER_PASS_FORWARD_TRANSPARENT) || (pMaterial->isTransparent() && ri.renderPass->getType() == RenderPassType::RENDER_PASS_FORWARD_TRANSPARENT)) {
              std::vector<Matrix4> bone_bind_poses;
              for (int i = 0; i < (int)bones.size(); i++) {
                bone_bind_poses.push_back(getBoneBindPose(i)); 
              }

              switch (pMaterial->getAlphaMode()) {
              case KRMaterial::KRMATERIAL_ALPHA_MODE_OPAQUE: // Non-transparent materials
              case KRMaterial::KRMATERIAL_ALPHA_MODE_TEST: // Alpha in diffuse texture is interpreted as punch-through when < 0.5
                if (pMaterial->bind(ri, &primitive->layout, CullMode::kCullBack, bones, bone_bind_poses, matModel, pLightMap, lod_coverage))
                {
                  renderSubmesh(ri.commandBuffer, iSubmesh, ri.renderPass, object_name, pMaterial->getName(), lod_coverage);
                }
                break;
              case KRMaterial::KRMATERIAL_ALPHA_MODE_BLEND: // Blended Alpha
                if (pMaterial->m_doubleSided) {
                  // Blended alpha rendered in two passes.  First pass renders backfaces; second pass renders frontfaces.
                  // 
                  // Render back faces before front faces
                  if (pMaterial->bind(ri, &primitive->layout, CullMode::kCullFront, bones, bone_bind_poses, matModel, pLightMap, lod_coverage))
                  {
                    renderSubmesh(ri.commandBuffer, iSubmesh, ri.renderPass, object_name, pMaterial->getName(), lod_coverage);
                  }
                }

                // Render front faces
                if (pMaterial->bind(ri, &primitive->layout, CullMode::kCullBack, bones, bone_bind_poses, matModel, pLightMap, lod_coverage))
                {
                  renderSubmesh(ri.commandBuffer, iSubmesh, ri.renderPass, object_name, pMaterial->getName(), lod_coverage);
                }
                break;
              }
            }
          }
        }
      }
    }
  }
}

float KRMesh::getMaxDimension()
{
  float m = 0.0;
  Vector3 size = getExtents().size();
  if (size.x > m) m = size.x;
  if (size.y > m) m = size.y;
  if (size.z > m) m = size.z;
  return m;
}

bool KRMesh::hasTransparency()
{
  return m_hasTransparency;
}


void KRMesh::initSubBlocks()
{
  assert(m_pMetaData == nullptr);
  assert(m_vertexBlocks.empty());
  assert(m_indexBlocks.empty());

  pack_header ph;
  m_pData->copy((void*)&ph, 0, sizeof(ph));
  m_pMetaData = m_pData->getSubBlock(0, sizeof(pack_header) + sizeof(pack_primitive) * ph.submesh_count + sizeof(pack_bone) * ph.bone_count);
  m_pMetaData->lock();

  pack_header* pHeader = getHeader();
  for (int i = 0; i < pHeader->submesh_count; i++) {
    pack_primitive* primitive = getPrimitive(i);

    Block* vertex_data_block = nullptr;
    if (primitive->vertexCount > 0) {
      vertex_data_block = m_pData->getSubBlock(primitive->vertexOffset, primitive->vertexCount * primitive->layout.vertexSize);
    }
    m_vertexBlocks.emplace_back(vertex_data_block);

    Block* index_data_block = nullptr;
    if (primitive->indexCount > 0) {
      index_data_block = m_pData->getSubBlock(primitive->indexOffset, primitive->indexCount * 2); // TODO: Support 16 and 32 bit index sizes
    }
    m_indexBlocks.emplace_back(index_data_block);
  }
}

void KRMesh::getPrimitives()
{
  if (vbo_data_blocks.size() == 0) {
    pack_header* pHeader = getHeader();
    KRMeshManager::KRVBOData::vbo_type t = m_constant ? KRMeshManager::KRVBOData::CONSTANT : KRMeshManager::KRVBOData::STREAMING;

    for (int i = 0; i < pHeader->submesh_count; i++) {
      pack_primitive* primitive = getPrimitive(i);

      vbo_data_blocks.emplace_back(std::make_shared<KRMeshManager::KRVBOData>(getContext().getMeshManager(), m_vertexBlocks[i], m_indexBlocks[i], &primitive->layout, true, t
#if KRENGINE_DEBUG_GPU_LABELS
        , m_lodBaseName.c_str()
#endif
      ));
    }
  }
}

void KRMesh::renderNoMaterials(VkCommandBuffer& commandBuffer, const KRRenderPass* renderPass, const std::string& object_name, const std::string& material_name, float lodCoverage)
{
  int submesh_count = getSubmeshCount();
  for (int i = 0; i < submesh_count; i++) {
    renderSubmesh(commandBuffer, i, renderPass, object_name, material_name, lodCoverage);
  }
}

void KRMesh::renderSubmesh(VkCommandBuffer& commandBuffer, int iSubmesh, const KRRenderPass* renderPass, const std::string& object_name, const std::string& material_name, float lodCoverage)
{
  getPrimitives();

  const pack_primitive* primitive = getPrimitive(iSubmesh);
  if (primitive->vertexCount == 0) {
    return;
  }

  KRMeshManager::KRVBOData& vbo_data_block = *vbo_data_blocks[iSubmesh];
  m_pContext->getMeshManager()->bindVBO(commandBuffer, &vbo_data_block, lodCoverage);
  assert(vbo_data_block.isVBOReady());

  int vbo_index = 0;
  if (primitive->indexCount > 0) {
    
    vkCmdDrawIndexed(commandBuffer, primitive->indexCount, 1, 0, 0, 0);
    m_pContext->getMeshManager()->log_draw_call(renderPass->getType(), object_name, material_name, primitive->indexCount);
  } else {
    vkCmdDraw(commandBuffer, primitive->vertexCount, 1, 0, 0);
    m_pContext->getMeshManager()->log_draw_call(renderPass->getType(), object_name, material_name, primitive->vertexCount);
  }
}

void KRMesh::LoadDesc(const KRMesh::MeshDesc& mi, bool calculate_normals, bool calculate_tangents)
{ 
  // Create a new header
  pack_header header = {};
  strcpy(header.szTag, "KRMESH1.0      ");
  header.submesh_count = mi.primitives.size();
  header.bone_count = mi.bone_names.size();
  header.extents.min = Vector3::Max();
  header.extents.max = Vector3::Min();

  // Get the vertex extents
  for (const PrimitiveDesc& pd : mi.primitives) {
    for (const Vector3& v : pd.vertices) {
      header.extents.encapsulate(v);
    }
  }

  // Collect the Bones 
  std::vector<pack_bone> bones;
  bones.reserve(mi.bone_names.size());
  for (int i = 0; i < mi.bone_names.size(); i++) {
    pack_bone bone;
    memset(bone.szName, 0, KRENGINE_MAX_NAME_LENGTH);
    strncpy(bone.szName, mi.bone_names[i].c_str(), KRENGINE_MAX_NAME_LENGTH);
    memcpy(bone.bind_pose, mi.bone_bind_poses[i].c, sizeof(float) * 16);
    bones.emplace_back(bone);
  }
  
  // Collect the primitives
  size_t dataOffset = sizeof(pack_header) + mi.primitives.size() * sizeof(pack_primitive) + bones.size() * sizeof(pack_bone);
  std::vector<pack_primitive> primitives;
  primitives.reserve(mi.primitives.size());
  for (const PrimitiveDesc& p : mi.primitives) {
    pack_primitive primitive = {};
    VertexAttributeInfo* attribute = primitive.layout.attributes;
    strncpy(primitive.szMaterialName, p.materialName.c_str(), KRENGINE_MAX_NAME_LENGTH);

    if (p.vertices.size()) {
      attribute->attribute = VertexAttribute::position;
      attribute->type = DataType::vec3;
      attribute->normalization = Normalization::none;
      attribute->component = ComponentType::float32;
      attribute++;
    }
    if (p.normals.size() || calculate_normals) {
      attribute->attribute = VertexAttribute::normal;
      attribute->type = DataType::vec3;
      attribute->normalization = Normalization::none;
      attribute->component = ComponentType::float16;
      attribute++;
    }
    if (p.tangents.size() || calculate_tangents) {
      attribute->attribute = VertexAttribute::tangent;
      attribute->type = DataType::vec3;
      attribute->normalization = Normalization::none;
      attribute->component = ComponentType::float16;
      attribute++;
    }
    for (int set = 0; set < 8; set++) {
      if (p.texcoord[set].size()) {
        attribute->attribute = VertexAttribute::texcoord;
        attribute->type = DataType::vec2;
        attribute->normalization = Normalization::none;
        attribute->component = ComponentType::float32;
        attribute++;
      }

      if (p.color[set].size()) {
        attribute->attribute = VertexAttribute::color;
        attribute->type = DataType::vec4;
        attribute->component = ComponentType::uint8;
        attribute->normalization = Normalization::normalized;
        attribute++;
      }
    } // for each set

    if (p.bone_weights.size()) {
      attribute->attribute = VertexAttribute::joints;
      attribute->type = DataType::vec4;
      attribute->component = ComponentType::uint8;
      attribute->normalization = Normalization::none;
      attribute++;

      attribute->attribute = VertexAttribute::weights;
      attribute->type = DataType::vec4;
      attribute->component = ComponentType::float32;
      attribute->normalization = Normalization::none;
      attribute++;
    }

    for (int i = 0; i < kMaxAttributes && primitive.layout.attributes[i].component != ComponentType::empty; i++) {
      primitive.layout.offsets[i] = primitive.layout.vertexSize;
      int componentSize = ComponentSize[(int)primitive.layout.attributes[i].component] * DataTypeComponentCount[(int)primitive.layout.attributes[i].type];
      primitive.layout.vertexSize += componentSize;
    }

    primitive.layout.topology = p.format;
    primitive.vertexCount = p.vertices.size();
    primitive.indexCount = p.indexes.size();

    const int kBufferAlignment = 64;
    const int kIndexSize = 2; // TODO: implement mixed 16 and 32 bit indexes

    dataOffset = (dataOffset + kBufferAlignment - 1) / kBufferAlignment * kBufferAlignment;

    primitive.indexOffset = dataOffset;
    dataOffset += primitive.indexCount * kIndexSize;

    dataOffset = (dataOffset + kBufferAlignment - 1) / kBufferAlignment * kBufferAlignment;
    primitive.vertexOffset = dataOffset;

    primitives.push_back(primitive);

    dataOffset += primitive.vertexCount * primitive.layout.vertexSize;

  } // for each PrimitiveDesc



  // Clear old data and start again
  releaseData();
  m_pData = new Block();
  m_pData->expand(dataOffset);
  m_pData->lock();
  m_pData->fill(0);

  // Append the Header
  std::byte* dest = (std::byte*)m_pData->getStart();
  memcpy(dest, &header, sizeof(pack_header));
  dest += sizeof(pack_header);

  // Append the primitives
  memcpy(dest, primitives.data(), primitives.size() * sizeof(pack_primitive));
  dest += primitives.size() * sizeof(pack_primitive);

  // Append the Bones
  memcpy(dest, bones.data(), bones.size() * sizeof(pack_bone));
  dest += bones.size() * sizeof(pack_bone);

  initSubBlocks();

  for (int pi = 0; pi < mi.primitives.size(); pi++) {
    if (m_vertexBlocks[pi]) {
      m_vertexBlocks[pi]->lock();
    }
    if (m_indexBlocks[pi]) {
      m_indexBlocks[pi]->lock();
    }
    const PrimitiveDesc& pd = mi.primitives[pi];
    const pack_primitive* primitive = getPrimitive(pi);

    int vertex_size = (int)primitive->layout.vertexSize;
    for (int iVertex = 0; iVertex < (int)pd.vertices.size(); iVertex++) {
      Vector3 source_vertex = pd.vertices[iVertex];
      setVertexPosition(pi, iVertex, source_vertex);
      if (mi.bone_names.size()) {
        hydra::Vector4 weights = hydra::Vector4::Zero();
        for (int bone_weight_index = 0; bone_weight_index < KRENGINE_MAX_BONE_WEIGHTS_PER_VERTEX; bone_weight_index++) {
          setBoneIndex(pi, iVertex, bone_weight_index, pd.bone_indexes[iVertex][bone_weight_index]);
          setBoneWeight(pi, iVertex, bone_weight_index, pd.bone_weights[iVertex][bone_weight_index]);
        }
      }

      for (int set = 0; set < 8; set++) {
        if ((int)pd.texcoord[set].size() > iVertex) {
          setVertexTexCoord(pi, iVertex, set, pd.texcoord[set][iVertex]);
        }

        if ((int)pd.color[set].size() > iVertex) {
          setVertexColor(pi, iVertex, set, pd.color[set][iVertex]);
        }
      }
      if ((int)pd.normals.size() > iVertex) {
        setVertexNormal(pi, iVertex, Vector3::Normalize(pd.normals[iVertex]));
      }
      if ((int)pd.tangents.size() > iVertex) {
        setVertexTangent(pi, iVertex, Vector3::Normalize(pd.tangents[iVertex]));
      }
    }
    for (int iIndex = 0; iIndex < (int)pd.indexes.size(); iIndex++) {
      setVertexIndex(pi, iIndex, pd.indexes[iIndex]);
    }

    auto calculateTriangleAttributes = [this, pi, calculate_normals, calculate_tangents](int i0, int i1, int i2) {
      Vector3 p1 = getVertexPosition(pi, i0);
      Vector3 p2 = getVertexPosition(pi, i1);
      Vector3 p3 = getVertexPosition(pi, i2);
      Vector3 v1 = p2 - p1;
      Vector3 v2 = p3 - p1;

      // -- Calculate normal if missing --
      if (calculate_normals) {
        Vector3 first_normal = getVertexNormal(pi, i0);
        if (first_normal.x == 0.0f && first_normal.y == 0.0f && first_normal.z == 0.0f) {
          // Note - We don't take into consideration smoothing groups or smoothing angles when generating normals; all generated normals represent flat shaded polygons
          Vector3 normal = Vector3::Cross(v1, v2);

          normal.normalize();
          setVertexNormal(pi, i0, normal);
          setVertexNormal(pi, i1, normal);
          setVertexNormal(pi, i2, normal);
        }
      }

      // -- Calculate tangent vector for normal mapping --
      if (calculate_tangents) {
        Vector3 first_tangent = getVertexTangent(pi, i0);
        if (first_tangent.x == 0.0f && first_tangent.y == 0.0f && first_tangent.z == 0.0f) {

          Vector2 uv0 = getVertexTexCoord(pi, 0, i0);
          Vector2 uv1 = getVertexTexCoord(pi, 1, i1);
          Vector2 uv2 = getVertexTexCoord(pi, 2, i2);

          Vector2 st1 = Vector2::Create(uv1.x - uv0.x, uv1.y - uv0.y);
          Vector2 st2 = Vector2::Create(uv2.x - uv0.x, uv2.y - uv0.y);
          float coef = 1 / (st1.x * st2.y - st2.x * st1.y);

          Vector3 tangent = Vector3::Create(
                            coef * ((v1.x * st2.y) + (v2.x * -st1.y)),
                            coef * ((v1.y * st2.y) + (v2.y * -st1.y)),
                            coef * ((v1.z * st2.y) + (v2.z * -st1.y))
          );

          tangent.normalize();
          setVertexTangent(pi, i0, tangent);
          setVertexTangent(pi, i1, tangent);
          setVertexTangent(pi, i2, tangent);
        }
      }
    };

    // Calculate missing surface normals and tangents
    if (calculate_normals || calculate_tangents) {
      switch (pd.format) {
      case Topology::Triangles:
      {
        // NOTE: This will not work properly if the vertices are already indexed
        for (int iVertex = 0; iVertex + 2 < (int)pd.vertices.size(); iVertex += 3) {
          calculateTriangleAttributes(iVertex, iVertex + 1, iVertex + 2);
        }
        break;
      }
      case Topology::TriangleStrips:
      {
        // NOTE: This will not work properly if the vertices are already indexed
        for (int iVertex = 0; iVertex + 2 < (int)pd.vertices.size(); iVertex++) {
          calculateTriangleAttributes(iVertex, iVertex + 1, iVertex + 2);
        }
        break;
      }
      case Topology::TriangleFans:
      {
        // NOTE: This will not work properly if the vertices are already indexed
        for (int iVertex = 1; iVertex + 1 < (int)pd.vertices.size(); iVertex++) {
          calculateTriangleAttributes(0, iVertex, iVertex + 1);
        }
        break;
      }
      default:
        assert(false); // Not Supported
      } // switch
    }
    if (m_vertexBlocks[pi]) {
      m_vertexBlocks[pi]->unlock();
    }
    if (m_indexBlocks[pi]) {
      m_indexBlocks[pi]->unlock();
    }
  } // for each primitive
  m_pData->unlock();


  optimize();

  if (m_constant) {
    // Ensure that constant models loaded immediately by the streamer
    getPrimitives();
    getMaterials();
  }
}

const AABB& KRMesh::getExtents() const
{
  return getHeader()->extents;
}

int KRMesh::getLODCoverage() const
{
  return m_lodCoverage;
}

std::string KRMesh::getLODBaseName() const
{
  return m_lodBaseName;
}

// Predicate used with std::sort to sort by highest detail model first, decending to lowest detail LOD model
bool KRMesh::lod_sort_predicate(const KRMesh* m1, const KRMesh* m2)
{
  return m1->m_lodCoverage > m2->m_lodCoverage;
}

int KRMesh::getAttributeIndex(int submesh, VertexAttribute attribute, int index) const
{
  const pack_primitive& primitive = *getPrimitive(submesh);
  int indexLeft = index;
  for (int i = 0; i < kMaxAttributes; i++) {
    VertexAttributeInfo info = primitive.layout.attributes[i];
    if (info.component == ComponentType::empty) {
      break;
    }
    if (info.attribute == attribute) {
      if (indexLeft == 0) {
        return i;
      }
      indexLeft--;
    }
  }
  return -1;
}


KRMesh::pack_header* KRMesh::getHeader() const
{
  return (pack_header*)m_pMetaData->getStart();
}

KRMesh::pack_primitive* KRMesh::getPrimitive(int index) const
{
  assert(index < getHeader()->submesh_count);
  pack_primitive* primitives = (pack_primitive*)((std::byte*)m_pMetaData->getStart() + sizeof(pack_header));
  return primitives + index;
}

KRMesh::pack_bone* KRMesh::getBone(int index)
{
  pack_header* header = getHeader();
  return (pack_bone*)((unsigned char*)m_pMetaData->getStart() + sizeof(pack_header) + sizeof(pack_primitive) * header->submesh_count + sizeof(pack_bone) * index);
}

std::byte* KRMesh::getVertexData(int submesh, int index) const
{
  return (std::byte*)m_vertexBlocks[submesh]->getStart() + getPrimitive(submesh)->layout.vertexSize * index;
}

int KRMesh::getSubmeshCount() const
{
  pack_header* header = getHeader();
  int submesh_count = header->submesh_count;
  return submesh_count;
}

int KRMesh::getVertexCount(int submesh) const
{
  return getPrimitive(submesh)->vertexCount;
}

int KRMesh::getIndexCount(int submesh) const
{
  return getPrimitive(submesh)->indexCount;
}

const VertexBufferLayout* KRMesh::getLayout(int submesh) const
{
  return &getPrimitive(submesh)->layout;
}

template<typename T>
constexpr void denormalizeAttributeComponent(const Normalization norm, T val, float* out)
{
  static_assert(std::is_scalar_v<T>, "Input type must be a scalar.");
  assert(norm != Normalization::normalized || std::is_integral_v<T>); // Can only denormalize integral types.
  assert(norm != Normalization::srgb || (std::is_integral_v<T> && !std::is_signed_v<T>)); // Can only reverse SRGB normalization for unsigned integral types.

  switch (norm) {
  case Normalization::none:
  case Normalization::scaled:
    *out = static_cast<float>(val);
    break;
  case Normalization::normalized:
    *out = static_cast<float>(val) / static_cast<float>(std::numeric_limits<T>::max());
    break;
  case Normalization::srgb:
    {
      // See https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html
      // sRGB EOTF-1

      constexpr float split = 0.0031308f * 12.92f; // 0.040449936

      float linearDenorm = static_cast<float>(val) / static_cast<float>(std::numeric_limits<T>::max());
      if (linearDenorm <= split) {
        *out = val / 12.92f;
      } else {
        *out = std::pow((linearDenorm + 0.055f) / 1.055f, 2.4f);
      }
    }
    break;
  }
}

template<typename T>
constexpr void normalizeAttributeComponent(const Normalization norm, float val, T* out)
{
  static_assert(std::is_scalar_v<T>, "Return type must be a scalar.");
  assert(norm != Normalization::normalized || std::is_integral_v<T>); // Can only denormalize integral types.
  assert(norm != Normalization::srgb || (std::is_integral_v<T> && !std::is_signed_v<T>)); // Can only reverse SRGB normalization for unsigned integral types.

  constexpr float minFloat = static_cast<float>(std::numeric_limits<T>::min());
  constexpr float maxFloat = static_cast<float>(std::numeric_limits<T>::max());
  

  switch (norm)     {
  case Normalization::none:
  case Normalization::scaled:
    {
      if constexpr (std::is_same_v<T, float>) {
        *out = val;
      } else {
        float clampedFloat = std::clamp(val, minFloat, maxFloat);
        *out = static_cast<T>(clampedFloat);
      }
    }
    break;
  case Normalization::normalized:
    if constexpr (std::is_signed_v<T>) {
      *out = static_cast<T>(std::lround(std::clamp(val, -1.f, 1.f) * maxFloat));
    } else {
      *out = static_cast<T>(std::lround(std::clamp(val, 0.f, 1.f) * maxFloat));
    }
    break;
  case Normalization::srgb:
    {
      // See https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html
      // sRGB EOTF-1

      float clamped = std::clamp(val, 0.f, 1.f);
      float srgb = 0.f;
      if (clamped <= 0.0031308f) {
        srgb = clamped * 12.92f;
      } else {
        srgb = 1.055f * std::pow(clamped, 1.0f / 2.4f) - 0.055f;
      }
      *out = static_cast<T>(std::lround(srgb * maxFloat));
    }
    break;
  }
}

#if defined(KRAKEN_ARCH_X86_64)
  uint16_t floatToHalf(float val)
  {
    __m128 simdFloat = _mm_set_ss(val);
    __m128i simdHalf = _mm_cvtps_ph(simdFloat, _MM_FROUND_TO_NEAREST_INT);
    return _mm_cvtsi128_si32(simdHalf) & 0xFFFF;
  }

  float halfToFloat(uint16_t val)
  {
    __m128i simdHalf = _mm_cvtsi32_si128(val);
    __m128 simdFloat = _mm_cvtph_ps(simdHalf);
    return _mm_cvtss_f32(simdFloat);
  }

#elif defined(KRAKEN_ARCH_ARM64)
  uint16_t floatToHalf(float val)
  {
    __fp16 h = static_cast<__fp16>(val);
    uint16_t bits;
    std::memcpy(&bits, &h, 2);
    return bits;
  }

  // 2. Half to Float
  float halfToFloat(uint16_t val)
  {
    __fp16 h;
    std::memcpy(&h, &val, 2);
    return static_cast<float>(h);
  }

#else

  uint16_t floatToHalf(float val)
  {
    uint32_t f32Bits;
    std::memcpy(&f32Bits, &val, sizeof(float));

    uint32_t sign = (f32Bits >> 16) & 0x8000;
    int32_t exponent = ((f32Bits >> 23) & 0xFF) - 127;
    uint32_t mantissa = f32Bits & 0x007FFFFF;

    if (exponent <= -15) {
      if (exponent < -24) return sign;
      mantissa |= 0x00800000;
      return sign | (mantissa >> (-14 - exponent));
    }
    if (exponent > 15) return sign | 0x7C00;

    return sign | ((exponent + 15) << 10) | (mantissa >> 13);
  }

  float halfToFloat(uint16_t val)
  {
    uint32_t sign = (val & 0x8000) << 16;
    uint32_t exponent = (val & 0x7C00) >> 10;
    uint32_t mantissa = val & 0x03FF;
    uint32_t f32Bits = 0;

    if (exponent == 0) {
      if (mantissa != 0) {
        while ((mantissa & 0x0400) == 0) {
          mantissa <<= 1;
          exponent--;
        }
        exponent++;
        mantissa &= 0x03FF;
        f32Bits = sign | ((exponent + 112) << 23) | (mantissa << 13);
      } else {
        f32Bits = sign;
      }
    } else if (exponent == 31) {
      f32Bits = sign | 0x7F800000 | (mantissa << 13);
    } else {
      f32Bits = sign | ((exponent + 112) << 23) | (mantissa << 13);
    }

    float result;
    std::memcpy(&result, &f32Bits, sizeof(float));
    return result;
  }

#endif

// IEEE 754 Half precision specialization
constexpr void normalizeAttributeComponent_float16(const Normalization norm, float val, uint16_t* out)
{
  assert(norm == Normalization::none);

  constexpr float minFloat = -65504.0f;
  constexpr float maxFloat = 65504.0f;

  float clampedFloat = std::clamp(val, minFloat, maxFloat);
  *out = floatToHalf(clampedFloat);
}

// IEEE 754 Half precision specialization
constexpr void denormalizeAttributeComponent_float16(const Normalization norm, uint16_t val, float* out)
{
  assert(norm == Normalization::none);

  *out = halfToFloat(val);
}

void writeVertexAttributeComponent(const VertexAttributeInfo& attribute, void* address, int componentIndex, float val)
{
  void* componentAddress = (uint8_t*)address + ComponentSize[(int)attribute.component] * componentIndex;
  switch (attribute.component) {
  case ComponentType::empty:
    break;
  case ComponentType::int8:
    normalizeAttributeComponent(attribute.normalization, val, (__int8_t*)componentAddress);
    break;
  case ComponentType::uint8:
    normalizeAttributeComponent(attribute.normalization, val, (__uint8_t*)componentAddress);
    break;
  case ComponentType::int16:
    normalizeAttributeComponent(attribute.normalization, val, (__int16_t*)componentAddress);
    break;
  case ComponentType::uint16:
    normalizeAttributeComponent(attribute.normalization, val, (__uint16_t*)componentAddress);
    break;
  case ComponentType::int32:
    normalizeAttributeComponent(attribute.normalization, val, (__int32_t*)componentAddress);
    break;
  case ComponentType::uint32:
    normalizeAttributeComponent(attribute.normalization, val, (__uint32_t*)componentAddress);
    break;
  case ComponentType::int64:
    normalizeAttributeComponent(attribute.normalization, val, (__int64_t*)componentAddress);
    break;
  case ComponentType::uint64:
    normalizeAttributeComponent(attribute.normalization, val, (__uint64_t*)componentAddress);
    break;
  case ComponentType::float16:
    normalizeAttributeComponent_float16(attribute.normalization, val, (__uint16_t*)componentAddress);
    break;
  case ComponentType::float32:
    normalizeAttributeComponent(attribute.normalization, val, (float*)componentAddress);
    break;
  case ComponentType::float64:
    normalizeAttributeComponent(attribute.normalization, val, (double*)componentAddress);
    break;
  }
}

void readVertexAttributeComponent(const VertexAttributeInfo& attribute, const void* address, int componentIndex, float* out)
{
  void* componentAddress = (uint8_t*)address + ComponentSize[(int)attribute.component] * componentIndex;
  switch (attribute.component) {
  case ComponentType::empty:
    break;
  case ComponentType::int8:
    denormalizeAttributeComponent(attribute.normalization, *(__int8_t*)componentAddress, out);
    break;
  case ComponentType::uint8:
    denormalizeAttributeComponent(attribute.normalization, *(__uint8_t*)componentAddress, out);
    break;
  case ComponentType::int16:
    denormalizeAttributeComponent(attribute.normalization, *(__int16_t*)componentAddress, out);
    break;
  case ComponentType::uint16:
    denormalizeAttributeComponent(attribute.normalization, *(__uint16_t*)componentAddress, out);
    break;
  case ComponentType::int32:
    denormalizeAttributeComponent(attribute.normalization, *(__int32_t*)componentAddress, out);
    break;
  case ComponentType::uint32:
    denormalizeAttributeComponent(attribute.normalization, *(__uint32_t*)componentAddress, out);
    break;
  case ComponentType::int64:
    denormalizeAttributeComponent(attribute.normalization, *(__int64_t*)componentAddress, out);
    break;
  case ComponentType::uint64:
    denormalizeAttributeComponent(attribute.normalization, *(__uint64_t*)componentAddress, out);
    break;
  case ComponentType::float16:
    denormalizeAttributeComponent_float16(attribute.normalization, *(__uint16_t*)componentAddress, out);
    break;
  case ComponentType::float32:
    denormalizeAttributeComponent(attribute.normalization, *(float*)componentAddress, out);
    break;
  case ComponentType::float64:
    denormalizeAttributeComponent(attribute.normalization, *(double*)componentAddress, out);
    break;
  }
}

void KRMesh::setVertexAttribute(int submesh, int vertexIndex, int attribIndex, float val)
{
  const pack_primitive& primitive = *getPrimitive(submesh);
  const VertexAttributeInfo& attribute = primitive.layout.attributes[attribIndex];
  void* address = getVertexData(submesh, vertexIndex) + primitive.layout.offsets[attribIndex];

  if (attribute.type != DataType::scalar)
  {
    assert(false);
    return;
  }

  writeVertexAttributeComponent(attribute, address, 0, val);
}

void KRMesh::getVertexAttribute(int submesh, int vertexIndex, int attribIndex, float* val) const
{
  const pack_primitive& primitive = *getPrimitive(submesh);
  const VertexAttributeInfo& attribute = primitive.layout.attributes[attribIndex];
  void* address = getVertexData(submesh, vertexIndex) + primitive.layout.offsets[attribIndex];

  if (attribute.type != DataType::scalar) {
    assert(false);
    return;
  }

  readVertexAttributeComponent(attribute, address, 0, val);
}

void KRMesh::setVertexAttribute(int submesh, int vertexIndex, int attribIndex, Vector2 val)
{
  const pack_primitive& primitive = *getPrimitive(submesh);
  const VertexAttributeInfo& attribute = primitive.layout.attributes[attribIndex];
  void* address = getVertexData(submesh, vertexIndex) + primitive.layout.offsets[attribIndex];

  if (attribute.type != DataType::vec2) {
    assert(false);
    return;
  }

  writeVertexAttributeComponent(attribute, address, 0, val.x);
  writeVertexAttributeComponent(attribute, address, 1, val.y);
}

void KRMesh::getVertexAttribute(int submesh, int vertexIndex, int attribIndex, Vector2* val) const
{
  const pack_primitive& primitive = *getPrimitive(submesh);
  const VertexAttributeInfo& attribute = primitive.layout.attributes[attribIndex];
  void* address = getVertexData(submesh, vertexIndex) + primitive.layout.offsets[attribIndex];

  if (attribute.type != DataType::vec2) {
    assert(false);
    return;
  }

  readVertexAttributeComponent(attribute, address, 0, &val->x);
  readVertexAttributeComponent(attribute, address, 1, &val->y);
}

void KRMesh::setVertexAttribute(int submesh, int vertexIndex, int attribIndex, Vector3 val)
{
  const pack_primitive& primitive = *getPrimitive(submesh);
  const VertexAttributeInfo& attribute = primitive.layout.attributes[attribIndex];
  void* address = getVertexData(submesh, vertexIndex) + primitive.layout.offsets[attribIndex];

  if (attribute.type != DataType::vec3) {
    assert(false);
    return;
  }

  writeVertexAttributeComponent(attribute, address, 0, val.x);
  writeVertexAttributeComponent(attribute, address, 1, val.y);
  writeVertexAttributeComponent(attribute, address, 2, val.z);
}

void KRMesh::getVertexAttribute(int submesh, int vertexIndex, int attribIndex, Vector3* val) const
{
  const pack_primitive& primitive = *getPrimitive(submesh);
  const VertexAttributeInfo& attribute = primitive.layout.attributes[attribIndex];
  void* address = getVertexData(submesh, vertexIndex) + primitive.layout.offsets[attribIndex];

  if (attribute.type != DataType::vec3) {
    assert(false);
    return;
  }

  readVertexAttributeComponent(attribute, address, 0, &val->x);
  readVertexAttributeComponent(attribute, address, 1, &val->y);
  readVertexAttributeComponent(attribute, address, 2, &val->z);
}

void KRMesh::setVertexAttribute(int submesh, int vertexIndex, int attribIndex, Vector4 val)
{
  const pack_primitive& primitive = *getPrimitive(submesh);
  const VertexAttributeInfo& attribute = primitive.layout.attributes[attribIndex];
  void* address = getVertexData(submesh, vertexIndex) + primitive.layout.offsets[attribIndex];

  if (attribute.type != DataType::vec4) {
    assert(false);
    return;
  }

  writeVertexAttributeComponent(attribute, address, 0, val.x);
  writeVertexAttributeComponent(attribute, address, 1, val.y);
  writeVertexAttributeComponent(attribute, address, 2, val.z);
  writeVertexAttributeComponent(attribute, address, 3, val.w);
}

void KRMesh::getVertexAttribute(int submesh, int vertexIndex, int attribIndex, Vector4* val) const
{
  const pack_primitive& primitive = *getPrimitive(submesh);
  const VertexAttributeInfo& attribute = primitive.layout.attributes[attribIndex];
  void* address = getVertexData(submesh, vertexIndex) + primitive.layout.offsets[attribIndex];

  if (attribute.type != DataType::vec4) {
    assert(false);
    return;
  }

  readVertexAttributeComponent(attribute, address, 0, &val->x);
  readVertexAttributeComponent(attribute, address, 1, &val->y);
  readVertexAttributeComponent(attribute, address, 2, &val->z);
  readVertexAttributeComponent(attribute, address, 3, &val->w);
}

void KRMesh::setVertexAttribute(int submesh, int vertexIndex, int attribIndex, Matrix2 val)
{
  const pack_primitive& primitive = *getPrimitive(submesh);
  const VertexAttributeInfo& attribute = primitive.layout.attributes[attribIndex];
  void* address = getVertexData(submesh, vertexIndex) + primitive.layout.offsets[attribIndex];

  if (attribute.type != DataType::mat2) {
    assert(false);
    return;
  }

  writeVertexAttributeComponent(attribute, address, 0, val.c[0]);
  writeVertexAttributeComponent(attribute, address, 1, val.c[1]);
  writeVertexAttributeComponent(attribute, address, 2, val.c[2]);
  writeVertexAttributeComponent(attribute, address, 3, val.c[3]);
}

void KRMesh::getVertexAttribute(int submesh, int vertexIndex, int attribIndex, Matrix2* val) const
{
  const pack_primitive& primitive = *getPrimitive(submesh);
  const VertexAttributeInfo& attribute = primitive.layout.attributes[attribIndex];
  void* address = getVertexData(submesh, vertexIndex) + primitive.layout.offsets[attribIndex];

  if (attribute.type != DataType::mat2) {
    assert(false);
    return;
  }

  readVertexAttributeComponent(attribute, address, 0, &val->c[0]);
  readVertexAttributeComponent(attribute, address, 1, &val->c[1]);
  readVertexAttributeComponent(attribute, address, 2, &val->c[2]);
  readVertexAttributeComponent(attribute, address, 3, &val->c[3]);
}

void KRMesh::setVertexAttribute(int submesh, int vertexIndex, int attribIndex, Matrix4 val)
{
  const pack_primitive& primitive = *getPrimitive(submesh);
  const VertexAttributeInfo& attribute = primitive.layout.attributes[attribIndex];
  void* address = getVertexData(submesh, vertexIndex) + primitive.layout.offsets[attribIndex];

  if (attribute.type != DataType::mat4) {
    assert(false);
    return;
  }

  for (int i = 0; i < 16; i++) {
    writeVertexAttributeComponent(attribute, address, i, val.c[i]);
  }
}

void KRMesh::getVertexAttribute(int submesh, int vertexIndex, int attribIndex, Matrix4* val) const
{
  const pack_primitive& primitive = *getPrimitive(submesh);
  const VertexAttributeInfo& attribute = primitive.layout.attributes[attribIndex];
  void* address = getVertexData(submesh, vertexIndex) + primitive.layout.offsets[attribIndex];

  if (attribute.type != DataType::mat4) {
    assert(false);
    return;
  }

  for (int i = 0; i < 16; i++) {
    readVertexAttributeComponent(attribute, address, i, &val->c[i]);
  }
}

Vector3 KRMesh::getVertexPosition(int submesh, int index) const
{
  int attribIndex = getAttributeIndex(submesh, VertexAttribute::position, 0);
  if (attribIndex == -1) {
    return Vector3::Zero();
  }
  Vector3 v;
  getVertexAttribute(submesh, index, attribIndex, &v);
  return v;
}

Vector3 KRMesh::getVertexNormal(int submesh, int index) const
{
  int attribIndex = getAttributeIndex(submesh, VertexAttribute::normal, 0);
  if (attribIndex == -1) {
    return Vector3::Zero();
  }
  Vector3 v;
  getVertexAttribute(submesh, index, attribIndex, &v);
  return v;
}

Vector3 KRMesh::getVertexTangent(int submesh, int index) const
{
  int attribIndex = getAttributeIndex(submesh, VertexAttribute::tangent, 0);
  if (attribIndex == -1) {
    return Vector3::Zero();
  }
  Vector3 v;
  getVertexAttribute(submesh, index, attribIndex, &v);
  return v;
}

Vector2 KRMesh::getVertexTexCoord(int submesh, int set, int index) const
{
  int attribIndex = getAttributeIndex(submesh, VertexAttribute::texcoord, set);
  if (attribIndex == -1) {
    return Vector2::Zero();
  }
  Vector2 v;
  getVertexAttribute(submesh, index, attribIndex, &v);
  return v;
}

Vector4 KRMesh::getVertexColor(int submesh, int set, int index) const
{
  int attribIndex = getAttributeIndex(submesh, VertexAttribute::color, set);
  if (attribIndex == -1) {
    return Vector4::Zero();
  }
  Vector4 v;
  getVertexAttribute(submesh, index, attribIndex, &v);
  return v;
}

void KRMesh::setVertexPosition(int submesh, int index, const Vector3& v)
{
  int attribIndex = getAttributeIndex(submesh, VertexAttribute::position, 0);
  if (attribIndex == -1) {
    return;
  }

  setVertexAttribute(submesh, index, attribIndex, v);
}

void KRMesh::setVertexNormal(int submesh, int index, const Vector3& v)
{
  int attribIndex = getAttributeIndex(submesh, VertexAttribute::normal, 0);
  if (attribIndex == -1) {
    return;
  }

  setVertexAttribute(submesh, index, attribIndex, v);
}

void KRMesh::setVertexTangent(int submesh, int index, const Vector3& v)
{
  int attribIndex = getAttributeIndex(submesh, VertexAttribute::tangent, 0);
  if (attribIndex == -1) {
    return;
  }

  setVertexAttribute(submesh, index, attribIndex, v);
}

void KRMesh::setVertexTexCoord(int submesh, int index, int set, const Vector2& v)
{
  int attribIndex = getAttributeIndex(submesh, VertexAttribute::texcoord, set);
  if (attribIndex == -1) {
    return;
  }

  setVertexAttribute(submesh, index, attribIndex, v);
}

void KRMesh::setVertexColor(int submesh, int index, int set, const Vector4& v)
{
  int attribIndex = getAttributeIndex(submesh, VertexAttribute::color, set);
  if (attribIndex == -1) {
    return;
  }

  setVertexAttribute(submesh, index, attribIndex, v);
}

int KRMesh::getBoneIndex(int submesh, int index, int weight_index) const
{
  int attribIndex = getAttributeIndex(submesh, VertexAttribute::joints, weight_index / 4);
  if (attribIndex == -1) {
    return 0;
  }

  // TODO - Implement integer based attribute access
  Vector4 v;
  getVertexAttribute(submesh, index, attribIndex, &v);
  return static_cast<int>(v[weight_index % 4]);
}

void KRMesh::setBoneIndex(int submesh, int index, int weight_index, int bone_index)
{
  int attribIndex = getAttributeIndex(submesh, VertexAttribute::joints, weight_index / 4);
  if (attribIndex == -1) {
    return;
  }

  // TODO - Implement integer based attribute access
  Vector4 v;
  getVertexAttribute(submesh, index, attribIndex, &v);
  v[weight_index % 4] = bone_index;
  setVertexAttribute(submesh, index, attribIndex, v);
}

void KRMesh::setBoneWeight(int submesh, int index, int bone_index, float weight)
{
  int attribIndex = getAttributeIndex(submesh, VertexAttribute::weights, bone_index / 4);
  if (attribIndex == -1) {
    return;
  }
  hydra::Vector4 v;
  getVertexAttribute(submesh, index, attribIndex, &v);
  v[bone_index % 4] = weight;
  setVertexAttribute(submesh, index, attribIndex, v);
}

float KRMesh::getBoneWeight(int submesh, int index, int weight_index) const
{
  int attribIndex = getAttributeIndex(submesh, VertexAttribute::weights, weight_index / 4);
  if (attribIndex == -1) {
    return 0.f;
  }

  Vector4 v;
  getVertexAttribute(submesh, index, attribIndex, &v);
  return v[weight_index % 4];
}

VkFormat KRMesh::AttributeVulkanFormat(const VertexAttributeInfo &attribute)
{
  switch (attribute.type) {

  // ----====---- scalar ----====----
  case DataType::scalar:
    switch (attribute.component) {
    case ComponentType::empty:
      return VK_FORMAT_UNDEFINED;
    case ComponentType::int8:
      switch (attribute.normalization) {
      case Normalization::none:
        return VK_FORMAT_R8_SINT;
      case Normalization::scaled:
        return VK_FORMAT_R8_SSCALED;
      case Normalization::normalized:
        return VK_FORMAT_R8_SNORM;
      case Normalization::srgb:
        return VK_FORMAT_R8_SRGB;
      }
    case ComponentType::uint8:
      switch (attribute.normalization) {
      case Normalization::none:
        return VK_FORMAT_R8_UINT;
      case Normalization::scaled:
        return VK_FORMAT_R8_USCALED;
      case Normalization::normalized:
        return VK_FORMAT_R8_UNORM;
      case Normalization::srgb:
        return VK_FORMAT_R8_SRGB;
      }
    case ComponentType::int16:
      switch (attribute.normalization) {
      case Normalization::none:
        return VK_FORMAT_R16_SINT;
      case Normalization::scaled:
        return VK_FORMAT_R16_SSCALED;
      case Normalization::normalized:
        return VK_FORMAT_R16_SNORM;
      case Normalization::srgb:
        return VK_FORMAT_UNDEFINED;
      }
    case ComponentType::uint16:
      switch (attribute.normalization) {
      case Normalization::none:
        return VK_FORMAT_R16_UINT;
      case Normalization::scaled:
        return VK_FORMAT_R16_USCALED;
      case Normalization::normalized:
        return VK_FORMAT_R16_UNORM;
      case Normalization::srgb:
        return VK_FORMAT_UNDEFINED;
      }
    case ComponentType::int32:
        return VK_FORMAT_R32_SINT;
    case ComponentType::uint32:
        return VK_FORMAT_R32_UINT;
    case ComponentType::int64:
        return VK_FORMAT_R64_SINT;
    case ComponentType::uint64:
        return VK_FORMAT_R64_UINT;
    case ComponentType::float16:
      return VK_FORMAT_R16_SFLOAT;
    case ComponentType::float32:
      return VK_FORMAT_R32_SFLOAT;
    case ComponentType::float64:
      return VK_FORMAT_R64_SFLOAT;
    }
    break;


  // ----====---- vec2, mat2 ----====----
  case DataType::vec2:
  case DataType::mat2:
    switch (attribute.component) {
    case ComponentType::empty:
      return VK_FORMAT_UNDEFINED;
    case ComponentType::int8:
      switch (attribute.normalization) {
      case Normalization::none:
        return VK_FORMAT_R8G8_SINT;
      case Normalization::scaled:
        return VK_FORMAT_R8G8_SSCALED;
      case Normalization::normalized:
        return VK_FORMAT_R8G8_SNORM;
      case Normalization::srgb:
        return VK_FORMAT_R8G8_SRGB;
      }
    case ComponentType::uint8:
      switch (attribute.normalization) {
      case Normalization::none:
        return VK_FORMAT_R8G8_UINT;
      case Normalization::scaled:
        return VK_FORMAT_R8G8_USCALED;
      case Normalization::normalized:
        return VK_FORMAT_R8G8_UNORM;
      case Normalization::srgb:
        return VK_FORMAT_R8G8_SRGB;
      }
    case ComponentType::int16:
      switch (attribute.normalization) {
      case Normalization::none:
        return VK_FORMAT_R16G16_SINT;
      case Normalization::scaled:
        return VK_FORMAT_R16G16_SSCALED;
      case Normalization::normalized:
        return VK_FORMAT_R16G16_SNORM;
      case Normalization::srgb:
        return VK_FORMAT_UNDEFINED;
      }
    case ComponentType::uint16:
      switch (attribute.normalization) {
      case Normalization::none:
        return VK_FORMAT_R16G16_UINT;
      case Normalization::scaled:
        return VK_FORMAT_R16G16_USCALED;
      case Normalization::normalized:
        return VK_FORMAT_R16G16_UNORM;
      case Normalization::srgb:
        return VK_FORMAT_UNDEFINED;
      }
    case ComponentType::int32:
      return VK_FORMAT_R32G32_SINT;
    case ComponentType::uint32:
      return VK_FORMAT_R32G32_UINT;
    case ComponentType::int64:
      return VK_FORMAT_R64G64_SINT;
    case ComponentType::uint64:
      return VK_FORMAT_R64G64_UINT;
    case ComponentType::float16:
      return VK_FORMAT_R16G16_SFLOAT;
    case ComponentType::float32:
      return VK_FORMAT_R32G32_SFLOAT;
    case ComponentType::float64:
      return VK_FORMAT_R64G64_SFLOAT;
    }
    break;

  // ----====---- vec3, mat3 ----====----
  case DataType::vec3:
  case DataType::mat3:
    switch (attribute.component) {
    case ComponentType::empty:
      return VK_FORMAT_UNDEFINED;
    case ComponentType::int8:
      switch (attribute.normalization) {
      case Normalization::none:
        return VK_FORMAT_R8G8B8_SINT;
      case Normalization::scaled:
        return VK_FORMAT_R8G8B8_SSCALED;
      case Normalization::normalized:
        return VK_FORMAT_R8G8B8_SNORM;
      case Normalization::srgb:
        return VK_FORMAT_R8G8B8_SRGB;
      }
    case ComponentType::uint8:
      switch (attribute.normalization) {
      case Normalization::none:
        return VK_FORMAT_R8G8B8_UINT;
      case Normalization::scaled:
        return VK_FORMAT_R8G8B8_USCALED;
      case Normalization::normalized:
        return VK_FORMAT_R8G8B8_UNORM;
      case Normalization::srgb:
        return VK_FORMAT_R8G8B8_SRGB;
      }
    case ComponentType::int16:
      switch (attribute.normalization) {
      case Normalization::none:
        return VK_FORMAT_R16G16B16_SINT;
      case Normalization::scaled:
        return VK_FORMAT_R16G16B16_SSCALED;
      case Normalization::normalized:
        return VK_FORMAT_R16G16B16_SNORM;
      case Normalization::srgb:
        return VK_FORMAT_UNDEFINED;
      }
    case ComponentType::uint16:
      switch (attribute.normalization) {
      case Normalization::none:
        return VK_FORMAT_R16G16B16_UINT;
      case Normalization::scaled:
        return VK_FORMAT_R16G16B16_USCALED;
      case Normalization::normalized:
        return VK_FORMAT_R16G16B16_UNORM;
      case Normalization::srgb:
        return VK_FORMAT_UNDEFINED;
      }
    case ComponentType::int32:
      return VK_FORMAT_R32G32B32_SINT;
    case ComponentType::uint32:
      return VK_FORMAT_R32G32B32_UINT;
    case ComponentType::int64:
      return VK_FORMAT_R64G64B64_SINT;
    case ComponentType::uint64:
      return VK_FORMAT_R64G64B64_UINT;
    case ComponentType::float16:
      return VK_FORMAT_R16G16B16_SFLOAT;
    case ComponentType::float32:
      return VK_FORMAT_R32G32B32_SFLOAT;
    case ComponentType::float64:
      return VK_FORMAT_R64G64B64_SFLOAT;
    }
    break;

  // ----====---- vec4, mat4 ----====----
  case DataType::vec4:
  case DataType::mat4:
    switch (attribute.component) {
    case ComponentType::empty:
      return VK_FORMAT_UNDEFINED;
    case ComponentType::int8:
      switch (attribute.normalization) {
      case Normalization::none:
        return VK_FORMAT_R8G8B8A8_SINT;
      case Normalization::scaled:
        return VK_FORMAT_R8G8B8A8_SSCALED;
      case Normalization::normalized:
        return VK_FORMAT_R8G8B8A8_SNORM;
      case Normalization::srgb:
        return VK_FORMAT_R8G8B8A8_SRGB;
      }
    case ComponentType::uint8:
      switch (attribute.normalization) {
      case Normalization::none:
        return VK_FORMAT_R8G8B8A8_UINT;
      case Normalization::scaled:
        return VK_FORMAT_R8G8B8A8_USCALED;
      case Normalization::normalized:
        return VK_FORMAT_R8G8B8A8_UNORM;
      case Normalization::srgb:
        return VK_FORMAT_R8G8B8A8_SRGB;
      }
    case ComponentType::int16:
      switch (attribute.normalization) {
      case Normalization::none:
        return VK_FORMAT_R16G16B16A16_SINT;
      case Normalization::scaled:
        return VK_FORMAT_R16G16B16A16_SSCALED;
      case Normalization::normalized:
        return VK_FORMAT_R16G16B16A16_SNORM;
      case Normalization::srgb:
        return VK_FORMAT_UNDEFINED;
      }
    case ComponentType::uint16:
      switch (attribute.normalization) {
      case Normalization::none:
        return VK_FORMAT_R16G16B16A16_UINT;
      case Normalization::scaled:
        return VK_FORMAT_R16G16B16A16_USCALED;
      case Normalization::normalized:
        return VK_FORMAT_R16G16B16A16_UNORM;
      case Normalization::srgb:
        return VK_FORMAT_UNDEFINED;
      }
    case ComponentType::int32:
      return VK_FORMAT_R32G32B32A32_SINT;
    case ComponentType::uint32:
      return VK_FORMAT_R32G32B32A32_UINT;
    case ComponentType::int64:
      return VK_FORMAT_R64G64B64A64_SINT;
    case ComponentType::uint64:
      return VK_FORMAT_R64G64B64A64_UINT;
    case ComponentType::float16:
      return VK_FORMAT_R16G16B16A16_SFLOAT;
    case ComponentType::float32:
      return VK_FORMAT_R32G32B32A32_SFLOAT;
    case ComponentType::float64:
      return VK_FORMAT_R64G64B64A64_SFLOAT;
    }
    break;
  }
  return VK_FORMAT_UNDEFINED;
}

int KRMesh::getBoneCount()
{
  pack_header* header = getHeader();
  int bone_count = header->bone_count;
  return bone_count;
}

char* KRMesh::getBoneName(int bone_index)
{
  return getBone(bone_index)->szName;
}

Matrix4 KRMesh::getBoneBindPose(int bone_index)
{
  return Matrix4::Create(getBone(bone_index)->bind_pose);
}

bool KRMesh::rayCast(const Vector3& start, const Vector3& dir, const Triangle3& tri, const Vector3& tri_n0, const Vector3& tri_n1, const Vector3& tri_n2, HitInfo& hitinfo)
{
  Vector3 hit_point;
  if (tri.rayCast(start, dir, hit_point)) {
    // ---===--- hit_point is in triangle ---===---

    float new_hit_distance = (hit_point - start).magnitude();
    if (new_hit_distance < hitinfo.getDistance() || !hitinfo.didHit()) {
      // Update the hitinfo object if this hit is closer than the prior hit

      // Interpolate between the three vertex normals, performing a 3-way lerp of tri_n0, tri_n1, and tri_n2
      float distance_v0 = (tri[0] - hit_point).magnitude();
      float distance_v1 = (tri[1] - hit_point).magnitude();
      float distance_v2 = (tri[2] - hit_point).magnitude();
      float distance_total = distance_v0 + distance_v1 + distance_v2;
      distance_v0 /= distance_total;
      distance_v1 /= distance_total;
      distance_v2 /= distance_total;
      Vector3 normal = Vector3::Normalize(tri_n0 * (1.0f - distance_v0) + tri_n1 * (1.0f - distance_v1) + tri_n2 * (1.0f - distance_v2));

      hitinfo = HitInfo(hit_point, normal, new_hit_distance);
      return true;
    } else {
      return false; // The hit was farther than an existing hit
    }

  } else {
    // Dit not hit the triangle
    return false;
  }

}


bool KRMesh::rayCast(const Vector3& start, const Vector3& dir, HitInfo& hitinfo) const
{
  m_pData->lock();
  bool hit_found = false;
  for (int submesh_index = 0; submesh_index < getSubmeshCount(); submesh_index++) {
    const pack_primitive* primitive = getPrimitive(submesh_index);
    switch (primitive->layout.topology) {
    case Topology::Triangles:
      for (int triangle_index = 0; triangle_index < primitive->vertexCount / 3; triangle_index++) {
        int tri_vert_index[3]; // FINDME, HACK!  This is not very efficient for indexed collider meshes...
        tri_vert_index[0] = getVertexIndex(submesh_index, triangle_index * 3);
        tri_vert_index[1] = getVertexIndex(submesh_index, triangle_index * 3 + 1);
        tri_vert_index[2] = getVertexIndex(submesh_index, triangle_index * 3 + 2);

        Triangle3 tri = Triangle3::Create(getVertexPosition(submesh_index, tri_vert_index[0]), getVertexPosition(submesh_index, tri_vert_index[1]), getVertexPosition(submesh_index, tri_vert_index[2]));

        if (rayCast(start, dir, tri, getVertexNormal(submesh_index, tri_vert_index[0]), getVertexNormal(submesh_index, tri_vert_index[1]), getVertexNormal(submesh_index, tri_vert_index[2]), hitinfo)) hit_found = true;
      }
      break;
    default:
      assert(false); // Not yet implemented
      break;
    }
  }
  m_pData->unlock();
  return hit_found;
}


bool KRMesh::sphereCast(const Matrix4& model_to_world, const Vector3& v0, const Vector3& v1, float radius, HitInfo& hitinfo) const
{
  m_pData->lock();

  bool hit_found = false;
  for (int submesh_index = 0; submesh_index < getSubmeshCount(); submesh_index++) {
    const pack_primitive* primitive = getPrimitive(submesh_index);
    switch (primitive->layout.topology) {
      case Topology::Triangles:
      for (int triangle_index = 0; triangle_index < primitive->vertexCount / 3; triangle_index++) {
        int tri_vert_index[3]; // FINDME, HACK!  This is not very efficient for indexed collider meshes...
        tri_vert_index[0] = getVertexIndex(submesh_index, triangle_index * 3);
        tri_vert_index[1] = getVertexIndex(submesh_index, triangle_index * 3 + 1);
        tri_vert_index[2] = getVertexIndex(submesh_index, triangle_index * 3 + 2);

        Triangle3 tri = Triangle3::Create(getVertexPosition(submesh_index, tri_vert_index[0]), getVertexPosition(submesh_index, tri_vert_index[1]), getVertexPosition(submesh_index, tri_vert_index[2]));

        if (sphereCast(model_to_world, v0, v1, radius, tri, hitinfo)) hit_found = true;
      }
      break;
    default:
      assert(false); // Not yet implemented
      break;
    }
  }
  m_pData->unlock();

  return hit_found;
}

bool KRMesh::sphereCast(const Matrix4& model_to_world, const Vector3& v0, const Vector3& v1, float radius, const Triangle3& tri, HitInfo& hitinfo)
{

  Vector3 dir = Vector3::Normalize(v1 - v0);
  Vector3 start = v0;

  Vector3 new_hit_point;
  float new_hit_distance;

  Triangle3 world_tri = Triangle3::Create(Matrix4::Dot(model_to_world, tri[0]), Matrix4::Dot(model_to_world, tri[1]), Matrix4::Dot(model_to_world, tri[2]));

  if (world_tri.sphereCast(start, dir, radius, new_hit_point, new_hit_distance)) {
    if ((!hitinfo.didHit() || hitinfo.getDistance() > new_hit_distance) && new_hit_distance <= (v1 - v0).magnitude()) {

      /*
      // Interpolate between the three vertex normals, performing a 3-way lerp of tri_n0, tri_n1, and tri_n2
      float distance_v0 = (tri[0] - new_hit_point).magnitude();
      float distance_v1 = (tri[1] - new_hit_point).magnitude();
      float distance_v2 = (tri[2] - new_hit_point).magnitude();
      float distance_total = distance_v0 + distance_v1 + distance_v2;
      distance_v0 /= distance_total;
      distance_v1 /= distance_total;
      distance_v2 /= distance_total;
      Vector3 normal = Vector3::Normalize(Matrix4::DotNoTranslate(model_to_world, (tri_n0 * (1.0 - distance_v0) + tri_n1 * (1.0 - distance_v1) + tri_n2 * (1.0 - distance_v2))));
      */
      hitinfo = HitInfo(new_hit_point, world_tri.calculateNormal(), new_hit_distance);
      return true;
    }
  }

  return false;
}

bool KRMesh::lineCast(const Vector3& v0, const Vector3& v1, HitInfo& hitinfo) const
{
  m_pData->lock();
  HitInfo new_hitinfo;
  Vector3 dir = Vector3::Normalize(v1 - v0);
  if (rayCast(v0, dir, new_hitinfo)) {
    if ((new_hitinfo.getPosition() - v0).sqrMagnitude() <= (v1 - v0).sqrMagnitude()) {
      // The hit was between v1 and v2
      hitinfo = new_hitinfo;
      m_pData->unlock();
      return true;
    }
  }
  m_pData->unlock();
  return false; // Either no hit, or the hit was beyond v1
}

void KRMesh::convertToIndexed()
{
  // TODO: Update and re-enable
  /*
  m_pData->lock();
  KRMesh::pack_header* header = getHeader();
  const VertexBufferLayout* layout = &header->primitive.layout;
  pack_material* packMaterial = (pack_material*)(header + 1);

  // Convert model to indexed vertices, identying vertexes with identical attributes and optimizing order of trianges for best usage post-vertex-transform cache on GPU
  int vertex_index_offset = 0;
  int vertex_index_base_start_vertex = 0;

  std::vector<std::byte> newVertexData;
  int newVertexCount = 0;
  std::vector<uint16_t> newIndexes;
  std::vector<std::pair<int, int>> newVertexIndexRanges;

  for (int submesh_index = 0; submesh_index < getSubmeshCount(); submesh_index++) {
    pack_material* pPackMaterial = getSubmesh(submesh_index);

    int vertexes_remaining = getVertexCount(submesh_index);

    int vertex_count = vertexes_remaining;
    if (vertex_count > 0xffff) {
      vertex_count = 0xffff;
    }

    if (submesh_index == 0 || vertex_index_offset + vertex_count > 0xffff) {
      newVertexIndexRanges.push_back(std::pair<int, int>((int)newIndexes.size(), newVertexCount));
      vertex_index_offset = 0;
      vertex_index_base_start_vertex = newVertexCount;
    }

    int source_index = pPackMaterial->start_vertex;
    pPackMaterial->start_vertex = (int)newVertexIndexRanges.size() - 1 + (vertex_index_offset << 16);
    pPackMaterial->vertex_count = vertexes_remaining;

    while (vertexes_remaining) {
      typedef std::vector<std::byte> vertex_data_t;

      std::map<vertex_data_t, int> prevIndices;

      for (int i = 0; i < vertex_count; i++) {
        vertex_data_t vertexData;
        const std::byte* vertexBytes = reinterpret_cast<const std::byte*>(getVertexData(source_index));
        vertexData.insert(vertexData.end(), vertexBytes, vertexBytes + layout->vertexSize);
        
        int found_index = -1;
        if (prevIndices.count(vertexData) == 0) {
          found_index = (int)(newVertexCount) - vertex_index_base_start_vertex;
          prevIndices[vertexData] = found_index;
          newVertexData.insert(newVertexData.end(), vertexData.begin(), vertexData.end());
          newVertexCount++;
        } else {
          found_index = prevIndices[vertexData];
        }

        newIndexes.push_back(found_index);
        //fprintf(stderr, "Submesh: %6i  IndexBase: %3i  Index: %6i\n", submesh_index, vertex_index_bases.size(), found_index);

        source_index++;
      }

      vertexes_remaining -= vertex_count;
      vertex_index_offset += vertex_count;

      vertex_count = vertexes_remaining;
      if (vertex_count > 0xffff) {
        vertex_count = 0xffff;
      }

      if (vertex_index_offset + vertex_count > 0xffff) {
        newVertexIndexRanges.push_back(std::pair<int, int>((int)newIndexes.size(), newVertexCount));
        vertex_index_offset = 0;
        vertex_index_base_start_vertex = newVertexCount;
      }
    }
  }

  KRContext::Log(KRContext::LOG_LEVEL_INFORMATION, "Convert to indexed, before: %i after: %i (%.2f%% saving)", getHeader()->primitive.vertexCount, newVertexCount, ((float)getHeader()->primitive.vertexCount - (float)newVertexCount) / (float)getHeader()->primitive.vertexCount * 100.0f);

  int submesh_count = getSubmeshCount();
  int bone_count = getBoneCount();

  header->index_base_count = newVertexIndexRanges.size();
  header->primitive.indexCount = newIndexes.size();
  header->primitive.vertexCount = newVertexCount;

  size_t new_file_size = sizeof(pack_header) + sizeof(pack_material) * submesh_count + sizeof(pack_bone) * bone_count + KRALIGN(2 * header->primitive.indexCount) + KRALIGN(8 * header->index_base_count) + newVertexData.size();

  pack_header ph;
  m_pData->copy((void*)&ph, 0, sizeof(ph));

  // ---- Resize Data Blocks ----
  m_pData->unlock();
  releaseData(false);

  m_pData->expand(new_file_size);

  m_pMetaData = m_pData->getSubBlock(0, sizeof(pack_header) + sizeof(pack_material) * ph.submesh_count + sizeof(pack_bone) * ph.bone_count);
  m_pMetaData->lock();
  m_pIndexBaseData = m_pData->getSubBlock(sizeof(pack_header) + sizeof(pack_material) * ph.submesh_count + sizeof(pack_bone) * ph.bone_count + KRALIGN(2 * ph.primitive.indexCount), ph.index_base_count * 8);
  m_pIndexBaseData->lock();

  // ---- Copy new buffers ----
  m_pData->lock();

  // Vertex Data
  void *vertex_data = getVertexData();
  memcpy(vertex_data, newVertexData.data(), newVertexData.size());
  
  // Index Data
  __uint16_t* index_data = getIndexData();
  memcpy(index_data, newIndexes.data(), newIndexes.size());

  // Index data ranges
  __uint32_t* index_base_data = getIndexBaseData();
  for (std::vector<std::pair<int, int> >::const_iterator itr = newVertexIndexRanges.begin(); itr != newVertexIndexRanges.end(); itr++) {
    *index_base_data++ = (*itr).first;
    *index_base_data++ = (*itr).second;
  }

  m_pData->unlock();
  // ---- End: Copy new buffers ----

  optimize();

  if (m_constant) {
    // Ensure that constant models loaded immediately by the streamer
    getPrimitives();
    getMaterials();
  }
  */
}

void KRMesh::optimize()
{
  if (getIndexCount(0) > 0) {
    optimizeIndexes();
  } else {
    convertToIndexed(); // HACK, FINDME, TODO - This may not be ideal in every case and should be exposed through the API independently
  }
}

int KRMesh::getVertexIndex(int submesh, int index) const
{
  if (getIndexCount(submesh) > 0) {
    const int kVertexIndexSize = 2; // TODO: Support both 16 and 32 bit vertex indexes
    std::byte* vertexIndexData = (std::byte*)m_indexBlocks[submesh]->getStart() + kVertexIndexSize * index;
    return *((uint16_t*)vertexIndexData);
  } else {
    return index;
  }
}

void KRMesh::setVertexIndex(int submesh, int index, int indexVal)
{
  const int kVertexIndexSize = 2; // TODO: Support both 16 and 32 bit vertex indexes
  std::byte* vertexIndexData = (std::byte*)m_indexBlocks[submesh]->getStart() + kVertexIndexSize * index;
  uint16_t val = static_cast<uint16_t>(indexVal);
  *((uint16_t*)vertexIndexData) = val;
}

void KRMesh::optimizeIndexes()
{
  // TODO - Re-enable this once crash with KRMeshSphere vertices is corrected and it is updated to match refactored KRMesh code
  return;
  /*

  m_pData->lock();
  // TODO - Implement optimization for indexed strips
  if (getTopology() == Topology::Triangles && getIndexCount(0) > 0) {
    int vertex_size = (int)getHeader()->primitive.layout.vertexSize;
    __uint16_t* new_indices = (__uint16_t*)malloc(0x10000 * sizeof(__uint16_t));
    __uint16_t* vertex_mapping = (__uint16_t*)malloc(0x10000 * sizeof(__uint16_t));
    unsigned char* new_vertex_data = (unsigned char*)malloc(vertex_size * 0x10000);

    // FINDME, TODO, HACK - This will segfault if the KRData object is still mmap'ed to a read-only file.  Need to detach from the file before calling this function.  Currently, this function is only being used during the import process, so it isn't going to cause any problems for now.

    pack_header* header = getHeader();

    __uint16_t* index_data = getIndexData();
    // unsigned char *vertex_data = getVertexData(); // Uncomment when re-enabling Step 2 below

    for (int submesh_index = 0; submesh_index < header->submesh_count; submesh_index++) {
      pack_material* submesh = getSubmesh(submesh_index);
      int vertexes_remaining = submesh->vertex_count;
      int index_group = getSubmesh(submesh_index)->index_group;
      int index_group_offset = getSubmesh(submesh_index)->index_group_offset;
      while (vertexes_remaining > 0) {
        int start_index_offset, start_vertex_offset, index_count, vertex_count;
        getIndexedRange(index_group++, start_index_offset, start_vertex_offset, index_count, vertex_count);

        int vertexes_to_process = vertexes_remaining;
        if (vertexes_to_process + index_group_offset > 0xffff) {
          vertexes_to_process = 0xffff - index_group_offset;
        }

        __uint16_t* index_data_start = index_data + start_index_offset + index_group_offset;


        // ----====---- Step 1: Optimize triangle drawing order to maximize use of the GPU's post-transform vertex cache ----====----
        Forsyth::OptimizeFaces(index_data_start, vertexes_to_process, vertex_count, new_indices, 16); // FINDME, TODO - GPU post-transform vertex cache size of 16 should be configureable
        memcpy(index_data_start, new_indices, vertexes_to_process * sizeof(__uint16_t));
        vertexes_remaining -= vertexes_to_process;

         unsigned char * vertex_data_start = vertex_data + start_vertex_offset;

        // ----====---- Step 2: Re-order the vertex data to maintain cache coherency ----====----
        for(int i=0; i < vertex_count; i++) {
            vertex_mapping[i] = i;
        }
        int new_vertex_index=0;
        for(int index_number=0; index_number<index_count; index_number++) {
            int prev_vertex_index = index_data_start[index_number];
            if(prev_vertex_index > new_vertex_index) {
                // Swap prev_vertex_index and new_vertex_index

                for(int i=0; i < index_count; i++) {
                    if(index_data_start[i] == prev_vertex_index) {
                        index_data_start[i] = new_vertex_index;
                    } else if(index_data_start[i] == new_vertex_index) {
                        index_data_start[i] = prev_vertex_index;
                    }
                }

                int tmp = vertex_mapping[prev_vertex_index];
                vertex_mapping[prev_vertex_index] = vertex_mapping[new_vertex_index];
                vertex_mapping[new_vertex_index] = tmp;


                new_vertex_index++;
            }
        }

        for(int i=0; i < vertex_count; i++) {
            int vertex_size = (int)getHeader()->primitive.vertexSize;
            memcpy(new_vertex_data + vertex_mapping[i] * vertex_size, vertex_data_start + i * m_vertex_size, m_vertex_size);
        }
        memcpy(vertex_data_start, new_vertex_data, vertex_count * vertex_size);




        index_group_offset = 0;
      }
    }

    free(new_indices);
    free(vertex_mapping);
    free(new_vertex_data);
  } // getTopology() == Topology::Triangles && getIndexCount(0) > 0

  m_pData->unlock();
  */
}
