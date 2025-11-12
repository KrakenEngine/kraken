//
//  KRModel.cpp
//  Kraken Engine
//
//  Copyright 2025 Kearwood Gilbert. All rights reserved.
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

#include "KRModel.h"
#include "KRContext.h"
#include "resources/mesh/KRMesh.h"
#include "KRNode.h"
#include "KRRenderPass.h"

using namespace hydra;

/* static */
void KRModel::InitNodeInfo(KrNodeInfo* nodeInfo)
{
  KRNode::InitNodeInfo(nodeInfo);
  nodeInfo->model.faces_camera = false;
  nodeInfo->model.light_map_texture = KR_NULL_HANDLE;
  nodeInfo->model.lod_min_coverage = 0.0f;
  for (int lod = 0; lod < kMeshLODCount; lod++) {
    nodeInfo->model.mesh[lod] = KR_NULL_HANDLE;
  }
  nodeInfo->model.receives_shadow = true;
  nodeInfo->model.rim_color = Vector3::Zero();
  nodeInfo->model.rim_power = 0.0f;
}

KRModel::KRModel(KRScene& scene, std::string name)
  : KRNode(scene, name)
  , m_min_lod_coverage(0.0f)
  , m_receivesShadow(true)
  , m_faces_camera(false)
  , m_rim_color(Vector3::Zero())
  , m_rim_power(0.0f)
{
  m_boundsCachedMat.c[0] = -1.0f;
  m_boundsCachedMat.c[1] = -1.0f;
  m_boundsCachedMat.c[2] = -1.0f;
  m_boundsCachedMat.c[3] = -1.0f;
  m_boundsCachedMat.c[4] = -1.0f;
  m_boundsCachedMat.c[5] = -1.0f;
  m_boundsCachedMat.c[6] = -1.0f;
  m_boundsCachedMat.c[7] = -1.0f;
  m_boundsCachedMat.c[8] = -1.0f;
  m_boundsCachedMat.c[9] = -1.0f;
  m_boundsCachedMat.c[10] = -1.0f;
  m_boundsCachedMat.c[11] = -1.0f;
  m_boundsCachedMat.c[12] = -1.0f;
  m_boundsCachedMat.c[13] = -1.0f;
  m_boundsCachedMat.c[14] = -1.0f;
  m_boundsCachedMat.c[15] = -1.0f;
}

KRModel::KRModel(KRScene& scene, std::string instance_name, std::string model_name[kMeshLODCount], std::string light_map, float lod_min_coverage, bool receives_shadow, bool faces_camera, Vector3 rim_color, float rim_power)
  : KRNode(scene, instance_name)
{
  m_lightMap.setName(light_map);
  for (int lod = 0; lod < kMeshLODCount; lod++) {
    m_meshes[lod].setName(model_name[lod]);
  }
  m_min_lod_coverage = lod_min_coverage;
  m_receivesShadow = receives_shadow;
  m_faces_camera = faces_camera;
  m_rim_color = rim_color;
  m_rim_power = rim_power;

  m_boundsCachedMat.c[0] = -1.0f;
  m_boundsCachedMat.c[1] = -1.0f;
  m_boundsCachedMat.c[2] = -1.0f;
  m_boundsCachedMat.c[3] = -1.0f;
  m_boundsCachedMat.c[4] = -1.0f;
  m_boundsCachedMat.c[5] = -1.0f;
  m_boundsCachedMat.c[6] = -1.0f;
  m_boundsCachedMat.c[7] = -1.0f;
  m_boundsCachedMat.c[8] = -1.0f;
  m_boundsCachedMat.c[9] = -1.0f;
  m_boundsCachedMat.c[10] = -1.0f;
  m_boundsCachedMat.c[11] = -1.0f;
  m_boundsCachedMat.c[12] = -1.0f;
  m_boundsCachedMat.c[13] = -1.0f;
  m_boundsCachedMat.c[14] = -1.0f;
  m_boundsCachedMat.c[15] = -1.0f;
}

KRModel::~KRModel()
{

}

KrResult KRModel::update(const KrNodeInfo* nodeInfo)
{
  KrResult res = KRNode::update(nodeInfo);
  if (res != KR_SUCCESS) {
    return res;
  }
  m_faces_camera = nodeInfo->model.faces_camera;
  m_min_lod_coverage = nodeInfo->model.lod_min_coverage;
  m_receivesShadow = nodeInfo->model.receives_shadow;
  m_rim_color = nodeInfo->model.rim_color;
  m_rim_power = nodeInfo->model.rim_power;
  
  KRTexture* light_map_texture = nullptr;
  if (nodeInfo->model.light_map_texture != KR_NULL_HANDLE) {
    res = m_pContext->getMappedResource<KRTexture>(nodeInfo->model.light_map_texture, &light_map_texture);
    if (res != KR_SUCCESS) {
      return res;
    }
  }
  m_lightMap.set(light_map_texture);

  for (int lod = 0; lod < kMeshLODCount; lod++) {
    KRMesh* mesh = nullptr;
    if (nodeInfo->model.mesh[lod] != KR_NULL_HANDLE) {
      res = m_pContext->getMappedResource<KRMesh>(nodeInfo->model.mesh[lod], &mesh);
      if (res != KR_SUCCESS) {
        return res;
      }
    }
    m_meshes[lod].set(mesh);
  }

  return KR_SUCCESS;
}

std::string KRModel::getElementName()
{
  return "model";
}

tinyxml2::XMLElement* KRModel::saveXML(tinyxml2::XMLNode* parent)
{
  tinyxml2::XMLElement* e = KRNode::saveXML(parent);
  e->SetAttribute("mesh", m_meshes[0].getName().c_str());
  for (int lod = 1; lod < kMeshLODCount; lod++) {
    char attribName[8];
    snprintf(attribName, 8, "mesh%i", lod);
    e->SetAttribute(attribName, m_meshes[lod].getName().c_str());
  }
  e->SetAttribute("light_map", m_lightMap.getName().c_str());
  e->SetAttribute("lod_min_coverage", m_min_lod_coverage);
  e->SetAttribute("receives_shadow", m_receivesShadow ? "true" : "false");
  e->SetAttribute("faces_camera", m_faces_camera ? "true" : "false");
  kraken::setXMLAttribute("rim_color", e, m_rim_color, Vector3::Zero());
  e->SetAttribute("rim_power", m_rim_power);
  return e;
}

void KRModel::setRimColor(const Vector3& rim_color)
{
  m_rim_color = rim_color;
}

void KRModel::setRimPower(float rim_power)
{
  m_rim_power = rim_power;
}

Vector3 KRModel::getRimColor()
{
  return m_rim_color;
}

float KRModel::getRimPower()
{
  return m_rim_power;
}

void KRModel::setLightMap(const std::string& name)
{
  m_lightMap.setName(name);
}

std::string KRModel::getLightMap()
{
  return m_lightMap.getName();
}

void KRModel::loadModel()
{
  bool meshChanged = false;
  for (int lod = 0; lod < kMeshLODCount; lod++) {
    KRMesh* prevMesh = nullptr;
    prevMesh = m_meshes[lod].get();
    m_meshes[lod].load(&getContext());
    if (m_meshes[lod].get() != prevMesh) {
      meshChanged = true;
    }
    if (m_meshes[lod].isLoaded()) {
      KRMesh* model = m_meshes[lod].get();
      std::vector<KRBone*> model_bones;
      int bone_count = model->getBoneCount();
      bool all_bones_found = true;
      for (int bone_index = 0; bone_index < bone_count; bone_index++) {
        KRBone* matching_bone = dynamic_cast<KRBone*>(getScene().getRootNode()->find<KRNode>(model->getBoneName(bone_index)));
        if (matching_bone) {
          model_bones.push_back(matching_bone);
        } else {
          all_bones_found = false; // Reject when there are any missing bones or multiple matches
        }
      }
      if (all_bones_found) {
        if (m_bones[lod] != model_bones) {
          m_bones[lod] = model_bones;
          meshChanged = true;
        }
      } else {
        if (!m_bones[lod].empty()) {
          m_bones[lod].clear();
          meshChanged = true;
        }
      }
    } else {
      if (!m_bones[lod].empty()) {
        m_bones[lod].clear();
        meshChanged = true;
      }
    }
  }
  
  if (meshChanged) {
    getScene().notify_sceneGraphModify(this);
    invalidateBounds();
  }
}

void KRModel::render(KRNode::RenderInfo& ri)
{
  if (ri.renderPass->getType() == RenderPassType::RENDER_PASS_PRESTREAM) {
    if (m_lod_visible >= LOD_VISIBILITY_PRESTREAM) {
      preStream(*ri.viewport);
    }
  }
  if (m_lod_visible <= LOD_VISIBILITY_PRESTREAM) return;

  ri.reflectedObjects.push_back(this);

  KRNode::render(ri);

  // Don't render meshes on second pass of the deferred lighting renderer, as only lights will be applied
  if (ri.renderPass->getType() != RenderPassType::RENDER_PASS_DEFERRED_LIGHTS
    && ri.renderPass->getType() != RenderPassType::RENDER_PASS_ADDITIVE_PARTICLES
    && ri.renderPass->getType() != RenderPassType::RENDER_PASS_PARTICLE_OCCLUSION
    && ri.renderPass->getType()!= RenderPassType::RENDER_PASS_VOLUMETRIC_EFFECTS_ADDITIVE
    && ri.renderPass->getType() != RenderPassType::RENDER_PASS_SHADOWMAP
    && ri.renderPass->getType() != RenderPassType::RENDER_PASS_PRESTREAM) {
    loadModel();

    /*
    float lod_coverage = 0.0f;
    if(m_models.size() > 1) {
        lod_coverage = viewport.coverage(getBounds()); // This also checks the view frustrum culling
    } else if(viewport.visible(getBounds())) {
        lod_coverage = 1.0f;
    }
    */

    float lod_coverage = ri.viewport->coverage(getBounds()); // This also checks the view frustrum culling
    if (lod_coverage > m_min_lod_coverage) {
      // ---===--- Select the best LOD model based on screen coverage ---===---
      int bestLOD = -1;
      KRMesh* pModel = nullptr;
      for (int lod = 0; lod < kMeshLODCount; lod++) {
        if (m_meshes[lod].isLoaded()) {
          KRMesh* pLODModel = m_meshes[lod].get();

          if ((float)pLODModel->getLODCoverage() / 100.0f > lod_coverage) {
            if(bestLOD == -1 || pLODModel->getLODCoverage() < pModel->getLODCoverage()) {
              pModel = pLODModel;
              bestLOD = lod;
              continue;
            }
          }
        }
      }

      m_lightMap.load(&getContext());

      if (m_lightMap.isLoaded() && ri.camera->settings.bEnableLightMap && ri.renderPass->getType() != RENDER_PASS_SHADOWMAP && ri.renderPass->getType() != RENDER_PASS_SHADOWMAP) {
        m_lightMap.get()->resetPoolExpiry(lod_coverage, KRTexture::TEXTURE_USAGE_LIGHT_MAP);
        // TODO - Vulkan refactoring.  We need to bind the shadow map in KRMesh::Render
        // m_pContext->getTextureManager()->selectTexture(5, m_pLightMap, lod_coverage, KRTexture::TEXTURE_USAGE_LIGHT_MAP);
      }

      if (pModel) {
        Matrix4 matModel = getModelMatrix();
        if (m_faces_camera) {
          Vector3 model_center = Matrix4::Dot(matModel, Vector3::Zero());
          Vector3 camera_pos = ri.viewport->getCameraPosition();
          matModel = Quaternion::Create(Vector3::Forward(), Vector3::Normalize(camera_pos - model_center)).rotationMatrix() * matModel;
        }

        pModel->render(ri, getName(), matModel, m_lightMap.get(), m_bones[bestLOD], lod_coverage);
      }
    }
  }

  ri.reflectedObjects.pop_back();
}

void KRModel::preStream(const KRViewport& viewport)
{
  loadModel();
  float lod_coverage = viewport.coverage(getBounds());

  for (int i = 0; i < kMeshLODCount; i++) {
    if (m_meshes[i].isLoaded()) {
      m_meshes[i].get()->preStream(lod_coverage);
    }
  }

  m_lightMap.load(&getContext());

  if (m_lightMap.isLoaded()) {
    m_lightMap.get()->resetPoolExpiry(lod_coverage, KRTexture::TEXTURE_USAGE_LIGHT_MAP);
  }
}


kraken_stream_level KRModel::getStreamLevel(const KRViewport& viewport)
{
  kraken_stream_level stream_level = KRNode::getStreamLevel(viewport);

  loadModel();

  for (int lod = 0; lod < kMeshLODCount; lod++) {
    if (m_meshes[lod].isLoaded()) {
        stream_level = KRMIN(stream_level, m_meshes[lod].get()->getStreamLevel());
    }
  }

  return stream_level;
}

AABB KRModel::getBounds()
{
  loadModel();

  // Get the bounds of the lowest lod mesh
  for(int lod=0; lod<kMeshLODCount; lod++) {
    if (!m_meshes[lod].isLoaded()) {
      continue;
    }
    KRMesh* mesh = m_meshes[lod].get();
    if (m_faces_camera) {
      AABB normal_bounds = AABB::Create(mesh->getMinPoint(), mesh->getMaxPoint(), getModelMatrix());
      float max_dimension = normal_bounds.longest_radius();
      return AABB::Create(normal_bounds.center() - Vector3::Create(max_dimension), normal_bounds.center() + Vector3::Create(max_dimension));
    } else {
      if (!(m_boundsCachedMat == getModelMatrix())) {
        m_boundsCachedMat = getModelMatrix();
        m_boundsCached = AABB::Create(mesh->getMinPoint(), mesh->getMaxPoint(), getModelMatrix());
      }
      return m_boundsCached;
    }
  }

  // No models loaded
  return AABB::Infinite();
}


bool KRModel::getShaderValue(ShaderValue value, hydra::Vector3* output) const
{
  switch (value) {
  case ShaderValue::rim_color:
    *output = m_rim_color;
    return true;
  }
  return KRNode::getShaderValue(value, output);
}

bool KRModel::getShaderValue(ShaderValue value, float* output) const
{
  switch (value) {
  case ShaderValue::rim_power:
    *output = m_rim_power;
    return true;
  }
  return KRNode::getShaderValue(value, output);
}


