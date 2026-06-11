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
  m_pIndexBaseData = NULL;
  m_constant = false;
}

KRMesh::KRMesh(KRContext& context, std::string name, Block* data) : KRResource(context, name)
{
  setName(name);

  m_hasTransparency = false;
  m_pData = NULL;
  m_pMetaData = NULL;
  m_pIndexBaseData = NULL;
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
  m_submeshes.clear();
  if (m_pIndexBaseData) {
    m_pIndexBaseData->unlock();
    delete m_pIndexBaseData;
    m_pIndexBaseData = NULL;
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

  pack_header ph;
  m_pData->copy((void*)&ph, 0, sizeof(ph));
  m_pMetaData = m_pData->getSubBlock(0, sizeof(pack_header) + sizeof(pack_material) * ph.submesh_count + sizeof(pack_bone) * ph.bone_count);
  m_pMetaData->lock();

  m_pIndexBaseData = m_pData->getSubBlock(sizeof(pack_header) + sizeof(pack_material) * ph.submesh_count + sizeof(pack_bone) * ph.bone_count + KRALIGN(2 * ph.primitive.indexCount), ph.index_base_count * 8);
  m_pIndexBaseData->lock();

  m_extents = ph.extents;
}

void KRMesh::getMaterials()
{
  if (m_materials.size() != 0) {
    return;
  }

  for (std::vector<KRMesh::Submesh>::iterator itr = m_submeshes.begin(); itr != m_submeshes.end(); itr++) {
    const char* szMaterialName = (*itr).szMaterialName;
    m_materials.push_back(KRMaterialBinding(szMaterialName));
  }
}

void KRMesh::requestResidency(uint32_t usage, float lodCoverage)
{
  KRResource::requestResidency(usage, lodCoverage);

  for (Submesh& mesh : m_submeshes) {
    for (shared_ptr<KRMeshManager::KRVBOData>& vbo : mesh.vbo_data_blocks) {
      vbo->requestResidency(lodCoverage);
    }
  }
}

void KRMesh::preStream()
{
  getSubmeshes();
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
  getSubmeshes();
  getMaterials();

  for (KRMaterialBinding& material : m_materials) {
    if (material.isBound()) {
      stream_level = std::min(stream_level, material.get()->getStreamLevel());
    }
  }
  bool all_vbo_data_loaded = true;
  bool vbo_data_loaded = false;
  for (const Submesh& mesh : m_submeshes) {
    for (const shared_ptr<KRMeshManager::KRVBOData>& vbo_data : mesh.vbo_data_blocks) {
      if (vbo_data->isVBOReady()) {
        vbo_data_loaded = true;
      } else {
        all_vbo_data_loaded = false;
      }
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
      getSubmeshes();
      getMaterials();

      int cSubmeshes = (int)m_submeshes.size();
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
                if (pMaterial->bind(ri, &getHeader()->primitive.layout, CullMode::kCullBack, bones, bone_bind_poses, matModel, pLightMap, lod_coverage))
                {
                  renderSubmesh(ri.commandBuffer, iSubmesh, ri.renderPass, object_name, pMaterial->getName(), lod_coverage);
                }
                break;
              case KRMaterial::KRMATERIAL_ALPHA_MODE_BLEND: // Blended Alpha
                if (pMaterial->m_doubleSided) {
                  // Blended alpha rendered in two passes.  First pass renders backfaces; second pass renders frontfaces.
                  // 
                  // Render back faces before front faces
                  if (pMaterial->bind(ri, &getHeader()->primitive.layout, CullMode::kCullFront, bones, bone_bind_poses, matModel, pLightMap, lod_coverage))
                  {
                    renderSubmesh(ri.commandBuffer, iSubmesh, ri.renderPass, object_name, pMaterial->getName(), lod_coverage);
                  }
                }

                // Render front faces
                if (pMaterial->bind(ri, &getHeader()->primitive.layout, CullMode::kCullBack, bones, bone_bind_poses, matModel, pLightMap, lod_coverage))
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
  Vector3 size = m_extents.size();
  if (size.x > m) m = size.x;
  if (size.y > m) m = size.y;
  if (size.z > m) m = size.z;
  return m;
}

bool KRMesh::hasTransparency()
{
  return m_hasTransparency;
}


void KRMesh::getSubmeshes()
{
  if (m_submeshes.size() == 0) {
    pack_header* pHeader = getHeader();
    pack_material* pPackMaterials = (pack_material*)(pHeader + 1);
    m_submeshes.clear();
    for (int iMaterial = 0; iMaterial < pHeader->submesh_count; iMaterial++) {
      pack_material* pPackMaterial = pPackMaterials + iMaterial;

      Submesh& mesh = m_submeshes.emplace_back();
      mesh.start_vertex = pPackMaterial->start_vertex;
      mesh.vertex_count = pPackMaterial->vertex_count;

      strncpy(mesh.szMaterialName, pPackMaterial->szName, KRENGINE_MAX_NAME_LENGTH);
      mesh.szMaterialName[KRENGINE_MAX_NAME_LENGTH - 1] = '\0';
      //fprintf(stderr, "Submesh material: \"%s\"\n", mesh->szMaterialName);
    }
    createDataBlocks(m_constant ? KRMeshManager::KRVBOData::CONSTANT : KRMeshManager::KRVBOData::STREAMING);
  }
}

void KRMesh::createDataBlocks(KRMeshManager::KRVBOData::vbo_type t)
{
  int cSubmeshes = (int)m_submeshes.size();
  for (int iSubmesh = 0; iSubmesh < cSubmeshes; iSubmesh++) {
    Submesh& mesh = m_submeshes[iSubmesh];
    int cVertexes = mesh.vertex_count;
    int vertex_data_offset = (int)getVertexDataOffset();
    int index_data_offset = (int)getIndexDataOffset();
    pack_header* pHeader = getHeader();
    int32_t vertex_count = pHeader->primitive.vertexCount;

    int vbo_index = 0;
    if (getIndexCount(iSubmesh) > 0) {

      int index_group = getSubmesh(iSubmesh)->index_group;
      int index_group_offset = getSubmesh(iSubmesh)->index_group_offset;
      while (cVertexes > 0) {

        int start_index_offset, start_vertex_offset, index_count, vertex_count;
        getIndexedRange(index_group++, start_index_offset, start_vertex_offset, index_count, vertex_count);

        if ((int)mesh.vertex_data_blocks.size() <= vbo_index) {
          int vertex_count = getHeader()->primitive.vertexCount;
          int vertex_size = getHeader()->primitive.layout.vertexSize;
          Block* vertex_data_block = m_pData->getSubBlock(vertex_data_offset + start_vertex_offset * vertex_size, vertex_count * vertex_size);
          Block* index_data_block = m_pData->getSubBlock(index_data_offset + start_index_offset * 2, index_count * 2);
          mesh.vbo_data_blocks.emplace_back(std::make_shared<KRMeshManager::KRVBOData>(getContext().getMeshManager(), vertex_data_block, index_data_block, &pHeader->primitive.layout, true, t
#if KRENGINE_DEBUG_GPU_LABELS
            , m_lodBaseName.c_str()
#endif
          ));
          mesh.vertex_data_blocks.push_back(vertex_data_block);
          mesh.index_data_blocks.push_back(index_data_block);
        }
        vbo_index++;


        int vertex_draw_count = cVertexes;
        if (vertex_draw_count > index_count - index_group_offset) vertex_draw_count = index_count - index_group_offset;

        cVertexes -= vertex_draw_count;
        index_group_offset = 0;
      }
    } else {
      int cBuffers = (vertex_count + MAX_VBO_SIZE - 1) / MAX_VBO_SIZE;
      int iVertex = mesh.start_vertex;
      int iBuffer = iVertex / MAX_VBO_SIZE;
      iVertex = iVertex % MAX_VBO_SIZE;
      while (cVertexes > 0) {
        int cBufferVertexes = iBuffer < cBuffers - 1 ? MAX_VBO_SIZE : vertex_count % MAX_VBO_SIZE;
        int vertex_size = getHeader()->primitive.layout.vertexSize;

        if ((int)mesh.vertex_data_blocks.size() <= vbo_index) {
          Block* index_data_block = NULL;
          Block* vertex_data_block = m_pData->getSubBlock(vertex_data_offset + iBuffer * MAX_VBO_SIZE * vertex_size, vertex_size * cBufferVertexes);
          mesh.vbo_data_blocks.emplace_back(std::make_shared<KRMeshManager::KRVBOData>(getContext().getMeshManager(), vertex_data_block, index_data_block, &pHeader->primitive.layout, true, t
#if KRENGINE_DEBUG_GPU_LABELS
            , m_lodBaseName.c_str()
#endif
          ));
          mesh.vertex_data_blocks.emplace_back(vertex_data_block);
        }
        vbo_index++;

        if (iVertex + cVertexes >= MAX_VBO_SIZE) {
          assert(iVertex + (MAX_VBO_SIZE - iVertex) <= cBufferVertexes);

          cVertexes -= (MAX_VBO_SIZE - iVertex);
          iVertex = 0;
          iBuffer++;
        } else {
          assert(iVertex + cVertexes <= cBufferVertexes);

          cVertexes = 0;
        }

      }
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

bool KRMesh::isReady() const
{
  // TODO - This should be cached...
  for (const Submesh& mesh : m_submeshes) {
    for (const shared_ptr<KRMeshManager::KRVBOData>& vbo_data_block : mesh.vbo_data_blocks) {
      if (!vbo_data_block->isVBOReady()) {
        return false;
      }
    }
  }
  return true;
}

void KRMesh::renderSubmesh(VkCommandBuffer& commandBuffer, int iSubmesh, const KRRenderPass* renderPass, const std::string& object_name, const std::string& material_name, float lodCoverage)
{
  getSubmeshes();

  Submesh& mesh = m_submeshes[iSubmesh];
  int cVertexes = mesh.vertex_count;

  vector<shared_ptr<KRMeshManager::KRVBOData>>::iterator vbo_itr = mesh.vbo_data_blocks.begin();
  int vbo_index = 0;
  if (getIndexCount(0) > 0) {

    int index_group = getSubmesh(iSubmesh)->index_group;
    int index_group_offset = getSubmesh(iSubmesh)->index_group_offset;
    while (cVertexes > 0) {

      int start_index_offset, start_vertex_offset, index_count, vertex_count;
      getIndexedRange(index_group++, start_index_offset, start_vertex_offset, index_count, vertex_count);

      KRMeshManager::KRVBOData& vbo_data_block = **(vbo_itr++);
      assert(vbo_data_block.isVBOReady());

      m_pContext->getMeshManager()->bindVBO(commandBuffer, &vbo_data_block, lodCoverage);

      int vertex_draw_count = cVertexes;
      if (vertex_draw_count > index_count - index_group_offset) vertex_draw_count = index_count - index_group_offset;

      vkCmdDrawIndexed(commandBuffer, vertex_draw_count, 1, index_group_offset, 0, 0);
      m_pContext->getMeshManager()->log_draw_call(renderPass->getType(), object_name, material_name, vertex_draw_count);
      cVertexes -= vertex_draw_count;
      index_group_offset = 0;
    }

  } else {
    int cBuffers = (cVertexes + MAX_VBO_SIZE - 1) / MAX_VBO_SIZE;
    int iVertex = mesh.start_vertex;
    int iBuffer = iVertex / MAX_VBO_SIZE;
    iVertex = iVertex % MAX_VBO_SIZE;
    while (cVertexes > 0) {
      int cBufferVertexes = iBuffer < cBuffers - 1 ? MAX_VBO_SIZE : cVertexes % MAX_VBO_SIZE;

      KRMeshManager::KRVBOData& vbo_data_block = **vbo_itr++;
      assert(vbo_data_block.isVBOReady());
      m_pContext->getMeshManager()->bindVBO(commandBuffer, &vbo_data_block, lodCoverage);


      if (iVertex + cVertexes >= MAX_VBO_SIZE) {
        assert(iVertex + (MAX_VBO_SIZE - iVertex) <= cBufferVertexes);
        if (getIndexCount(0) == 0) {
          vkCmdDraw(commandBuffer, (MAX_VBO_SIZE - iVertex), 1, iVertex, 0);
        } else {
          vkCmdDrawIndexed(commandBuffer, (MAX_VBO_SIZE - iVertex), 1, iVertex, 0, 0);
        }
        m_pContext->getMeshManager()->log_draw_call(renderPass->getType(), object_name, material_name, (MAX_VBO_SIZE - iVertex));

        cVertexes -= (MAX_VBO_SIZE - iVertex);
        iVertex = 0;
        iBuffer++;
      } else {
        assert(iVertex + cVertexes <= cBufferVertexes);

        if (getIndexCount(0) == 0) {
          vkCmdDraw(commandBuffer, cVertexes, 1, iVertex, 0);
        } else {
          vkCmdDrawIndexed(commandBuffer, cVertexes, 1, iVertex, 0, 0);
        }
        m_pContext->getMeshManager()->log_draw_call(renderPass->getType(), object_name, material_name, cVertexes);

        cVertexes = 0;
      }

    }
  }
}

void KRMesh::LoadData(const KRMesh::mesh_info& mi, bool calculate_normals, bool calculate_tangents)
{

  releaseData();

  // TODO, FINDME - These values should be passed as a parameter and set by GUI flags
  bool use_short_vertexes = false;
  bool use_short_normals = true;
  bool use_short_tangents = true;
  bool use_short_texcoord[8] = { false, false, false, false, false, false, false, false };

  if (use_short_vertexes) {
    for (std::vector<Vector3>::const_iterator itr = mi.vertices.begin(); itr != mi.vertices.end(); itr++) {
      if (fabsf((*itr).x) > 1.0f || fabsf((*itr).y) > 1.0f || fabsf((*itr).z) > 1.0f) {
        use_short_vertexes = false;
      }
    }
  }

  for (int set = 0; set < 8; set++) {
    if (use_short_texcoord[set]) {
      for (std::vector<Vector2>::const_iterator itr = mi.texcoord[set].begin(); itr != mi.texcoord[set].end(); itr++) {
        if (fabsf((*itr).x) > 1.0f || fabsf((*itr).y) > 1.0f) {
          use_short_texcoord[set] = false;
          break;
        }
      }
    }
  }

  PrimitiveInfo primitive = {};
  VertexAttributeInfo* attribute = primitive.layout.attributes;

  if (mi.vertices.size()) {
    attribute->attribute = VertexAttribute::position;
    attribute->type = DataType::vec3;
    if (use_short_vertexes) {
      attribute->component = ComponentType::float16;
      attribute->normalization = Normalization::normalized;
    } else {
      attribute->component = ComponentType::float32;
      attribute->normalization = Normalization::none;
    }
    attribute++;
  }
  if (mi.normals.size() || calculate_normals) {
    attribute->attribute = VertexAttribute::normal;
    attribute->type = DataType::vec3;
    if (use_short_normals) {
      attribute->component = ComponentType::float16;
      attribute->normalization = Normalization::normalized;
    } else {
      attribute->component = ComponentType::float32;
      attribute->normalization = Normalization::none;
    }
    attribute++;
  }
  if (mi.tangents.size() || calculate_tangents) {
    attribute->attribute = VertexAttribute::tangent;
    attribute->type = DataType::vec3;
    if (use_short_tangents) {
      attribute->component = ComponentType::float16;
      attribute->normalization = Normalization::normalized;
    } else {
      attribute->component = ComponentType::float32;
      attribute->normalization = Normalization::none;
    }
    attribute++;
  }
  for (int set = 0; set < 8; set++) {
    if (mi.texcoord[set].size()) {
      attribute->attribute = VertexAttribute::texcoord;
      attribute->type = DataType::vec2;
      if (use_short_texcoord[set]) {
        attribute->component = ComponentType::float16;
        attribute->normalization = Normalization::normalized;
      } else {
        attribute->component = ComponentType::float32;
        attribute->normalization = Normalization::none;
      }
      attribute++;
    }

    if (mi.color[set].size()) {
      attribute->attribute = VertexAttribute::texcoord;
      attribute->type = DataType::vec4;
      attribute->component = ComponentType::uint8;
      attribute->normalization = Normalization::normalized;
      attribute++;
    }
  }
  if (mi.bone_names.size()) {
    attribute->attribute = VertexAttribute::joints;
    attribute->type = DataType::vec4;
    attribute->component = ComponentType::uint8;
    attribute->normalization = Normalization::none;
    attribute++;

    attribute->attribute = VertexAttribute::joints;
    attribute->type = DataType::vec4;
    attribute->component = ComponentType::float32;
    attribute->normalization = Normalization::none;
    attribute++;
  }
  for (int i = 0; i < kMaxAttributes; i++) {
    primitive.layout.offsets[i] = primitive.layout.vertexSize;
    int componentSize = ComponentSize[(int)primitive.layout.attributes[i].component] * DataTypeComponentCount[(int)primitive.layout.attributes[i].type];
    primitive.layout.vertexSize += componentSize;
  }

  primitive.layout.topology = mi.format;
  primitive.vertexCount = mi.vertices.size();
  primitive.indexCount = mi.vertex_indexes.size();

  size_t index_count = mi.vertex_indexes.size();
  size_t index_base_count = mi.vertex_index_bases.size();
  size_t submesh_count = mi.submesh_lengths.size();
  size_t bone_count = mi.bone_names.size();
  size_t new_file_size = sizeof(pack_header) + sizeof(pack_material) * submesh_count + sizeof(pack_bone) * bone_count + KRALIGN(2 * index_count) + KRALIGN(8 * index_base_count) + primitive.layout.vertexSize * primitive.vertexCount;
  m_pData = new Block();
  m_pMetaData = m_pData;
  m_pData->expand(new_file_size);
  m_pData->lock();
  pack_header* pHeader = getHeader();
  memset(pHeader, 0, sizeof(pack_header));
  memcpy(&pHeader->primitive, &primitive, sizeof(PrimitiveInfo));
  pHeader->submesh_count = (__int32_t)submesh_count;
  pHeader->bone_count = (__int32_t)bone_count;
  pHeader->index_base_count = (__int32_t)index_base_count;
  strcpy(pHeader->szTag, "KRMESH1.0      ");

  pack_material* pPackMaterials = (pack_material*)(pHeader + 1);

  for (int iMaterial = 0; iMaterial < pHeader->submesh_count; iMaterial++) {
    pack_material* pPackMaterial = pPackMaterials + iMaterial;
    pPackMaterial->start_vertex = mi.submesh_starts[iMaterial];
    pPackMaterial->vertex_count = mi.submesh_lengths[iMaterial];
    memset(pPackMaterial->szName, 0, KRENGINE_MAX_NAME_LENGTH);
    strncpy(pPackMaterial->szName, mi.material_names[iMaterial].c_str(), KRENGINE_MAX_NAME_LENGTH);
  }

  for (int bone_index = 0; bone_index < bone_count; bone_index++) {
    pack_bone* bone = getBone(bone_index);
    memset(bone->szName, 0, KRENGINE_MAX_NAME_LENGTH);
    strncpy(bone->szName, mi.bone_names[bone_index].c_str(), KRENGINE_MAX_NAME_LENGTH);
    memcpy(bone->bind_pose, mi.bone_bind_poses[bone_index].c, sizeof(float) * 16);
  }

  bool bFirstVertex = true;
  int vertex_size = (int)getHeader()->primitive.layout.vertexSize;
  memset(getVertexData(), 0, vertex_size * (int)mi.vertices.size());
  for (int iVertex = 0; iVertex < (int)mi.vertices.size(); iVertex++) {
    Vector3 source_vertex = mi.vertices[iVertex];
    setVertexPosition(iVertex, source_vertex);
    if (mi.bone_names.size()) {
      hydra::Vector4 weights = hydra::Vector4::Zero();
      for (int bone_weight_index = 0; bone_weight_index < KRENGINE_MAX_BONE_WEIGHTS_PER_VERTEX; bone_weight_index++) {
        setBoneIndex(iVertex, bone_weight_index, mi.bone_indexes[iVertex][bone_weight_index]);
        setBoneWeight(iVertex, bone_weight_index, mi.bone_weights[iVertex][bone_weight_index]);
      }
    }
    if (bFirstVertex) {
      bFirstVertex = false;
      m_extents.min = source_vertex;
      m_extents.max = source_vertex;
    } else {
      m_extents.encapsulate(source_vertex);
    }
    for (int set = 0; set < 8; set++) {
      if ((int)mi.texcoord[set].size() > iVertex) {
        setVertexTexCoord(iVertex, set, mi.texcoord[set][iVertex]);
      }

      if ((int)mi.color[set].size() > iVertex) {
        setVertexColor(iVertex, set, mi.color[set][iVertex]);
      }
    }
    if ((int)mi.normals.size() > iVertex) {
      setVertexNormal(iVertex, Vector3::Normalize(mi.normals[iVertex]));
    }
    if ((int)mi.tangents.size() > iVertex) {
      setVertexTangent(iVertex, Vector3::Normalize(mi.tangents[iVertex]));
    }
  }

  pHeader->extents = m_extents;

  __uint16_t* index_data = getIndexData();
  for (std::vector<__uint16_t>::const_iterator itr = mi.vertex_indexes.begin(); itr != mi.vertex_indexes.end(); itr++) {
    *index_data++ = *itr;
  }

  __uint32_t* index_base_data = getIndexBaseData();
  for (std::vector<std::pair<int, int> >::const_iterator itr = mi.vertex_index_bases.begin(); itr != mi.vertex_index_bases.end(); itr++) {
    *index_base_data++ = (*itr).first;
    *index_base_data++ = (*itr).second;
  }

  auto calculateTriangleAttributes = [this, calculate_normals, calculate_tangents](int i0, int i1, int i2) {
    Vector3 p1 = getVertexPosition(i0);
    Vector3 p2 = getVertexPosition(i1);
    Vector3 p3 = getVertexPosition(i2);
    Vector3 v1 = p2 - p1;
    Vector3 v2 = p3 - p1;

    // -- Calculate normal if missing --
    if (calculate_normals) {
      Vector3 first_normal = getVertexNormal(i0);
      if (first_normal.x == 0.0f && first_normal.y == 0.0f && first_normal.z == 0.0f) {
        // Note - We don't take into consideration smoothing groups or smoothing angles when generating normals; all generated normals represent flat shaded polygons
        Vector3 normal = Vector3::Cross(v1, v2);

        normal.normalize();
        setVertexNormal(i0, normal);
        setVertexNormal(i1, normal);
        setVertexNormal(i2, normal);
      }
    }

    // -- Calculate tangent vector for normal mapping --
    if (calculate_tangents) {
      Vector3 first_tangent = getVertexTangent(i0);
      if (first_tangent.x == 0.0f && first_tangent.y == 0.0f && first_tangent.z == 0.0f) {

        Vector2 uv0 = getVertexTexCoord(0, i0);
        Vector2 uv1 = getVertexTexCoord(0, i1);
        Vector2 uv2 = getVertexTexCoord(0, i2);

        Vector2 st1 = Vector2::Create(uv1.x - uv0.x, uv1.y - uv0.y);
        Vector2 st2 = Vector2::Create(uv2.x - uv0.x, uv2.y - uv0.y);
        float coef = 1 / (st1.x * st2.y - st2.x * st1.y);

        Vector3 tangent = Vector3::Create(
                          coef * ((v1.x * st2.y) + (v2.x * -st1.y)),
                          coef * ((v1.y * st2.y) + (v2.y * -st1.y)),
                          coef * ((v1.z * st2.y) + (v2.z * -st1.y))
        );

        tangent.normalize();
        setVertexTangent(i0, tangent);
        setVertexTangent(i1, tangent);
        setVertexTangent(i2, tangent);
      }
    }
  };

  // Calculate missing surface normals and tangents
  if (calculate_normals || calculate_tangents) {
    switch (getTopology()) {
    case Topology::Triangles:
    {
      // NOTE: This will not work properly if the vertices are already indexed
      for (int iVertex = 0; iVertex + 2 < (int)mi.vertices.size(); iVertex += 3) {
        calculateTriangleAttributes(iVertex, iVertex + 1, iVertex + 2);
      }
      break;
    }
    case Topology::TriangleStrips:
    {
      // NOTE: This will not work properly if the vertices are already indexed
      for (int iVertex = 0; iVertex + 2 < (int)mi.vertices.size(); iVertex++) {
        calculateTriangleAttributes(iVertex, iVertex + 1, iVertex + 2);
      }
      break;
    }
    case Topology::TriangleFans:
    {
      // NOTE: This will not work properly if the vertices are already indexed
      for (int iVertex = 1; iVertex + 1 < (int)mi.vertices.size(); iVertex++) {
        calculateTriangleAttributes(0, iVertex, iVertex + 1);
      }
      break;
    }
    default:
      assert(false); // Not Supported
    } // switch
  }
  m_pData->unlock();

  // ----

  pack_header ph;
  m_pData->copy((void*)&ph, 0, sizeof(ph));
  m_pMetaData = m_pData->getSubBlock(0, sizeof(pack_header) + sizeof(pack_material) * ph.submesh_count + sizeof(pack_bone) * ph.bone_count);
  m_pMetaData->lock();
  m_pIndexBaseData = m_pData->getSubBlock(sizeof(pack_header) + sizeof(pack_material) * ph.submesh_count + sizeof(pack_bone) * ph.bone_count + KRALIGN(2 * ph.primitive.indexCount), ph.index_base_count * 8);
  m_pIndexBaseData->lock();

  // ----

  optimize();

  if (m_constant) {
    // Ensure that constant models loaded immediately by the streamer
    getSubmeshes();
    getMaterials();
  }
}

const AABB& KRMesh::getExtents() const
{
  return m_extents;
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

int KRMesh::getAttributeIndex(const PrimitiveInfo& primitive, VertexAttribute attribute, int index)
{
  int indexLeft = index;
  for (int i = 0; i < kMaxAttributes; i++) {
    VertexAttributeInfo info = primitive.layout.attributes[i];
    if (info.attribute == VertexAttribute::position) {
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

KRMesh::pack_bone* KRMesh::getBone(int index)
{
  pack_header* header = getHeader();
  return (pack_bone*)((unsigned char*)m_pMetaData->getStart() + sizeof(pack_header) + sizeof(pack_material) * header->submesh_count + sizeof(pack_bone) * index);
}

unsigned char* KRMesh::getVertexData() const
{
  return ((unsigned char*)m_pData->getStart()) + getVertexDataOffset();
}

size_t KRMesh::getVertexDataOffset() const
{
  pack_header* pHeader = getHeader();
  return sizeof(pack_header) + sizeof(pack_material) * pHeader->submesh_count + sizeof(pack_bone) * pHeader->bone_count + KRALIGN(2 * pHeader->primitive.indexCount) + KRALIGN(8 * pHeader->index_base_count);
}

__uint16_t* KRMesh::getIndexData() const
{

  return (__uint16_t*)((unsigned char*)m_pData->getStart() + getIndexDataOffset());
}

size_t KRMesh::getIndexDataOffset() const
{
  pack_header* pHeader = getHeader();
  return sizeof(pack_header) + sizeof(pack_material) * pHeader->submesh_count + sizeof(pack_bone) * pHeader->bone_count;
}

__uint32_t* KRMesh::getIndexBaseData() const
{
  if (m_pIndexBaseData == NULL) {
    pack_header* pHeader = getHeader();
    return (__uint32_t*)((unsigned char*)m_pData->getStart() + sizeof(pack_header) + sizeof(pack_material) * pHeader->submesh_count + sizeof(pack_bone) * pHeader->bone_count + KRALIGN(2 * pHeader->primitive.indexCount));
  } else {
    return (__uint32_t*)m_pIndexBaseData->getStart();
  }
}


KRMesh::pack_material* KRMesh::getSubmesh(int mesh_index) const
{
  return (pack_material*)((unsigned char*)m_pMetaData->getStart() + sizeof(pack_header)) + mesh_index;
}

unsigned char* KRMesh::getVertexData(int index) const
{
  int vertex_size = (int)getHeader()->primitive.layout.vertexSize;
  return getVertexData() + vertex_size * index;
}

int KRMesh::getSubmeshCount() const
{
  pack_header* header = getHeader();
  int submesh_count = header->submesh_count;
  return submesh_count;
}

int KRMesh::getVertexCount(int submesh) const
{
  return getSubmesh(submesh)->vertex_count;
}

int KRMesh::getIndexCount(int submesh) const
{
  pack_header* pHeader = getHeader();
  if (pHeader->primitive.indexCount == 0) {
    return 0;
  }
  int index_group = getSubmesh(submesh)->index_group;

  int start_index_offset, start_vertex_offset, index_count, vertex_count;
  getIndexedRange(index_group, start_index_offset, start_vertex_offset, index_count, vertex_count);

  return index_count;
}

const VertexBufferLayout* KRMesh::getLayout(int submesh) const
{
  return &getHeader()->primitive.layout;
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
      float clampedFloat = std::clamp(val, minFloat, maxFloat);
      *out = static_cast<T>(clampedFloat);
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

void writeVertexAttributeComponent(const VertexAttributeInfo& attribute, void* address, int componentIndex, float val)
{
  void* componentAddress = (uint8_t*)address + ComponentSize[(int)attribute.component] * componentIndex;
  switch (attribute.component) {
  case ComponentType::empty:
    break;
  case ComponentType::int8:
    normalizeAttributeComponent(attribute.normalization, val, (__int8_t*)address);
    break;
  case ComponentType::uint8:
    normalizeAttributeComponent(attribute.normalization, val, (__uint8_t*)address);
    break;
  case ComponentType::int16:
    normalizeAttributeComponent(attribute.normalization, val, (__int16_t*)address);
    break;
  case ComponentType::uint16:
    normalizeAttributeComponent(attribute.normalization, val, (__uint16_t*)address);
    break;
  case ComponentType::int32:
    normalizeAttributeComponent(attribute.normalization, val, (__int32_t*)address);
    break;
  case ComponentType::uint32:
    normalizeAttributeComponent(attribute.normalization, val, (__uint32_t*)address);
    break;
  case ComponentType::int64:
    normalizeAttributeComponent(attribute.normalization, val, (__int64_t*)address);
    break;
  case ComponentType::uint64:
    normalizeAttributeComponent(attribute.normalization, val, (__uint64_t*)address);
    break;
  case ComponentType::float16:
    normalizeAttributeComponent(attribute.normalization, val, (short*)address);
    break;
  case ComponentType::float32:
    normalizeAttributeComponent(attribute.normalization, val, (float*)address);
    break;
  case ComponentType::float64:
    normalizeAttributeComponent(attribute.normalization, val, (double*)address);
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
    denormalizeAttributeComponent(attribute.normalization, *(__int8_t*)address, out);
    break;
  case ComponentType::uint8:
    denormalizeAttributeComponent(attribute.normalization, *(__uint8_t*)address, out);
    break;
  case ComponentType::int16:
    denormalizeAttributeComponent(attribute.normalization, *(__int16_t*)address, out);
    break;
  case ComponentType::uint16:
    denormalizeAttributeComponent(attribute.normalization, *(__uint16_t*)address, out);
    break;
  case ComponentType::int32:
    denormalizeAttributeComponent(attribute.normalization, *(__int32_t*)address, out);
    break;
  case ComponentType::uint32:
    denormalizeAttributeComponent(attribute.normalization, *(__uint32_t*)address, out);
    break;
  case ComponentType::int64:
    denormalizeAttributeComponent(attribute.normalization, *(__int64_t*)address, out);
    break;
  case ComponentType::uint64:
    denormalizeAttributeComponent(attribute.normalization, *(__uint64_t*)address, out);
    break;
  case ComponentType::float16:
    denormalizeAttributeComponent(attribute.normalization, *(short*)address, out);
    break;
  case ComponentType::float32:
    denormalizeAttributeComponent(attribute.normalization, *(float*)address, out);
    break;
  case ComponentType::float64:
    denormalizeAttributeComponent(attribute.normalization, *(double*)address, out);
    break;
  }
}

void KRMesh::setVertexAttribute(int vertexIndex, int attribIndex, float val)
{
  const PrimitiveInfo& primitive = getHeader()->primitive;
  const VertexAttributeInfo& attribute = primitive.layout.attributes[attribIndex];
  void* address = getVertexData(vertexIndex) + primitive.layout.offsets[attribIndex];

  if (attribute.type != DataType::scalar)
  {
    assert(false);
    return;
  }

  writeVertexAttributeComponent(attribute, address, 0, val);
}

void KRMesh::getVertexAttribute(int vertexIndex, int attribIndex, float* val) const
{
  const PrimitiveInfo& primitive = getHeader()->primitive;
  const VertexAttributeInfo& attribute = primitive.layout.attributes[attribIndex];
  void* address = getVertexData(vertexIndex) + primitive.layout.offsets[attribIndex];

  if (attribute.type != DataType::scalar) {
    assert(false);
    return;
  }

  readVertexAttributeComponent(attribute, address, 0, val);
}

void KRMesh::setVertexAttribute(int vertexIndex, int attribIndex, Vector2 val)
{
  const PrimitiveInfo& primitive = getHeader()->primitive;
  const VertexAttributeInfo& attribute = primitive.layout.attributes[attribIndex];
  void* address = getVertexData(vertexIndex) + primitive.layout.offsets[attribIndex];

  if (attribute.type != DataType::vec2) {
    assert(false);
    return;
  }

  writeVertexAttributeComponent(attribute, address, 0, val.x);
  writeVertexAttributeComponent(attribute, address, 1, val.y);
}

void KRMesh::getVertexAttribute(int vertexIndex, int attribIndex, Vector2* val) const
{
  const PrimitiveInfo& primitive = getHeader()->primitive;
  const VertexAttributeInfo& attribute = primitive.layout.attributes[attribIndex];
  void* address = getVertexData(vertexIndex) + primitive.layout.offsets[attribIndex];

  if (attribute.type != DataType::vec2) {
    assert(false);
    return;
  }

  readVertexAttributeComponent(attribute, address, 0, &val->x);
  readVertexAttributeComponent(attribute, address, 1, &val->y);
}

void KRMesh::setVertexAttribute(int vertexIndex, int attribIndex, Vector3 val)
{
  const PrimitiveInfo& primitive = getHeader()->primitive;
  const VertexAttributeInfo& attribute = primitive.layout.attributes[attribIndex];
  void* address = getVertexData(vertexIndex) + primitive.layout.offsets[attribIndex];

  if (attribute.type != DataType::vec3) {
    assert(false);
    return;
  }

  writeVertexAttributeComponent(attribute, address, 0, val.x);
  writeVertexAttributeComponent(attribute, address, 1, val.y);
  writeVertexAttributeComponent(attribute, address, 2, val.z);
}

void KRMesh::getVertexAttribute(int vertexIndex, int attribIndex, Vector3* val) const
{
  const PrimitiveInfo& primitive = getHeader()->primitive;
  const VertexAttributeInfo& attribute = primitive.layout.attributes[attribIndex];
  void* address = getVertexData(vertexIndex) + primitive.layout.offsets[attribIndex];

  if (attribute.type != DataType::vec3) {
    assert(false);
    return;
  }

  readVertexAttributeComponent(attribute, address, 0, &val->x);
  readVertexAttributeComponent(attribute, address, 1, &val->y);
  readVertexAttributeComponent(attribute, address, 2, &val->z);
}

void KRMesh::setVertexAttribute(int vertexIndex, int attribIndex, Vector4 val)
{
  const PrimitiveInfo& primitive = getHeader()->primitive;
  const VertexAttributeInfo& attribute = primitive.layout.attributes[attribIndex];
  void* address = getVertexData(vertexIndex) + primitive.layout.offsets[attribIndex];

  if (attribute.type != DataType::vec4) {
    assert(false);
    return;
  }

  writeVertexAttributeComponent(attribute, address, 0, val.x);
  writeVertexAttributeComponent(attribute, address, 1, val.y);
  writeVertexAttributeComponent(attribute, address, 2, val.z);
  writeVertexAttributeComponent(attribute, address, 3, val.w);
}

void KRMesh::getVertexAttribute(int vertexIndex, int attribIndex, Vector4* val) const
{
  const PrimitiveInfo& primitive = getHeader()->primitive;
  const VertexAttributeInfo& attribute = primitive.layout.attributes[attribIndex];
  void* address = getVertexData(vertexIndex) + primitive.layout.offsets[attribIndex];

  if (attribute.type != DataType::vec4) {
    assert(false);
    return;
  }

  readVertexAttributeComponent(attribute, address, 0, &val->x);
  readVertexAttributeComponent(attribute, address, 1, &val->y);
  readVertexAttributeComponent(attribute, address, 2, &val->z);
  readVertexAttributeComponent(attribute, address, 3, &val->w);
}

void KRMesh::setVertexAttribute(int vertexIndex, int attribIndex, Matrix2 val)
{
  const PrimitiveInfo& primitive = getHeader()->primitive;
  const VertexAttributeInfo& attribute = primitive.layout.attributes[attribIndex];
  void* address = getVertexData(vertexIndex) + primitive.layout.offsets[attribIndex];

  if (attribute.type != DataType::mat2) {
    assert(false);
    return;
  }

  writeVertexAttributeComponent(attribute, address, 0, val.c[0]);
  writeVertexAttributeComponent(attribute, address, 1, val.c[1]);
  writeVertexAttributeComponent(attribute, address, 2, val.c[2]);
  writeVertexAttributeComponent(attribute, address, 3, val.c[3]);
}

void KRMesh::getVertexAttribute(int vertexIndex, int attribIndex, Matrix2* val) const
{
  const PrimitiveInfo& primitive = getHeader()->primitive;
  const VertexAttributeInfo& attribute = primitive.layout.attributes[attribIndex];
  void* address = getVertexData(vertexIndex) + primitive.layout.offsets[attribIndex];

  if (attribute.type != DataType::mat2) {
    assert(false);
    return;
  }

  readVertexAttributeComponent(attribute, address, 0, &val->c[0]);
  readVertexAttributeComponent(attribute, address, 1, &val->c[1]);
  readVertexAttributeComponent(attribute, address, 2, &val->c[2]);
  readVertexAttributeComponent(attribute, address, 3, &val->c[3]);
}

void KRMesh::setVertexAttribute(int vertexIndex, int attribIndex, Matrix4 val)
{
  const PrimitiveInfo& primitive = getHeader()->primitive;
  const VertexAttributeInfo& attribute = primitive.layout.attributes[attribIndex];
  void* address = getVertexData(vertexIndex) + primitive.layout.offsets[attribIndex];

  if (attribute.type != DataType::mat4) {
    assert(false);
    return;
  }

  for (int i = 0; i < 16; i++) {
    writeVertexAttributeComponent(attribute, address, i, val.c[i]);
  }
}

void KRMesh::getVertexAttribute(int vertexIndex, int attribIndex, Matrix4* val) const
{
  const PrimitiveInfo& primitive = getHeader()->primitive;
  const VertexAttributeInfo& attribute = primitive.layout.attributes[attribIndex];
  void* address = getVertexData(vertexIndex) + primitive.layout.offsets[attribIndex];

  if (attribute.type != DataType::mat4) {
    assert(false);
    return;
  }

  for (int i = 0; i < 16; i++) {
    readVertexAttributeComponent(attribute, address, i, &val->c[i]);
  }
}

Vector3 KRMesh::getVertexPosition(int index) const
{
  int attribIndex = getAttributeIndex(getHeader()->primitive, VertexAttribute::position, 0);
  if (attribIndex == -1) {
    return Vector3::Zero();
  }
  Vector3 v;
  getVertexAttribute(index, attribIndex, &v);
  return v;
}

Vector3 KRMesh::getVertexNormal(int index) const
{
  int attribIndex = getAttributeIndex(getHeader()->primitive, VertexAttribute::normal, 0);
  if (attribIndex == -1) {
    return Vector3::Zero();
  }
  Vector3 v;
  getVertexAttribute(index, attribIndex, &v);
  return v;
}

Vector3 KRMesh::getVertexTangent(int index) const
{
  int attribIndex = getAttributeIndex(getHeader()->primitive, VertexAttribute::tangent, 0);
  if (attribIndex == -1) {
    return Vector3::Zero();
  }
  Vector3 v;
  getVertexAttribute(index, attribIndex, &v);
  return v;
}

Vector2 KRMesh::getVertexTexCoord(int set, int index) const
{
  int attribIndex = getAttributeIndex(getHeader()->primitive, VertexAttribute::texcoord, set);
  if (attribIndex == -1) {
    return Vector2::Zero();
  }
  Vector2 v;
  getVertexAttribute(index, attribIndex, &v);
  return v;
}

Vector4 KRMesh::getVertexColor(int set, int index) const
{
  int attribIndex = getAttributeIndex(getHeader()->primitive, VertexAttribute::color, set);
  if (attribIndex == -1) {
    return Vector4::Zero();
  }
  Vector4 v;
  getVertexAttribute(index, attribIndex, &v);
  return v;
}

void KRMesh::setVertexPosition(int index, const Vector3& v)
{
  int attribIndex = getAttributeIndex(getHeader()->primitive, VertexAttribute::position, 0);
  if (attribIndex == -1) {
    return;
  }

  setVertexAttribute(index, attribIndex, v);
}

void KRMesh::setVertexNormal(int index, const Vector3& v)
{
  int attribIndex = getAttributeIndex(getHeader()->primitive, VertexAttribute::normal, 0);
  if (attribIndex == -1) {
    return;
  }

  setVertexAttribute(index, attribIndex, v);
}

void KRMesh::setVertexTangent(int index, const Vector3& v)
{
  int attribIndex = getAttributeIndex(getHeader()->primitive, VertexAttribute::tangent, 0);
  if (attribIndex == -1) {
    return;
  }

  setVertexAttribute(index, attribIndex, v);
}

void KRMesh::setVertexTexCoord(int index, int set, const Vector2& v)
{
  int attribIndex = getAttributeIndex(getHeader()->primitive, VertexAttribute::texcoord, set);
  if (attribIndex == -1) {
    return;
  }

  setVertexAttribute(index, attribIndex, v);
}

void KRMesh::setVertexColor(int index, int set, const Vector4& v)
{
  int attribIndex = getAttributeIndex(getHeader()->primitive, VertexAttribute::color, set);
  if (attribIndex == -1) {
    return;
  }

  setVertexAttribute(index, attribIndex, v);
}

int KRMesh::getBoneIndex(int index, int weight_index) const
{
  int attribIndex = getAttributeIndex(getHeader()->primitive, VertexAttribute::joints, weight_index / 4);
  if (attribIndex == -1) {
    return 0;
  }

  // TODO - Implement integer based attribute access
  Vector4 v;
  getVertexAttribute(index, attribIndex, &v);
  return static_cast<int>(v[weight_index % 4]);
}

void KRMesh::setBoneIndex(int index, int weight_index, int bone_index)
{
  int attribIndex = getAttributeIndex(getHeader()->primitive, VertexAttribute::joints, weight_index / 4);
  if (attribIndex == -1) {
    return;
  }

  // TODO - Implement integer based attribute access
  Vector4 v;
  getVertexAttribute(index, attribIndex, &v);
  v[weight_index % 4] = bone_index;
  setVertexAttribute(index, attribIndex, v);
}

void KRMesh::setBoneWeight(int index, int bone_index, float weight)
{
  int attribIndex = getAttributeIndex(getHeader()->primitive, VertexAttribute::weights, bone_index / 4);
  if (attribIndex == -1) {
    return;
  }
  hydra::Vector4 v;
  getVertexAttribute(index, attribIndex, &v);
  v[bone_index % 4] = weight;
  setVertexAttribute(index, attribIndex, v);
}

float KRMesh::getBoneWeight(int index, int weight_index) const
{
  int attribIndex = getAttributeIndex(getHeader()->primitive, VertexAttribute::weights, weight_index / 4);
  if (attribIndex == -1) {
    return 0.f;
  }

  Vector4 v;
  getVertexAttribute(index, attribIndex, &v);
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

Topology KRMesh::getTopology() const
{
  return getHeader()->primitive.layout.topology;
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
    int vertex_count = getVertexCount(submesh_index);
    switch (getTopology()) {
    case Topology::Triangles:
      for (int triangle_index = 0; triangle_index < vertex_count / 3; triangle_index++) {
        int tri_vert_index[3]; // FINDME, HACK!  This is not very efficient for indexed collider meshes...
        tri_vert_index[0] = getVertexIndex(submesh_index, triangle_index * 3);
        tri_vert_index[1] = getVertexIndex(submesh_index, triangle_index * 3 + 1);
        tri_vert_index[2] = getVertexIndex(submesh_index, triangle_index * 3 + 2);

        Triangle3 tri = Triangle3::Create(getVertexPosition(tri_vert_index[0]), getVertexPosition(tri_vert_index[1]), getVertexPosition(tri_vert_index[2]));

        if (rayCast(start, dir, tri, getVertexNormal(tri_vert_index[0]), getVertexNormal(tri_vert_index[1]), getVertexNormal(tri_vert_index[2]), hitinfo)) hit_found = true;
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
    int vertex_count = getVertexCount(submesh_index);
    switch (getTopology()) {
      case Topology::Triangles:
      for (int triangle_index = 0; triangle_index < vertex_count / 3; triangle_index++) {
        int tri_vert_index[3]; // FINDME, HACK!  This is not very efficient for indexed collider meshes...
        tri_vert_index[0] = getVertexIndex(submesh_index, triangle_index * 3);
        tri_vert_index[1] = getVertexIndex(submesh_index, triangle_index * 3 + 1);
        tri_vert_index[2] = getVertexIndex(submesh_index, triangle_index * 3 + 2);

        Triangle3 tri = Triangle3::Create(getVertexPosition(tri_vert_index[0]), getVertexPosition(tri_vert_index[1]), getVertexPosition(tri_vert_index[2]));

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
    getSubmeshes();
    getMaterials();
  }
}

void KRMesh::optimize()
{
  if (getIndexCount(0) > 0) {
    optimizeIndexes();
  } else {
    convertToIndexed(); // HACK, FINDME, TODO - This may not be ideal in every case and should be exposed through the API independently
  }
}

void KRMesh::getIndexedRange(int index_group, int& start_index_offset, int& start_vertex_offset, int& index_count, int& vertex_count) const
{
  pack_header* h = getHeader();
  __uint32_t* index_base_data = getIndexBaseData();
  start_index_offset = index_base_data[index_group * 2];
  start_vertex_offset = index_base_data[index_group * 2 + 1];
  if (index_group + 1 < h->index_base_count) {
    index_count = index_base_data[index_group * 2 + 2] - start_index_offset;
    vertex_count = index_base_data[index_group * 2 + 3] - start_vertex_offset;
  } else {
    index_count = h->primitive.indexCount - start_index_offset;
    vertex_count = h->primitive.vertexCount - start_vertex_offset;
  }
}

int KRMesh::getVertexIndex(int submesh, int index) const
{
  if (getIndexCount(submesh) > 0) {
    __uint16_t* index_data = getIndexData();

    int start_index_offset, start_vertex_offset, index_count, vertex_count;
    int index_group = getSubmesh(submesh)->index_group;
    int index_group_offset = getSubmesh(submesh)->index_group_offset;
    int remaining_vertices = index_group_offset + index;
    getIndexedRange(index_group++, start_index_offset, start_vertex_offset, index_count, vertex_count);
    while (remaining_vertices >= index_count) {
      remaining_vertices -= index_count;
      getIndexedRange(index_group++, start_index_offset, start_vertex_offset, index_count, vertex_count);
    }
    return index_data[start_index_offset + remaining_vertices] + start_vertex_offset;
  } else {
    return getSubmesh(submesh)->start_vertex + index;
  }
}

void KRMesh::optimizeIndexes()
{
  // TODO - Re-enable this once crash with KRMeshSphere vertices is corrected
  return;

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

        /*

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
         */



        index_group_offset = 0;
      }
    }

    free(new_indices);
    free(vertex_mapping);
    free(new_vertex_data);
  } // getTopology() == Topology::Triangles && getIndexCount(0) > 0

  m_pData->unlock();
}
