//
//  KRParticleSystemNewtonian.cpp
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
#include "KRParticleSystemNewtonian.h"
#include "KRContext.h"
#include "KRRenderPass.h"

using namespace hydra;

KRParticleSystemNewtonian::KRParticleSystemNewtonian(KRScene& scene, std::string name) : KRParticleSystem(scene, name)
{
  m_particlesAbsoluteTime = 0.0f;
}

KRParticleSystemNewtonian::~KRParticleSystemNewtonian()
{

}

std::string KRParticleSystemNewtonian::getElementName()
{
  return "newtonian_particles";
}

void KRParticleSystemNewtonian::loadXML(tinyxml2::XMLElement* e)
{
  KRParticleSystem::loadXML(e);
}

tinyxml2::XMLElement* KRParticleSystemNewtonian::saveXML(tinyxml2::XMLNode* parent)
{
  tinyxml2::XMLElement* e = KRParticleSystem::saveXML(parent);
  return e;
}


AABB KRParticleSystemNewtonian::getBounds()
{
  return AABB::Create(-Vector3::One(), Vector3::One(), getModelMatrix());
}

void KRParticleSystemNewtonian::physicsUpdate(float deltaTime)
{
  KRParticleSystem::physicsUpdate(deltaTime);
  m_particlesAbsoluteTime += deltaTime;
}

bool KRParticleSystemNewtonian::hasPhysics()
{
  return true;
}

void KRParticleSystemNewtonian::render(RenderInfo& ri)
{
  KRNode::render(ri);

  if (ri.renderPass->getType() == RenderPassType::RENDER_PASS_ADDITIVE_PARTICLES) {
    if (ri.viewport->visible(getBounds())) {

      int particle_count = 10000;
      PipelineInfo info{};
      std::string shader_name("dust_particle");
      info.shader_name = &shader_name;
      info.pCamera = ri.camera;
      info.point_lights = &ri.point_lights;
      info.directional_lights = &ri.directional_lights;
      info.spot_lights = &ri.spot_lights;
      info.renderPass = ri.renderPass;
      info.rasterMode = RasterMode::kAdditive;
      info.cullMode = CullMode::kCullNone;
      info.vertexAttributes = (1 << KRMesh::KRENGINE_ATTRIB_VERTEX) | (1 << KRMesh::KRENGINE_ATTRIB_TEXUVA);
      info.modelFormat = ModelFormat::KRENGINE_MODEL_FORMAT_TRIANGLES;

      KRPipeline* pParticleShader = m_pContext->getPipelineManager()->getPipeline(*ri.surface, info);
      pParticleShader->bind(ri, getModelMatrix());

      m_pContext->getMeshManager()->bindVBO(ri.commandBuffer, &m_pContext->getMeshManager()->KRENGINE_VBO_DATA_RANDOM_PARTICLES, 1.0f);

      vkCmdDraw(ri.commandBuffer, particle_count * 3, 1, 0, 0);
    }
  }
}

bool KRParticleSystemNewtonian::getShaderValue(ShaderValue value, float* output) const
{
  switch (value) {
  case ShaderValue::dust_particle_size:
    *output = 1.0f;
    return true;
  }
  return KRParticleSystem::getShaderValue(value, output);
}