//
//  KRReverbZone.cpp
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

#include "KRReverbZone.h"
#include "KRContext.h"
#include "KRRenderPass.h"

using namespace hydra;

/* static */
void KRReverbZone::InitNodeInfo(KrNodeInfo* nodeInfo)
{
  KRNode::InitNodeInfo(nodeInfo);
  nodeInfo->reverb_zone.gain = decltype(m_reverb_gain)::defaultVal;
  nodeInfo->reverb_zone.gradient = decltype(m_gradient_distance)::defaultVal;
  nodeInfo->reverb_zone.sample = -1;
  nodeInfo->reverb_zone.pZoneName = decltype(m_zone)::defaultVal;
}

KRReverbZone::KRReverbZone(KRScene& scene, std::string name) : KRNode(scene, name)
{
}

KRReverbZone::~KRReverbZone()
{}

std::string KRReverbZone::getElementName()
{
  return "reverb_zone";
}

tinyxml2::XMLElement* KRReverbZone::saveXML(tinyxml2::XMLNode* parent)
{
  tinyxml2::XMLElement* e = KRNode::saveXML(parent);
  m_zone.save(e);
  m_reverb.save(e);
  m_reverb_gain.save(e);
  m_gradient_distance.save(e);
  return e;
}

void KRReverbZone::loadXML(tinyxml2::XMLElement* e)
{
  KRNode::loadXML(e);

  m_zone.load(e);
  m_gradient_distance.load(e);
  m_reverb.load(e);
  m_reverb_gain.load(e);
}

KRAudioSample* KRReverbZone::getReverb()
{
  m_reverb.val.bind(&getContext());
  return m_reverb.val.get();
}

void KRReverbZone::setReverb(const std::string& reverb)
{
  m_reverb = reverb;
}

float KRReverbZone::getReverbGain()
{
  return m_reverb_gain;
}

void KRReverbZone::setReverbGain(float reverb_gain)
{
  m_reverb_gain = reverb_gain;
}

std::string KRReverbZone::getZone()
{
  return m_zone;
}

void KRReverbZone::setZone(const std::string& zone)
{
  m_zone = zone;
}

void KRReverbZone::render(RenderInfo& ri)
{
  KRNode::render(ri);

  bool bVisualize = ri.camera->settings.debug_display == KRRenderSettings::KRENGINE_DEBUG_DISPLAY_SIREN_REVERB_ZONES;

  if (ri.renderPass->getType()== RenderPassType::RENDER_PASS_FORWARD_TRANSPARENT && bVisualize) {
    KRMesh* sphereModel = getContext().getMeshManager()->getMesh("__sphere");
    if (sphereModel) {
      Matrix4 sphereModelMatrix = getModelMatrix();
      PipelineInfo info{};
      std::string shader_name("visualize_overlay");
      info.shader_name = &shader_name;
      info.pCamera = ri.camera;
      info.point_lights = &ri.point_lights;
      info.directional_lights = &ri.directional_lights;
      info.spot_lights = &ri.spot_lights;
      info.renderPass = ri.renderPass;
      info.rasterMode = RasterMode::kAlphaBlend;
      info.modelFormat = sphereModel->getModelFormat();
      info.vertexAttributes = sphereModel->getVertexAttributes();

      KRPipeline* pShader = getContext().getPipelineManager()->getPipeline(*ri.surface, info);

      pShader->bind(ri, sphereModelMatrix);

      sphereModel->renderNoMaterials(ri.commandBuffer, ri.renderPass, getName(), "visualize_overlay", 1.0f);
    } // sphereModel
  }
}


float KRReverbZone::getGradientDistance()
{
  return m_gradient_distance;
}

void KRReverbZone::setGradientDistance(float gradient_distance)
{
  m_gradient_distance = gradient_distance;
}

AABB KRReverbZone::getBounds()
{
  // Reverb zones always have a -1, -1, -1 to 1, 1, 1 bounding box
  return AABB::Create(-Vector3::One(), Vector3::One(), getModelMatrix());
}

float KRReverbZone::getContainment(const Vector3& pos)
{
  AABB bounds = getBounds();
  if (bounds.contains(pos)) {
    Vector3 size = bounds.size();
    Vector3 diff = pos - bounds.center();
    diff = diff * 2.0f;
    diff = Vector3::Create(diff.x / size.x, diff.y / size.y, diff.z / size.z);
    float d = diff.magnitude();

    if (m_gradient_distance <= 0.0f) {
      // Avoid division by zero
      d = d > 1.0f ? 0.0f : 1.0f;
    } else {
      d = (1.0f - d) / m_gradient_distance;
      d = KRCLAMP(d, 0.0f, 1.0f);
    }
    return d;

  } else {
    return 0.0f;
  }
}
