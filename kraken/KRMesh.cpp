//
//  KRMesh.cpp
//  Kraken Engine
//
//  Copyright 2024 Kearwood Gilbert. All rights reserved.
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

#include "KRMesh.h"

#include "KRPipeline.h"
#include "KRPipelineManager.h"
#include "KRContext.h"
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


void KRMesh::parseName(const std::string& name, std::string& lodBaseName, int& lodCoverage)
{
  lodCoverage = 100;
  lodBaseName = name;

  size_t last_underscore_pos = name.find_last_of('_');
  if (last_underscore_pos != std::string::npos) {
    // Found an underscore
    std::string suffix = name.substr(last_underscore_pos + 1);
    if (suffix.find("lod") == 0) {
      std::string lod_level_string = suffix.substr(3);
      char* end = NULL;
      int c = (int)strtol(lod_level_string.c_str(), &end, 10);
      if (c >= 0 && c <= 100 && *end == '\0') {
        lodCoverage = c;
        lodBaseName = name.substr(0, last_underscore_pos);
      }
    }
  }
}

void KRMesh::setName(const std::string name)
{
  parseName(name, m_lodBaseName, m_lodCoverage);
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

void KRMesh::releaseData()
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
  if (m_pData) {
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

  m_pIndexBaseData = m_pData->getSubBlock(sizeof(pack_header) + sizeof(pack_material) * ph.submesh_count + sizeof(pack_bone) * ph.bone_count + KRALIGN(2 * ph.index_count), ph.index_base_count * 8);
  m_pIndexBaseData->lock();

  m_minPoint = Vector3::Create(ph.minx, ph.miny, ph.minz);
  m_maxPoint = Vector3::Create(ph.maxx, ph.maxy, ph.maxz);

  updateAttributeOffsets();
}

void KRMesh::getMaterials()
{
  if (m_materials.size() == 0) {

    for (std::vector<KRMesh::Submesh>::iterator itr = m_submeshes.begin(); itr != m_submeshes.end(); itr++) {
      const char* szMaterialName = (*itr).szMaterialName;
      KRMaterial* pMaterial = nullptr;
      if (*szMaterialName != '\0') {
        pMaterial = getContext().getMaterialManager()->getMaterial(szMaterialName);
      }
      m_materials.push_back(pMaterial);
      if (pMaterial) {
        m_uniqueMaterials.insert(pMaterial);
      } else if (*szMaterialName != '\0') {
        KRContext::Log(KRContext::LOG_LEVEL_WARNING, "Missing material: %s", szMaterialName);
      }
    }

    m_hasTransparency = false;
    for (std::set<KRMaterial*>::iterator mat_itr = m_uniqueMaterials.begin(); mat_itr != m_uniqueMaterials.end(); mat_itr++) {
      if ((*mat_itr)->isTransparent()) {
        m_hasTransparency = true;
        break;
      }
    }
  }
}

void KRMesh::preStream(float lodCoverage)
{
  getSubmeshes();
  getMaterials();

  for (std::set<KRMaterial*>::iterator mat_itr = m_uniqueMaterials.begin(); mat_itr != m_uniqueMaterials.end(); mat_itr++) {
    (*mat_itr)->preStream(lodCoverage);
  }

  for (Submesh& mesh : m_submeshes) {
    for (shared_ptr<KRMeshManager::KRVBOData>& vbo : mesh.vbo_data_blocks) {
      vbo->resetPoolExpiry(lodCoverage);
    }
  }
}

kraken_stream_level KRMesh::getStreamLevel()
{
  kraken_stream_level stream_level = kraken_stream_level::STREAM_LEVEL_IN_HQ;
  getSubmeshes();
  getMaterials();

  for (std::set<KRMaterial*>::iterator mat_itr = m_uniqueMaterials.begin(); mat_itr != m_uniqueMaterials.end(); mat_itr++) {
    stream_level = KRMIN(stream_level, (*mat_itr)->getStreamLevel());
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


void KRMesh::render(const KRNode::RenderInfo& ri, const std::string& object_name, const Matrix4& matModel, KRTexture* pLightMap, const std::vector<KRBone*>& bones, const Vector3& rim_color, float rim_power, float lod_coverage)
{
  //fprintf(stderr, "Rendering model: %s\n", m_name.c_str());
  if (ri.renderPass != KRNode::RENDER_PASS_ADDITIVE_PARTICLES && ri.renderPass != KRNode::RENDER_PASS_PARTICLE_OCCLUSION && ri.renderPass != KRNode::RENDER_PASS_VOLUMETRIC_EFFECTS_ADDITIVE) {
    preStream(lod_coverage);
    if (getStreamLevel() == kraken_stream_level::STREAM_LEVEL_OUT) {

    } else {

      getSubmeshes();
      getMaterials();

      int cSubmeshes = (int)m_submeshes.size();
      if (ri.renderPass == KRNode::RENDER_PASS_SHADOWMAP) {
        for (int iSubmesh = 0; iSubmesh < cSubmeshes; iSubmesh++) {
          KRMaterial* pMaterial = m_materials[iSubmesh];

          if (pMaterial != NULL) {

            if (!pMaterial->isTransparent()) {
              // Exclude transparent and semi-transparent meshes from shadow maps
              renderSubmesh(ri.commandBuffer, iSubmesh, ri.renderPass, object_name, pMaterial->getName(), lod_coverage);
            }
          }

        }
      } else {
        // Apply submeshes in per-material batches to reduce number of state changes
        for (std::set<KRMaterial*>::iterator mat_itr = m_uniqueMaterials.begin(); mat_itr != m_uniqueMaterials.end(); mat_itr++) {
          for (int iSubmesh = 0; iSubmesh < cSubmeshes; iSubmesh++) {
            KRMaterial* pMaterial = m_materials[iSubmesh];

            if (pMaterial != NULL && pMaterial == (*mat_itr)) {
              if ((!pMaterial->isTransparent() && ri.renderPass != KRNode::RENDER_PASS_FORWARD_TRANSPARENT) || (pMaterial->isTransparent() && ri.renderPass == KRNode::RENDER_PASS_FORWARD_TRANSPARENT)) {
                std::vector<Matrix4> bone_bind_poses;
                for (int i = 0; i < (int)bones.size(); i++) {
                  bone_bind_poses.push_back(getBoneBindPose(i));
                }


                switch (pMaterial->getAlphaMode()) {
                case KRMaterial::KRMATERIAL_ALPHA_MODE_OPAQUE: // Non-transparent materials
                case KRMaterial::KRMATERIAL_ALPHA_MODE_TEST: // Alpha in diffuse texture is interpreted as punch-through when < 0.5
                  pMaterial->bind(ri, getModelFormat(), getVertexAttributes(), CullMode::kCullBack, bones, bone_bind_poses, matModel, pLightMap, rim_color, rim_power, lod_coverage);
                  renderSubmesh(ri.commandBuffer, iSubmesh, ri.renderPass, object_name, pMaterial->getName(), lod_coverage);
                  break;
                case KRMaterial::KRMATERIAL_ALPHA_MODE_BLENDONESIDE: // Blended alpha with backface culling
                  pMaterial->bind(ri, getModelFormat(), getVertexAttributes(), CullMode::kCullBack, bones, bone_bind_poses, matModel, pLightMap, rim_color, rim_power, lod_coverage);
                  renderSubmesh(ri.commandBuffer, iSubmesh, ri.renderPass, object_name, pMaterial->getName(), lod_coverage);
                  break;
                case KRMaterial::KRMATERIAL_ALPHA_MODE_BLENDTWOSIDE: // Blended alpha rendered in two passes.  First pass renders backfaces; second pass renders frontfaces.
                    // Render back faces first
                  pMaterial->bind(ri, getModelFormat(), getVertexAttributes(), CullMode::kCullFront, bones, bone_bind_poses, matModel, pLightMap, rim_color, rim_power, lod_coverage);
                  renderSubmesh(ri.commandBuffer, iSubmesh, ri.renderPass, object_name, pMaterial->getName(), lod_coverage);

                  // Render front faces second
                  pMaterial->bind(ri, getModelFormat(), getVertexAttributes(), CullMode::kCullBack, bones, bone_bind_poses, matModel, pLightMap, rim_color, rim_power, lod_coverage);
                  renderSubmesh(ri.commandBuffer, iSubmesh, ri.renderPass, object_name, pMaterial->getName(), lod_coverage);
                  break;
                }
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
  if (m_maxPoint.x - m_minPoint.x > m) m = m_maxPoint.x - m_minPoint.x;
  if (m_maxPoint.y - m_minPoint.y > m) m = m_maxPoint.y - m_minPoint.y;
  if (m_maxPoint.z - m_minPoint.z > m) m = m_maxPoint.z - m_minPoint.z;
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
    int32_t vertex_attrib_flags = pHeader->vertex_attrib_flags;
    int32_t vertex_count = pHeader->vertex_count;

    int vbo_index = 0;
    if (getModelFormat() == ModelFormat::KRENGINE_MODEL_FORMAT_INDEXED_TRIANGLES
      || getModelFormat() == ModelFormat::KRENGINE_MODEL_FORMAT_INDEXED_STRIP) {

      int index_group = getSubmesh(iSubmesh)->index_group;
      int index_group_offset = getSubmesh(iSubmesh)->index_group_offset;
      while (cVertexes > 0) {

        int start_index_offset, start_vertex_offset, index_count, vertex_count;
        getIndexedRange(index_group++, start_index_offset, start_vertex_offset, index_count, vertex_count);

        if ((int)mesh.vertex_data_blocks.size() <= vbo_index) {
          Block* vertex_data_block = m_pData->getSubBlock(vertex_data_offset + start_vertex_offset * m_vertex_size, vertex_count * m_vertex_size);
          Block* index_data_block = m_pData->getSubBlock(index_data_offset + start_index_offset * 2, index_count * 2);
          mesh.vbo_data_blocks.emplace_back(std::make_shared<KRMeshManager::KRVBOData>(getContext().getMeshManager(), vertex_data_block, index_data_block, vertex_attrib_flags, true, t
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
        int vertex_size = m_vertex_size;

        if ((int)mesh.vertex_data_blocks.size() <= vbo_index) {
          Block* index_data_block = NULL;
          Block* vertex_data_block = m_pData->getSubBlock(vertex_data_offset + iBuffer * MAX_VBO_SIZE * vertex_size, vertex_size * cBufferVertexes);
          mesh.vbo_data_blocks.emplace_back(std::make_shared<KRMeshManager::KRVBOData>(getContext().getMeshManager(), vertex_data_block, index_data_block, vertex_attrib_flags, true, t
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

void KRMesh::renderNoMaterials(VkCommandBuffer& commandBuffer, KRNode::RenderPass renderPass, const std::string& object_name, const std::string& material_name, float lodCoverage)
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

void KRMesh::renderSubmesh(VkCommandBuffer& commandBuffer, int iSubmesh, KRNode::RenderPass renderPass, const std::string& object_name, const std::string& material_name, float lodCoverage)
{
  getSubmeshes();

  Submesh& mesh = m_submeshes[iSubmesh];
  int cVertexes = mesh.vertex_count;

  vector<shared_ptr<KRMeshManager::KRVBOData>>::iterator vbo_itr = mesh.vbo_data_blocks.begin();
  int vbo_index = 0;
  if (getModelFormat() == ModelFormat::KRENGINE_MODEL_FORMAT_INDEXED_TRIANGLES) {

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
      m_pContext->getMeshManager()->log_draw_call(renderPass, object_name, material_name, vertex_draw_count);
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
        switch (getModelFormat()) {
        case ModelFormat::KRENGINE_MODEL_FORMAT_TRIANGLES:
        case ModelFormat::KRENGINE_MODEL_FORMAT_STRIP:
          vkCmdDraw(commandBuffer, (MAX_VBO_SIZE - iVertex), 1, iVertex, 0);
          break;
        case ModelFormat::KRENGINE_MODEL_FORMAT_INDEXED_TRIANGLES:
        case ModelFormat::KRENGINE_MODEL_FORMAT_INDEXED_STRIP:
          vkCmdDrawIndexed(commandBuffer, (MAX_VBO_SIZE - iVertex), 1, iVertex, 0, 0);
          break;
        }
        m_pContext->getMeshManager()->log_draw_call(renderPass, object_name, material_name, (MAX_VBO_SIZE - iVertex));

        cVertexes -= (MAX_VBO_SIZE - iVertex);
        iVertex = 0;
        iBuffer++;
      } else {
        assert(iVertex + cVertexes <= cBufferVertexes);

        switch (getModelFormat()) {
        case ModelFormat::KRENGINE_MODEL_FORMAT_TRIANGLES:
        case ModelFormat::KRENGINE_MODEL_FORMAT_STRIP:
          vkCmdDraw(commandBuffer, cVertexes, 1, iVertex, 0);
          break;
        default:
          break;
        }
        m_pContext->getMeshManager()->log_draw_call(renderPass, object_name, material_name, cVertexes);

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
  bool use_short_uva = true;
  bool use_short_uvb = true;

  if (use_short_vertexes) {
    for (std::vector<Vector3>::const_iterator itr = mi.vertices.begin(); itr != mi.vertices.end(); itr++) {
      if (fabsf((*itr).x) > 1.0f || fabsf((*itr).y) > 1.0f || fabsf((*itr).z) > 1.0f) {
        use_short_vertexes = false;
      }
    }
  }

  if (use_short_uva) {
    for (std::vector<Vector2>::const_iterator itr = mi.uva.begin(); itr != mi.uva.end(); itr++) {
      if (fabsf((*itr).x) > 1.0f || fabsf((*itr).y) > 1.0f) {
        use_short_uva = false;
      }
    }
  }

  if (use_short_uvb) {
    for (std::vector<Vector2>::const_iterator itr = mi.uvb.begin(); itr != mi.uvb.end(); itr++) {
      if (fabsf((*itr).x) > 1.0f || fabsf((*itr).y) > 1.0f) {
        use_short_uvb = false;
      }
    }
  }

  __int32_t vertex_attrib_flags = 0;
  if (mi.vertices.size()) {
    if (use_short_vertexes) {
      vertex_attrib_flags |= (1 << KRENGINE_ATTRIB_VERTEX_SHORT);
    } else {
      vertex_attrib_flags |= (1 << KRENGINE_ATTRIB_VERTEX);
    }
  }
  if (mi.normals.size() || calculate_normals) {
    if (use_short_normals) {
      vertex_attrib_flags += (1 << KRENGINE_ATTRIB_NORMAL_SHORT);
    } else {
      vertex_attrib_flags += (1 << KRENGINE_ATTRIB_NORMAL);
    }
  }
  if (mi.tangents.size() || calculate_tangents) {
    if (use_short_tangents) {
      vertex_attrib_flags += (1 << KRENGINE_ATTRIB_TANGENT_SHORT);
    } else {
      vertex_attrib_flags += (1 << KRENGINE_ATTRIB_TANGENT);
    }
  }
  if (mi.uva.size()) {
    if (use_short_uva) {
      vertex_attrib_flags += (1 << KRENGINE_ATTRIB_TEXUVA_SHORT);
    } else {
      vertex_attrib_flags += (1 << KRENGINE_ATTRIB_TEXUVA);
    }
  }
  if (mi.uvb.size()) {
    if (use_short_uvb) {
      vertex_attrib_flags += (1 << KRENGINE_ATTRIB_TEXUVB_SHORT);
    } else {
      vertex_attrib_flags += (1 << KRENGINE_ATTRIB_TEXUVB);
    }
  }
  if (mi.bone_names.size()) {
    vertex_attrib_flags += (1 << KRENGINE_ATTRIB_BONEINDEXES) + (1 << KRENGINE_ATTRIB_BONEWEIGHTS);
  }
  size_t vertex_size = VertexSizeForAttributes(vertex_attrib_flags);
  size_t index_count = mi.vertex_indexes.size();
  size_t index_base_count = mi.vertex_index_bases.size();
  size_t submesh_count = mi.submesh_lengths.size();
  size_t vertex_count = mi.vertices.size();
  size_t bone_count = mi.bone_names.size();
  size_t new_file_size = sizeof(pack_header) + sizeof(pack_material) * submesh_count + sizeof(pack_bone) * bone_count + KRALIGN(2 * index_count) + KRALIGN(8 * index_base_count) + vertex_size * vertex_count;
  m_pData = new Block();
  m_pMetaData = m_pData;
  m_pData->expand(new_file_size);
  m_pData->lock();
  pack_header* pHeader = getHeader();
  memset(pHeader, 0, sizeof(pack_header));
  pHeader->vertex_attrib_flags = vertex_attrib_flags;
  pHeader->submesh_count = (__int32_t)submesh_count;
  pHeader->vertex_count = (__int32_t)vertex_count;
  pHeader->bone_count = (__int32_t)bone_count;
  pHeader->index_count = (__int32_t)index_count;
  pHeader->index_base_count = (__int32_t)index_base_count;
  pHeader->model_format = (__int32_t)mi.format;
  strcpy(pHeader->szTag, "KROBJPACK1.2   ");
  updateAttributeOffsets();

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

  memset(getVertexData(), 0, m_vertex_size * (int)mi.vertices.size());
  for (int iVertex = 0; iVertex < (int)mi.vertices.size(); iVertex++) {
    Vector3 source_vertex = mi.vertices[iVertex];
    setVertexPosition(iVertex, source_vertex);
    if (mi.bone_names.size()) {
      for (int bone_weight_index = 0; bone_weight_index < KRENGINE_MAX_BONE_WEIGHTS_PER_VERTEX; bone_weight_index++) {
        setBoneIndex(iVertex, bone_weight_index, mi.bone_indexes[iVertex][bone_weight_index]);
        setBoneWeight(iVertex, bone_weight_index, mi.bone_weights[iVertex][bone_weight_index]);
      }
    }
    if (bFirstVertex) {
      bFirstVertex = false;
      m_minPoint = source_vertex;
      m_maxPoint = source_vertex;
    } else {
      if (source_vertex.x < m_minPoint.x) m_minPoint.x = source_vertex.x;
      if (source_vertex.y < m_minPoint.y) m_minPoint.y = source_vertex.y;
      if (source_vertex.z < m_minPoint.z) m_minPoint.z = source_vertex.z;
      if (source_vertex.x > m_maxPoint.x) m_maxPoint.x = source_vertex.x;
      if (source_vertex.y > m_maxPoint.y) m_maxPoint.y = source_vertex.y;
      if (source_vertex.z > m_maxPoint.z) m_maxPoint.z = source_vertex.z;
    }
    if ((int)mi.uva.size() > iVertex) {
      setVertexUVA(iVertex, mi.uva[iVertex]);
    }
    if ((int)mi.uvb.size() > iVertex) {
      setVertexUVB(iVertex, mi.uvb[iVertex]);
    }
    if ((int)mi.normals.size() > iVertex) {
      setVertexNormal(iVertex, Vector3::Normalize(mi.normals[iVertex]));
    }
    if ((int)mi.tangents.size() > iVertex) {
      setVertexTangent(iVertex, Vector3::Normalize(mi.tangents[iVertex]));
    }
  }

  pHeader->minx = m_minPoint.x;
  pHeader->miny = m_minPoint.y;
  pHeader->minz = m_minPoint.z;
  pHeader->maxx = m_maxPoint.x;
  pHeader->maxy = m_maxPoint.y;
  pHeader->maxz = m_maxPoint.z;


  __uint16_t* index_data = getIndexData();
  for (std::vector<__uint16_t>::const_iterator itr = mi.vertex_indexes.begin(); itr != mi.vertex_indexes.end(); itr++) {
    *index_data++ = *itr;
  }

  __uint32_t* index_base_data = getIndexBaseData();
  for (std::vector<std::pair<int, int> >::const_iterator itr = mi.vertex_index_bases.begin(); itr != mi.vertex_index_bases.end(); itr++) {
    *index_base_data++ = (*itr).first;
    *index_base_data++ = (*itr).second;
  }

  if (getModelFormat() == ModelFormat::KRENGINE_MODEL_FORMAT_TRIANGLES) {
    // Calculate missing surface normals and tangents
    //cout << "  Calculate surface normals and tangents\n";
    if (calculate_normals || calculate_tangents) {
      // NOTE: This will not work properly if the vertices are already indexed
      for (int iVertex = 0; iVertex < (int)mi.vertices.size(); iVertex += 3) {
        Vector3 p1 = getVertexPosition(iVertex);
        Vector3 p2 = getVertexPosition(iVertex + 1);
        Vector3 p3 = getVertexPosition(iVertex + 2);
        Vector3 v1 = p2 - p1;
        Vector3 v2 = p3 - p1;


        // -- Calculate normal if missing --
        if (calculate_normals) {
          Vector3 first_normal = getVertexNormal(iVertex);
          if (first_normal.x == 0.0f && first_normal.y == 0.0f && first_normal.z == 0.0f) {
            // Note - We don't take into consideration smoothing groups or smoothing angles when generating normals; all generated normals represent flat shaded polygons
            Vector3 normal = Vector3::Cross(v1, v2);

            normal.normalize();
            setVertexNormal(iVertex, normal);
            setVertexNormal(iVertex + 1, normal);
            setVertexNormal(iVertex + 2, normal);
          }
        }

        // -- Calculate tangent vector for normal mapping --
        if (calculate_tangents) {
          Vector3 first_tangent = getVertexTangent(iVertex);
          if (first_tangent.x == 0.0f && first_tangent.y == 0.0f && first_tangent.z == 0.0f) {

            Vector2 uv0 = getVertexUVA(iVertex);
            Vector2 uv1 = getVertexUVA(iVertex + 1);
            Vector2 uv2 = getVertexUVA(iVertex + 2);

            Vector2 st1 = Vector2::Create(uv1.x - uv0.x, uv1.y - uv0.y);
            Vector2 st2 = Vector2::Create(uv2.x - uv0.x, uv2.y - uv0.y);
            float coef = 1 / (st1.x * st2.y - st2.x * st1.y);

            Vector3 tangent = Vector3::Create(
                              coef * ((v1.x * st2.y) + (v2.x * -st1.y)),
                              coef * ((v1.y * st2.y) + (v2.y * -st1.y)),
                              coef * ((v1.z * st2.y) + (v2.z * -st1.y))
            );

            tangent.normalize();
            setVertexTangent(iVertex, tangent);
            setVertexTangent(iVertex + 1, tangent);
            setVertexTangent(iVertex + 2, tangent);
          }
        }
      }
    }
  }
  m_pData->unlock();

  // ----

  pack_header ph;
  m_pData->copy((void*)&ph, 0, sizeof(ph));
  m_pMetaData = m_pData->getSubBlock(0, sizeof(pack_header) + sizeof(pack_material) * ph.submesh_count + sizeof(pack_bone) * ph.bone_count);
  m_pMetaData->lock();
  m_pIndexBaseData = m_pData->getSubBlock(sizeof(pack_header) + sizeof(pack_material) * ph.submesh_count + sizeof(pack_bone) * ph.bone_count + KRALIGN(2 * ph.index_count), ph.index_base_count * 8);
  m_pIndexBaseData->lock();

  // ----

  optimize();

  if (m_constant) {
    // Ensure that constant models loaded immediately by the streamer
    getSubmeshes();
    getMaterials();
  }
}

Vector3 KRMesh::getMinPoint() const
{
  return m_minPoint;
}

Vector3 KRMesh::getMaxPoint() const
{
  return m_maxPoint;
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

bool KRMesh::has_vertex_attribute(vertex_attrib_t attribute_type) const
{
  //return (getHeader()->vertex_attrib_flags & (1 << attribute_type)) != 0;
  return has_vertex_attribute(getHeader()->vertex_attrib_flags, attribute_type);
}

bool KRMesh::has_vertex_attribute(int vertex_attrib_flags, vertex_attrib_t attribute_type)
{
  return (vertex_attrib_flags & (1 << attribute_type)) != 0;
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
  return sizeof(pack_header) + sizeof(pack_material) * pHeader->submesh_count + sizeof(pack_bone) * pHeader->bone_count + KRALIGN(2 * pHeader->index_count) + KRALIGN(8 * pHeader->index_base_count);
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
    return (__uint32_t*)((unsigned char*)m_pData->getStart() + sizeof(pack_header) + sizeof(pack_material) * pHeader->submesh_count + sizeof(pack_bone) * pHeader->bone_count + KRALIGN(2 * pHeader->index_count));
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
  return getVertexData() + m_vertex_size * index;
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

__uint32_t KRMesh::getVertexAttributes() const
{
  pack_header* header = getHeader();
  __uint32_t attributes = header->vertex_attrib_flags;
  return attributes;
}

Vector3 KRMesh::getVertexPosition(int index) const
{
  if (has_vertex_attribute(KRENGINE_ATTRIB_VERTEX_SHORT)) {
    short* v = (short*)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_VERTEX_SHORT]);
    return Vector3::Create((float)v[0] / 32767.0f, (float)v[1] / 32767.0f, (float)v[2] / 32767.0f);
  } else if (has_vertex_attribute(KRENGINE_ATTRIB_VERTEX)) {
    return Vector3::Create((float*)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_VERTEX]));
  } else {
    return Vector3::Zero();
  }
}

Vector3 KRMesh::getVertexNormal(int index) const
{
  if (has_vertex_attribute(KRENGINE_ATTRIB_NORMAL_SHORT)) {
    short* v = (short*)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_NORMAL_SHORT]);
    return Vector3::Create((float)v[0] / 32767.0f, (float)v[1] / 32767.0f, (float)v[2] / 32767.0f);
  } else if (has_vertex_attribute(KRENGINE_ATTRIB_NORMAL)) {
    return Vector3::Create((float*)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_NORMAL]));
  } else {
    return Vector3::Zero();
  }
}

Vector3 KRMesh::getVertexTangent(int index) const
{
  if (has_vertex_attribute(KRENGINE_ATTRIB_TANGENT_SHORT)) {
    short* v = (short*)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_TANGENT_SHORT]);
    return Vector3::Create((float)v[0] / 32767.0f, (float)v[1] / 32767.0f, (float)v[2] / 32767.0f);
  } else if (has_vertex_attribute(KRENGINE_ATTRIB_TANGENT)) {
    return Vector3::Create((float*)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_TANGENT]));
  } else {
    return Vector3::Zero();
  }
}

Vector2 KRMesh::getVertexUVA(int index) const
{
  if (has_vertex_attribute(KRENGINE_ATTRIB_TEXUVA_SHORT)) {
    short* v = (short*)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_TEXUVA_SHORT]);
    return Vector2::Create((float)v[0] / 32767.0f, (float)v[1] / 32767.0f);
  } else if (has_vertex_attribute(KRENGINE_ATTRIB_TEXUVA)) {
    return Vector2::Create((float*)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_TEXUVA]));
  } else {
    return Vector2::Zero();
  }
}

Vector2 KRMesh::getVertexUVB(int index) const
{
  if (has_vertex_attribute(KRENGINE_ATTRIB_TEXUVB_SHORT)) {
    short* v = (short*)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_TEXUVB_SHORT]);
    return Vector2::Create((float)v[0] / 32767.0f, (float)v[1] / 32767.0f);
  } else if (has_vertex_attribute(KRENGINE_ATTRIB_TEXUVB)) {
    return Vector2::Create((float*)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_TEXUVB]));
  } else {
    return Vector2::Zero();
  }
}

void KRMesh::setVertexPosition(int index, const Vector3& v)
{
  if (has_vertex_attribute(KRENGINE_ATTRIB_VERTEX_SHORT)) {
    short* vert = (short*)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_VERTEX_SHORT]);
    vert[0] = (short)(v.x * 32767.0f);
    vert[1] = (short)(v.y * 32767.0f);
    vert[2] = (short)(v.z * 32767.0f);
  } else if (has_vertex_attribute(KRENGINE_ATTRIB_VERTEX)) {
    float* vert = (float*)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_VERTEX]);
    vert[0] = v.x;
    vert[1] = v.y;
    vert[2] = v.z;
  }
}

void KRMesh::setVertexNormal(int index, const Vector3& v)
{
  if (has_vertex_attribute(KRENGINE_ATTRIB_NORMAL_SHORT)) {
    short* vert = (short*)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_NORMAL_SHORT]);
    vert[0] = (short)(v.x * 32767.0f);
    vert[1] = (short)(v.y * 32767.0f);
    vert[2] = (short)(v.z * 32767.0f);
  } else if (has_vertex_attribute(KRENGINE_ATTRIB_NORMAL)) {
    float* vert = (float*)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_NORMAL]);
    vert[0] = v.x;
    vert[1] = v.y;
    vert[2] = v.z;
  }
}

void KRMesh::setVertexTangent(int index, const Vector3& v)
{
  if (has_vertex_attribute(KRENGINE_ATTRIB_TANGENT_SHORT)) {
    short* vert = (short*)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_TANGENT_SHORT]);
    vert[0] = (short)(v.x * 32767.0f);
    vert[1] = (short)(v.y * 32767.0f);
    vert[2] = (short)(v.z * 32767.0f);
  } else if (has_vertex_attribute(KRENGINE_ATTRIB_TANGENT)) {
    float* vert = (float*)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_TANGENT]);
    vert[0] = v.x;
    vert[1] = v.y;
    vert[2] = v.z;
  }
}

void KRMesh::setVertexUVA(int index, const Vector2& v)
{
  if (has_vertex_attribute(KRENGINE_ATTRIB_TEXUVA_SHORT)) {
    short* vert = (short*)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_TEXUVA_SHORT]);
    vert[0] = (short)(v.x * 32767.0f);
    vert[1] = (short)(v.y * 32767.0f);
  } else if (has_vertex_attribute(KRENGINE_ATTRIB_TEXUVA)) {
    float* vert = (float*)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_TEXUVA]);
    vert[0] = v.x;
    vert[1] = v.y;
  }
}

void KRMesh::setVertexUVB(int index, const Vector2& v)
{
  if (has_vertex_attribute(KRENGINE_ATTRIB_TEXUVB_SHORT)) {
    short* vert = (short*)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_TEXUVB_SHORT]);
    vert[0] = (short)(v.x * 32767.0f);
    vert[1] = (short)(v.y * 32767.0f);
  } else if (has_vertex_attribute(KRENGINE_ATTRIB_TEXUVB)) {
    float* vert = (float*)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_TEXUVB]);
    vert[0] = v.x;
    vert[1] = v.y;
  }
}


int KRMesh::getBoneIndex(int index, int weight_index) const
{
  unsigned char* vert = (unsigned char*)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_BONEINDEXES]);
  return vert[weight_index];
}

void KRMesh::setBoneIndex(int index, int weight_index, int bone_index)
{
  unsigned char* vert = (unsigned char*)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_BONEINDEXES]);
  vert[weight_index] = bone_index;
}

float KRMesh::getBoneWeight(int index, int weight_index) const
{
  float* vert = (float*)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_BONEWEIGHTS]);
  return vert[weight_index];
}

void KRMesh::setBoneWeight(int index, int weight_index, float bone_weight)
{
  float* vert = (float*)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_BONEWEIGHTS]);
  vert[weight_index] = bone_weight;
}

size_t KRMesh::VertexSizeForAttributes(__int32_t vertex_attrib_flags)
{
  size_t data_size = 0;
  if (has_vertex_attribute(vertex_attrib_flags, KRENGINE_ATTRIB_VERTEX)) {
    data_size += sizeof(float) * 3;
  }
  if (has_vertex_attribute(vertex_attrib_flags, KRENGINE_ATTRIB_NORMAL)) {
    data_size += sizeof(float) * 3;
  }
  if (has_vertex_attribute(vertex_attrib_flags, KRENGINE_ATTRIB_TANGENT)) {
    data_size += sizeof(float) * 3;
  }
  if (has_vertex_attribute(vertex_attrib_flags, KRENGINE_ATTRIB_TEXUVA)) {
    data_size += sizeof(float) * 2;
  }
  if (has_vertex_attribute(vertex_attrib_flags, KRENGINE_ATTRIB_TEXUVB)) {
    data_size += sizeof(float) * 2;
  }
  if (has_vertex_attribute(vertex_attrib_flags, KRENGINE_ATTRIB_BONEINDEXES)) {
    data_size += 4; // 4 bytes
  }
  if (has_vertex_attribute(vertex_attrib_flags, KRENGINE_ATTRIB_BONEWEIGHTS)) {
    data_size += sizeof(float) * 4;
  }
  if (has_vertex_attribute(vertex_attrib_flags, KRENGINE_ATTRIB_VERTEX_SHORT)) {
    data_size += sizeof(short) * 4; // Extra short added in order to maintain 32-bit alignment. TODO, FINDME - Perhaps we can bind this as a vec4 and use the 4th component for another attribute...
  }
  if (has_vertex_attribute(vertex_attrib_flags, KRENGINE_ATTRIB_NORMAL_SHORT)) {
    data_size += sizeof(short) * 4; // Extra short added in order to maintain 32-bit alignment. TODO, FINDME - Perhaps we can bind this as a vec4 and use the 4th component for another attribute...
  }
  if (has_vertex_attribute(vertex_attrib_flags, KRENGINE_ATTRIB_TANGENT_SHORT)) {
    data_size += sizeof(short) * 4; // Extra short added in order to maintain 32-bit alignment. TODO, FINDME - Perhaps we can bind this as a vec4 and use the 4th component for another attribute...
  }
  if (has_vertex_attribute(vertex_attrib_flags, KRENGINE_ATTRIB_TEXUVA_SHORT)) {
    data_size += sizeof(short) * 2;
  }
  if (has_vertex_attribute(vertex_attrib_flags, KRENGINE_ATTRIB_TEXUVB_SHORT)) {
    data_size += sizeof(short) * 2;
  }
  return data_size;
}

void KRMesh::updateAttributeOffsets()
{
  pack_header* header = getHeader();
  int mask = 0;
  for (size_t i = 0; i < KRENGINE_NUM_ATTRIBUTES; i++) {
    if (has_vertex_attribute((vertex_attrib_t)i)) {
      m_vertex_attribute_offset[i] = (int)VertexSizeForAttributes(header->vertex_attrib_flags & mask);
    } else {
      m_vertex_attribute_offset[i] = -1;
    }
    mask = (mask << 1) | 1;
  }
  m_vertex_size = (int)VertexSizeForAttributes(header->vertex_attrib_flags);
}

size_t KRMesh::AttributeOffset(__int32_t vertex_attrib, __int32_t vertex_attrib_flags)
{
  int mask = 0;
  for (int i = 0; i < vertex_attrib; i++) {
    if (vertex_attrib_flags & (1 << i)) {
      mask |= (1 << i);
    }
  }
  return VertexSizeForAttributes(mask);
}

VkFormat KRMesh::AttributeVulkanFormat(__int32_t vertex_attrib)
{
  switch (vertex_attrib) {
  case KRENGINE_ATTRIB_VERTEX:
  case KRENGINE_ATTRIB_NORMAL:
  case KRENGINE_ATTRIB_TANGENT:
  case KRENGINE_ATTRIB_BONEWEIGHTS:
    return VK_FORMAT_R32G32B32_SFLOAT;
  case KRENGINE_ATTRIB_TEXUVA:
  case KRENGINE_ATTRIB_TEXUVB:
    return VK_FORMAT_R32G32_SFLOAT;
  case KRENGINE_ATTRIB_BONEINDEXES:
    return VK_FORMAT_R8G8B8A8_UINT;
  case KRENGINE_ATTRIB_VERTEX_SHORT:
  case KRENGINE_ATTRIB_NORMAL_SHORT:
  case KRENGINE_ATTRIB_TANGENT_SHORT:
    return VK_FORMAT_R16G16B16A16_SNORM;
  case KRENGINE_ATTRIB_TEXUVA_SHORT:
  case KRENGINE_ATTRIB_TEXUVB_SHORT:
    return VK_FORMAT_R16G16_SNORM;
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

ModelFormat KRMesh::getModelFormat() const
{
  ModelFormat f = (ModelFormat)getHeader()->model_format;
  return f;
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
    //        int vertex_start = getSubmesh(submesh_index)->start_vertex;
    int vertex_count = getVertexCount(submesh_index);
    switch (getModelFormat()) {
    case ModelFormat::KRENGINE_MODEL_FORMAT_TRIANGLES:
    case ModelFormat::KRENGINE_MODEL_FORMAT_INDEXED_TRIANGLES:
      for (int triangle_index = 0; triangle_index < vertex_count / 3; triangle_index++) {
        int tri_vert_index[3]; // FINDME, HACK!  This is not very efficient for indexed collider meshes...
        tri_vert_index[0] = getTriangleVertexIndex(submesh_index, triangle_index * 3);
        tri_vert_index[1] = getTriangleVertexIndex(submesh_index, triangle_index * 3 + 1);
        tri_vert_index[2] = getTriangleVertexIndex(submesh_index, triangle_index * 3 + 2);

        Triangle3 tri = Triangle3::Create(getVertexPosition(tri_vert_index[0]), getVertexPosition(tri_vert_index[1]), getVertexPosition(tri_vert_index[2]));

        if (rayCast(start, dir, tri, getVertexNormal(tri_vert_index[0]), getVertexNormal(tri_vert_index[1]), getVertexNormal(tri_vert_index[2]), hitinfo)) hit_found = true;
      }
      break;
      /*

       NOTE: Not yet supported:

      case ModelFormat::KRENGINE_MODEL_FORMAT_STRIP:
      case ModelFormat::KRENGINE_MODEL_FORMAT_INDEXED_STRIP:
          for(int triangle_index=0; triangle_index < vertex_count - 2; triangle_index++) {
              int tri_vert_index[3];
              tri_vert_index[0] = getTriangleVertexIndex(submesh_index, vertex_start + triangle_index*3);
              tri_vert_index[1] = getTriangleVertexIndex(submesh_index, vertex_start + triangle_index*3 + 1);
              tri_vert_index[2] = getTriangleVertexIndex(submesh_index, vertex_start + triangle_index*3 + 2);

              if(rayCast(v0, dir, getVertexPosition(vertex_start + triangle_index), getVertexPosition(vertex_start + triangle_index+1), getVertexPosition(vertex_start + triangle_index+2), getVertexNormal(vertex_start + triangle_index), getVertexNormal(vertex_start + triangle_index+1), getVertexNormal(vertex_start + triangle_index+2), hitinfo)) hit_found = true;
          }
          break;
       */
    default:
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
    switch (getModelFormat()) {
    case ModelFormat::KRENGINE_MODEL_FORMAT_TRIANGLES:
    case ModelFormat::KRENGINE_MODEL_FORMAT_INDEXED_TRIANGLES:
      for (int triangle_index = 0; triangle_index < vertex_count / 3; triangle_index++) {
        int tri_vert_index[3]; // FINDME, HACK!  This is not very efficient for indexed collider meshes...
        tri_vert_index[0] = getTriangleVertexIndex(submesh_index, triangle_index * 3);
        tri_vert_index[1] = getTriangleVertexIndex(submesh_index, triangle_index * 3 + 1);
        tri_vert_index[2] = getTriangleVertexIndex(submesh_index, triangle_index * 3 + 2);

        Triangle3 tri = Triangle3::Create(getVertexPosition(tri_vert_index[0]), getVertexPosition(tri_vert_index[1]), getVertexPosition(tri_vert_index[2]));

        if (sphereCast(model_to_world, v0, v1, radius, tri, hitinfo)) hit_found = true;

        /*
        Triangle3 tri2 = Triangle3(getVertexPosition(tri_vert_index[1]), getVertexPosition(tri_vert_index[0]), getVertexPosition(tri_vert_index[2]));

        if(sphereCast(model_to_world, v0, v1, radius, tri2, new_hitinfo)) hit_found = true;
        */
      }
      break;
      /*

       NOTE: Not yet supported:

       case ModelFormat::KRENGINE_MODEL_FORMAT_STRIP:
       case ModelFormat::KRENGINE_MODEL_FORMAT_INDEXED_STRIP:
       for(int triangle_index=0; triangle_index < vertex_count - 2; triangle_index++) {
       int tri_vert_index[3];
       tri_vert_index[0] = getTriangleVertexIndex(submesh_index, vertex_start + triangle_index*3);
       tri_vert_index[1] = getTriangleVertexIndex(submesh_index, vertex_start + triangle_index*3 + 1);
       tri_vert_index[2] = getTriangleVertexIndex(submesh_index, vertex_start + triangle_index*3 + 2);

       if(sphereCast(model_to_world, v0, v1, getVertexPosition(vertex_start + triangle_index), getVertexPosition(vertex_start + triangle_index+1), getVertexPosition(vertex_start + triangle_index+2), getVertexNormal(vertex_start + triangle_index), getVertexNormal(vertex_start + triangle_index+1), getVertexNormal(vertex_start + triangle_index+2), new_hitinfo)) hit_found = true;
       }
       break;
       */
    default:
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
  char* szKey = new char[m_vertex_size * 2 + 1];

  // Convert model to indexed vertices, identying vertexes with identical attributes and optimizing order of trianges for best usage post-vertex-transform cache on GPU
  int vertex_index_offset = 0;
  int vertex_index_base_start_vertex = 0;

  mesh_info mi;

  int bone_count = getBoneCount();
  for (int bone_index = 0; bone_index < bone_count; bone_index++) {
    mi.bone_names.push_back(getBoneName(bone_index));
    mi.bone_bind_poses.push_back(getBoneBindPose(bone_index));
  }

  for (int submesh_index = 0; submesh_index < getSubmeshCount(); submesh_index++) {
    mi.material_names.push_back(getSubmesh(submesh_index)->szName);

    int vertexes_remaining = getVertexCount(submesh_index);

    int vertex_count = vertexes_remaining;
    if (vertex_count > 0xffff) {
      vertex_count = 0xffff;
    }

    if (submesh_index == 0 || vertex_index_offset + vertex_count > 0xffff) {
      mi.vertex_index_bases.push_back(std::pair<int, int>((int)mi.vertex_indexes.size(), (int)mi.vertices.size()));
      vertex_index_offset = 0;
      vertex_index_base_start_vertex = (int)mi.vertices.size();
    }

    mi.submesh_starts.push_back((int)mi.vertex_index_bases.size() - 1 + (vertex_index_offset << 16));
    mi.submesh_lengths.push_back(vertexes_remaining);
    int source_index = getSubmesh(submesh_index)->start_vertex;


    while (vertexes_remaining) {

      //typedef std::pair<std::vector<float>, std::vector<int> > vertex_key_t;
      typedef std::string vertex_key_t;

      unordered_map<vertex_key_t, int> prev_indexes = unordered_map<vertex_key_t, int>();

      for (int i = 0; i < vertex_count; i++) {

        Vector3 vertex_position = getVertexPosition(source_index);
        Vector2 vertex_uva = getVertexUVA(source_index);
        Vector2 vertex_uvb = getVertexUVB(source_index);
        Vector3 vertex_normal = getVertexNormal(source_index);
        Vector3 vertex_tangent = getVertexTangent(source_index);
        std::vector<int> vertex_bone_indexes;
        if (has_vertex_attribute(KRENGINE_ATTRIB_BONEINDEXES)) {
          vertex_bone_indexes.push_back(getBoneIndex(source_index, 0));
          vertex_bone_indexes.push_back(getBoneIndex(source_index, 1));
          vertex_bone_indexes.push_back(getBoneIndex(source_index, 2));
          vertex_bone_indexes.push_back(getBoneIndex(source_index, 3));
        }
        std::vector<float> vertex_bone_weights;
        if (has_vertex_attribute(KRENGINE_ATTRIB_BONEWEIGHTS)) {
          vertex_bone_weights.push_back(getBoneWeight(source_index, 0));
          vertex_bone_weights.push_back(getBoneWeight(source_index, 1));
          vertex_bone_weights.push_back(getBoneWeight(source_index, 2));
          vertex_bone_weights.push_back(getBoneWeight(source_index, 3));
        }



        unsigned char* vertex_data = (unsigned char*)getVertexData(source_index);
        for (int b = 0; b < m_vertex_size; b++) {
          const char* szHex = "0123456789ABCDEF";
          szKey[b * 2] = szHex[vertex_data[b] & 0x0f];
          szKey[b * 2 + 1] = szHex[((vertex_data[b] & 0xf0) >> 4)];
        }
        szKey[m_vertex_size * 2] = '\0';

        vertex_key_t vertex_key = szKey;
        /*

         vertex_key_t vertex_key = std::make_pair(std::vector<float>(), std::vector<int>());

        if(has_vertex_attribute(KRENGINE_ATTRIB_VERTEX) || has_vertex_attribute(KRENGINE_ATTRIB_VERTEX_SHORT)) {
            vertex_key.first.push_back(vertex_position.x);
            vertex_key.first.push_back(vertex_position.y);
            vertex_key.first.push_back(vertex_position.z);
        }
        if(has_vertex_attribute(KRENGINE_ATTRIB_NORMAL) || has_vertex_attribute(KRENGINE_ATTRIB_NORMAL_SHORT)) {
            vertex_key.first.push_back(vertex_normal.x);
            vertex_key.first.push_back(vertex_normal.y);
            vertex_key.first.push_back(vertex_normal.z);
        }
        if(has_vertex_attribute(KRENGINE_ATTRIB_TEXUVA) || has_vertex_attribute(KRENGINE_ATTRIB_TEXUVA_SHORT)) {
            vertex_key.first.push_back(vertex_uva.x);
            vertex_key.first.push_back(vertex_uva.y);
        }
        if(has_vertex_attribute(KRENGINE_ATTRIB_TEXUVB) || has_vertex_attribute(KRENGINE_ATTRIB_TEXUVB_SHORT)) {
            vertex_key.first.push_back(vertex_uvb.x);
            vertex_key.first.push_back(vertex_uvb.y);
        }
        if(has_vertex_attribute(KRENGINE_ATTRIB_TANGENT) || has_vertex_attribute(KRENGINE_ATTRIB_TANGENT_SHORT)) {
            vertex_key.first.push_back(vertex_tangent.x);
            vertex_key.first.push_back(vertex_tangent.y);
            vertex_key.first.push_back(vertex_tangent.z);
        }
        if(has_vertex_attribute(KRENGINE_ATTRIB_BONEINDEXES)) {
            vertex_key.second.push_back(vertex_bone_indexes[0]);
            vertex_key.second.push_back(vertex_bone_indexes[1]);
            vertex_key.second.push_back(vertex_bone_indexes[2]);
            vertex_key.second.push_back(vertex_bone_indexes[3]);
        }
        if(has_vertex_attribute(KRENGINE_ATTRIB_BONEWEIGHTS)) {
            vertex_key.first.push_back(vertex_bone_weights[0]);
            vertex_key.first.push_back(vertex_bone_weights[1]);
            vertex_key.first.push_back(vertex_bone_weights[2]);
            vertex_key.first.push_back(vertex_bone_weights[3]);
        }
        */
        int found_index = -1;
        if (prev_indexes.count(vertex_key) == 0) {
          found_index = (int)mi.vertices.size() - vertex_index_base_start_vertex;
          if (has_vertex_attribute(KRENGINE_ATTRIB_VERTEX) || has_vertex_attribute(KRENGINE_ATTRIB_VERTEX_SHORT)) {
            mi.vertices.push_back(vertex_position);
          }
          if (has_vertex_attribute(KRENGINE_ATTRIB_NORMAL) || has_vertex_attribute(KRENGINE_ATTRIB_NORMAL_SHORT)) {
            mi.normals.push_back(vertex_normal);
          }
          if (has_vertex_attribute(KRENGINE_ATTRIB_TANGENT) || has_vertex_attribute(KRENGINE_ATTRIB_TANGENT_SHORT)) {
            mi.tangents.push_back(vertex_tangent);
          }
          if (has_vertex_attribute(KRENGINE_ATTRIB_TEXUVA) || has_vertex_attribute(KRENGINE_ATTRIB_TEXUVA_SHORT)) {
            mi.uva.push_back(vertex_uva);
          }
          if (has_vertex_attribute(KRENGINE_ATTRIB_TEXUVB) || has_vertex_attribute(KRENGINE_ATTRIB_TEXUVB_SHORT)) {
            mi.uvb.push_back(vertex_uvb);
          }
          if (has_vertex_attribute(KRENGINE_ATTRIB_BONEINDEXES)) {
            mi.bone_indexes.push_back(vertex_bone_indexes);

          }
          if (has_vertex_attribute(KRENGINE_ATTRIB_BONEWEIGHTS)) {
            mi.bone_weights.push_back(vertex_bone_weights);
          }
          prev_indexes[vertex_key] = found_index;
        } else {
          found_index = prev_indexes[vertex_key];
        }

        mi.vertex_indexes.push_back(found_index);
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
        mi.vertex_index_bases.push_back(std::pair<int, int>((int)mi.vertex_indexes.size(), (int)mi.vertices.size()));
        vertex_index_offset = 0;
        vertex_index_base_start_vertex = (int)mi.vertices.size();
      }
    }
  }

  delete[] szKey;

  KRContext::Log(KRContext::LOG_LEVEL_INFORMATION, "Convert to indexed, before: %i after: %i (%.2f%% saving)", getHeader()->vertex_count, mi.vertices.size(), ((float)getHeader()->vertex_count - (float)mi.vertices.size()) / (float)getHeader()->vertex_count * 100.0f);

  switch (getModelFormat()) {
  case ModelFormat::KRENGINE_MODEL_FORMAT_TRIANGLES:
    mi.format = ModelFormat::KRENGINE_MODEL_FORMAT_INDEXED_TRIANGLES;
    break;
  case ModelFormat::KRENGINE_MODEL_FORMAT_STRIP:
    mi.format = ModelFormat::KRENGINE_MODEL_FORMAT_INDEXED_STRIP;
    break;
  default:
    assert(false);
  }

  m_pData->unlock();
  LoadData(mi, false, false);
}

void KRMesh::optimize()
{
  switch (getModelFormat()) {
  case ModelFormat::KRENGINE_MODEL_FORMAT_INDEXED_TRIANGLES:
  case ModelFormat::KRENGINE_MODEL_FORMAT_INDEXED_STRIP:
    optimizeIndexes();
    break;
  case ModelFormat::KRENGINE_MODEL_FORMAT_STRIP:
  case ModelFormat::KRENGINE_MODEL_FORMAT_TRIANGLES:
    convertToIndexed(); // HACK, FINDME, TODO - This may not be ideal in every case and should be exposed through the API independently
    break;
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
    index_count = h->index_count - start_index_offset;
    vertex_count = h->vertex_count - start_vertex_offset;
  }
}

int KRMesh::getTriangleVertexIndex(int submesh, int index) const
{
  switch (getModelFormat()) {
  case ModelFormat::KRENGINE_MODEL_FORMAT_INDEXED_TRIANGLES:
  {
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
  }
  break;
  default:
    return getSubmesh(submesh)->start_vertex + index;
    break;
  }
}

void KRMesh::optimizeIndexes()
{
  // TODO - Re-enable this once crash with KRMeshSphere vertices is corrected
  return;

  m_pData->lock();
  // TODO - Implement optimization for indexed strips
  if (getModelFormat() == ModelFormat::KRENGINE_MODEL_FORMAT_INDEXED_TRIANGLES) {

    __uint16_t* new_indices = (__uint16_t*)malloc(0x10000 * sizeof(__uint16_t));
    __uint16_t* vertex_mapping = (__uint16_t*)malloc(0x10000 * sizeof(__uint16_t));
    unsigned char* new_vertex_data = (unsigned char*)malloc(m_vertex_size * 0x10000);

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
            memcpy(new_vertex_data + vertex_mapping[i] * m_vertex_size, vertex_data_start + i * m_vertex_size, m_vertex_size);
        }
        memcpy(vertex_data_start, new_vertex_data, vertex_count * m_vertex_size);
         */



        index_group_offset = 0;
      }
    }

    free(new_indices);
    free(vertex_mapping);
    free(new_vertex_data);
  } // if(getModelFormat() == ModelFormat::KRENGINE_MODEL_FORMAT_INDEXED_TRIANGLES)

  m_pData->unlock();
}
