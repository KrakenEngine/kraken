//
//  KRCollider.cpp
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
#include "KRCollider.h"
#include "KRContext.h"
#include "resources/mesh/KRMesh.h"
#include "KRRenderPass.h"

using namespace hydra;

/* static */
void KRCollider::InitNodeInfo(KrNodeInfo* nodeInfo)
{
  KRNode::InitNodeInfo(nodeInfo);
  nodeInfo->collider.audio_occlusion = 1.0f;
  nodeInfo->collider.layer_mask = 65535;
  nodeInfo->collider.mesh = -1;
}

KRCollider::KRCollider(KRScene& scene, std::string name)
  : KRNode(scene, name)
  , m_layer_mask(0xffff)
  , m_audio_occlusion(1.0f)
{
  
}

KRCollider::KRCollider(KRScene& scene, std::string collider_name, std::string model_name, unsigned int layer_mask, float audio_occlusion)
  : KRNode(scene, collider_name)
  , m_layer_mask(layer_mask)
  , m_audio_occlusion(audio_occlusion)
{
  m_model.setName(model_name);
}

KRCollider::~KRCollider()
{

}

std::string KRCollider::getElementName()
{
  return "collider";
}

tinyxml2::XMLElement* KRCollider::saveXML(tinyxml2::XMLNode* parent)
{
  tinyxml2::XMLElement* e = KRNode::saveXML(parent);
  e->SetAttribute("mesh", m_model.getName().c_str());
  e->SetAttribute("layer_mask", m_layer_mask);
  e->SetAttribute("audio_occlusion", m_audio_occlusion);
  return e;
}

void KRCollider::loadXML(tinyxml2::XMLElement* e)
{
  KRNode::loadXML(e);
  m_model.setName(e->Attribute("mesh"));

  m_layer_mask = 65535;
  if (e->QueryUnsignedAttribute("layer_mask", &m_layer_mask) != tinyxml2::XML_SUCCESS) {
    m_layer_mask = 65535;
  }

  m_audio_occlusion = 1.0f;
  if (e->QueryFloatAttribute("audio_occlusion", &m_audio_occlusion) != tinyxml2::XML_SUCCESS) {
    m_audio_occlusion = 1.0f;
  }
}

void KRCollider::loadModel()
{
  KRMesh* prevModel = m_model.get();
  m_model.load(&getContext());
  if (m_model.get() != prevModel) {
    getScene().notify_sceneGraphModify(this);
  }
}

AABB KRCollider::getBounds()
{
  loadModel();
  if (m_model.isLoaded()) {
    return AABB::Create(m_model.get()->getMinPoint(), m_model.get()->getMaxPoint(), getModelMatrix());
  } else {
    return AABB::Infinite();
  }
}

bool KRCollider::lineCast(const Vector3& v0, const Vector3& v1, HitInfo& hitinfo, unsigned int layer_mask)
{
  if (layer_mask & m_layer_mask) { // Only test if layer masks have a common bit set
    loadModel();
    if (m_model.isLoaded()) {
      if (getBounds().intersectsLine(v0, v1)) {
        Vector3 v0_model_space = Matrix4::Dot(getInverseModelMatrix(), v0);
        Vector3 v1_model_space = Matrix4::Dot(getInverseModelMatrix(), v1);
        HitInfo hitinfo_model_space;
        if (hitinfo.didHit()) {
          Vector3 hit_position_model_space = Matrix4::Dot(getInverseModelMatrix(), hitinfo.getPosition());
          hitinfo_model_space = HitInfo(hit_position_model_space, Matrix4::DotNoTranslate(getInverseModelMatrix(), hitinfo.getNormal()), (hit_position_model_space - v0_model_space).magnitude(), hitinfo.getNode());
        }

        if (m_model.get()->lineCast(v0_model_space, v1_model_space, hitinfo_model_space)) {
          Vector3 hit_position_world_space = Matrix4::Dot(getModelMatrix(), hitinfo_model_space.getPosition());
          hitinfo = HitInfo(hit_position_world_space, Vector3::Normalize(Matrix4::DotNoTranslate(getModelMatrix(), hitinfo_model_space.getNormal())), (hit_position_world_space - v0).magnitude(), this);
          return true;
        }
      }
    }
  }
  return false;
}

bool KRCollider::rayCast(const Vector3& v0, const Vector3& dir, HitInfo& hitinfo, unsigned int layer_mask)
{
  if (layer_mask & m_layer_mask) { // Only test if layer masks have a common bit set
    loadModel();
    if (m_model.isLoaded()) {
      if (getBounds().intersectsRay(v0, dir)) {
        Vector3 v0_model_space = Matrix4::Dot(getInverseModelMatrix(), v0);
        Vector3 dir_model_space = Vector3::Normalize(Matrix4::DotNoTranslate(getInverseModelMatrix(), dir));
        HitInfo hitinfo_model_space;
        if (hitinfo.didHit()) {
          Vector3 hit_position_model_space = Matrix4::Dot(getInverseModelMatrix(), hitinfo.getPosition());
          hitinfo_model_space = HitInfo(hit_position_model_space, Vector3::Normalize(Matrix4::DotNoTranslate(getInverseModelMatrix(), hitinfo.getNormal())), (hit_position_model_space - v0_model_space).magnitude(), hitinfo.getNode());
        }

        if (m_model.get()->rayCast(v0_model_space, dir_model_space, hitinfo_model_space)) {
          Vector3 hit_position_world_space = Matrix4::Dot(getModelMatrix(), hitinfo_model_space.getPosition());
          hitinfo = HitInfo(hit_position_world_space, Vector3::Normalize(Matrix4::DotNoTranslate(getModelMatrix(), hitinfo_model_space.getNormal())), (hit_position_world_space - v0).magnitude(), this);
          return true;
        }
      }
    }
  }
  return false;
}

bool KRCollider::sphereCast(const Vector3& v0, const Vector3& v1, float radius, HitInfo& hitinfo, unsigned int layer_mask)
{
  if (layer_mask & m_layer_mask) { // Only test if layer masks have a common bit set
    loadModel();
    if (m_model.isLoaded()) {
      AABB sphereCastBounds = AABB::Create( // TODO - Need to cache this; perhaps encasulate within a "spherecast" class to be passed through these functions
          Vector3::Create(KRMIN(v0.x, v1.x) - radius, KRMIN(v0.y, v1.y) - radius, KRMIN(v0.z, v1.z) - radius),
          Vector3::Create(KRMAX(v0.x, v1.x) + radius, KRMAX(v0.y, v1.y) + radius, KRMAX(v0.z, v1.z) + radius)
      );

      if (getBounds().intersects(sphereCastBounds)) {
        if (m_model.get()->sphereCast(getModelMatrix(), v0, v1, radius, hitinfo)) {
          hitinfo = HitInfo(hitinfo.getPosition(), hitinfo.getNormal(), hitinfo.getDistance(), this);
          return true;
        }
      }
    }
  }
  return false;
}

unsigned int KRCollider::getLayerMask()
{
  return m_layer_mask;
}

void KRCollider::setLayerMask(unsigned int layer_mask)
{
  m_layer_mask = layer_mask;
}

float KRCollider::getAudioOcclusion()
{
  return m_audio_occlusion;
}

void KRCollider::setAudioOcclusion(float audio_occlusion)
{
  m_audio_occlusion = audio_occlusion;
}


void KRCollider::render(RenderInfo& ri)
{
  KRNode::render(ri);

  if (ri.renderPass->getType() == RenderPassType::RENDER_PASS_FORWARD_TRANSPARENT && ri.camera->settings.debug_display == KRRenderSettings::KRENGINE_DEBUG_DISPLAY_COLLIDERS) {
    loadModel();
    if (m_model.isLoaded()) {

      GL_PUSH_GROUP_MARKER("Debug Overlays");

      PipelineInfo info{};
      std::string shader_name("visualize_overlay");
      info.shader_name = &shader_name;
      info.pCamera = ri.camera;
      info.point_lights = &ri.point_lights;
      info.directional_lights = &ri.directional_lights;
      info.spot_lights = &ri.spot_lights;
      info.renderPass = ri.renderPass;
      info.rasterMode = RasterMode::kAdditive;
      info.modelFormat = m_model.get()->getModelFormat();
      info.vertexAttributes = m_model.get()->getVertexAttributes();

      KRPipeline* pShader = getContext().getPipelineManager()->getPipeline(*ri.surface, info);

      pShader->bind(ri, getModelMatrix());

      m_model.get()->renderNoMaterials(ri.commandBuffer, ri.renderPass, getName(), "visualize_overlay", 1.0f);

      GL_POP_GROUP_MARKER;
    }
  }
}
