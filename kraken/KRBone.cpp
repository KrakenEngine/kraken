//
//  KRBone.cpp
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

#include "KRBone.h"
#include "KRContext.h"

using namespace hydra;

/* static */
void KRBone::InitNodeInfo(KrNodeInfo* nodeInfo)
{
  KRNode::InitNodeInfo(nodeInfo);
  // No additional members
}

KRBone::KRBone(KRScene& scene, std::string name) : KRNode(scene, name)
{
  setScaleCompensation(true);
}

KRBone::~KRBone()
{}

std::string KRBone::getElementName()
{
  return "bone";
}

tinyxml2::XMLElement* KRBone::saveXML(tinyxml2::XMLNode* parent)
{
  tinyxml2::XMLElement* e = KRNode::saveXML(parent);

  return e;
}

void KRBone::loadXML(tinyxml2::XMLElement* e)
{
  KRNode::loadXML(e);
  setScaleCompensation(true);
}

AABB KRBone::getBounds()
{
  return AABB::Create(-Vector3::One(), Vector3::One(), getModelMatrix()); // Only required for bone debug visualization
}

void KRBone::render(RenderInfo& ri)
{
  if (m_lod_visible <= LOD_VISIBILITY_PRESTREAM) return;

  KRNode::render(ri);

  bool bVisualize = ri.camera->settings.debug_display == KRRenderSettings::KRENGINE_DEBUG_DISPLAY_BONES;

  if (ri.renderPass == KRNode::RENDER_PASS_FORWARD_TRANSPARENT && bVisualize) {
    KRMesh* sphereModel = getContext().getMeshManager()->getMaxLODModel("__sphere");
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
      info.rasterMode = RasterMode::kAdditiveNoTest;
      info.modelFormat = sphereModel->getModelFormat();
      info.vertexAttributes = sphereModel->getVertexAttributes();

      KRPipeline* pShader = getContext().getPipelineManager()->getPipeline(*ri.surface, info);
      pShader->bind(ri.commandBuffer, *ri.camera, ri.viewport, sphereModelMatrix, &ri.point_lights, &ri.directional_lights, &ri.spot_lights, ri.renderPass);

      sphereModel->renderNoMaterials(ri.commandBuffer, ri.renderPass, getName(), "visualize_overlay", 1.0f);
    } // sphereModel
  }
}


void KRBone::setBindPose(const Matrix4& pose)
{
  m_bind_pose = pose;
}
const Matrix4& KRBone::getBindPose()
{
  return m_bind_pose;
}
