//
//  KRPointLight.cpp
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

#include "KRPointLight.h"
#include "KRCamera.h"
#include "KRContext.h"
#include "KRRenderPass.h"

using namespace hydra;

/* static */
void KRPointLight::InitNodeInfo(KrNodeInfo* nodeInfo)
{
  KRLight::InitNodeInfo(nodeInfo);
  // No additional members
}

KRPointLight::KRPointLight(KRScene& scene, std::string name) : KRLight(scene, name)
{
  m_sphereVertices = NULL;
  m_cVertices = 0;
}

KRPointLight::~KRPointLight()
{
  if (m_sphereVertices) {
    delete m_sphereVertices;
    m_cVertices = 0;
  }
}

std::string KRPointLight::getElementName()
{
  return "point_light";
}

AABB KRPointLight::getBounds()
{
  float influence_radius = m_decayStart - sqrt(m_intensity * 0.01f) / sqrt(KRLIGHT_MIN_INFLUENCE);
  if (influence_radius < m_flareOcclusionSize) {
    influence_radius = m_flareOcclusionSize;
  }
  return AABB::Create(Vector3::Create(-influence_radius), Vector3::Create(influence_radius), getModelMatrix());
}

void KRPointLight::render(RenderInfo& ri)
{
  if (m_lod_visible <= LOD_VISIBILITY_PRESTREAM) return;

  KRLight::render(ri);

  bool bVisualize = ri.renderPass->getType() == RenderPassType::RENDER_PASS_FORWARD_TRANSPARENT && ri.camera->settings.bShowDeferred;

  if (ri.renderPass->getType() == RenderPassType::RENDER_PASS_DEFERRED_LIGHTS || bVisualize) {
    // Lights are rendered on the second pass of the deferred renderer

    std::vector<KRPointLight*> this_light;
    this_light.push_back(this);

    Vector3 light_position = getLocalTranslation();

    float influence_radius = m_decayStart - sqrt(m_intensity * 0.01f) / sqrt(KRLIGHT_MIN_INFLUENCE);

    Matrix4 sphereModelMatrix = Matrix4();
    sphereModelMatrix.scale(influence_radius);
    sphereModelMatrix.translate(light_position.x, light_position.y, light_position.z);

    if (ri.viewport->visible(getBounds())) { // Cull out any lights not within the view frustrum

      Vector3 view_light_position = Matrix4::Dot(ri.viewport->getViewMatrix(), light_position);

      bool bInsideLight = view_light_position.sqrMagnitude() <= (influence_radius + ri.camera->settings.getPerspectiveNearZ()) * (influence_radius + ri.camera->settings.getPerspectiveNearZ());

      std::string shader_name(bVisualize ? "visualize_overlay" : (bInsideLight ? "light_point_inside" : "light_point"));
      PipelineInfo info{};
      info.shader_name = &shader_name;
      info.pCamera = ri.camera;
      info.point_lights = &this_light;
      info.renderPass = ri.renderPass;
      if (bInsideLight) {
        info.rasterMode = bVisualize ? RasterMode::kAdditiveNoTest : RasterMode::kAlphaBlendNoTest;
      } else {
        info.rasterMode = bVisualize ? RasterMode::kAdditive : RasterMode::kAlphaBlend;
      }
      info.vertexAttributes = bInsideLight ? m_pContext->getMeshManager()->KRENGINE_VBO_DATA_2D_SQUARE_VERTICES.getVertexAttributes() : 1 << KRMesh::KRENGINE_ATTRIB_VERTEX;
      info.modelFormat = bInsideLight ? ModelFormat::KRENGINE_MODEL_FORMAT_STRIP : ModelFormat::KRENGINE_MODEL_FORMAT_TRIANGLES;

      KRPipeline* pShader = getContext().getPipelineManager()->getPipeline(*ri.surface, info);
      pShader->setPushConstant(ShaderValue::light_color, m_color);
      pShader->setPushConstant(ShaderValue::light_intensity, m_intensity * 0.01f);
      pShader->setPushConstant(ShaderValue::light_decay_start, getDecayStart());
      pShader->setPushConstant(ShaderValue::light_cutoff, KRLIGHT_MIN_INFLUENCE);
      pShader->setPushConstant(ShaderValue::light_position, light_position);
      pShader->bind(ri, sphereModelMatrix); // TODO: Pass light index to shader

      if (bInsideLight) {
        // Render a full screen quad
        m_pContext->getMeshManager()->bindVBO(ri.commandBuffer, &m_pContext->getMeshManager()->KRENGINE_VBO_DATA_2D_SQUARE_VERTICES, 1.0f);
        vkCmdDraw(ri.commandBuffer, 4, 1, 0, 0);
      } else {
        // Render sphere of light's influence
        generateMesh();

        GLDEBUG(glVertexAttribPointer(KRMesh::KRENGINE_ATTRIB_VERTEX, 3, GL_FLOAT, 0, 0, m_sphereVertices));

        vkCmdDraw(ri.commandBuffer, m_cVertices, 1, 0, 0);
      }

    }
  }
}

void KRPointLight::generateMesh()
{
  // Create a triangular facet approximation to a sphere
  // Based on algorithm from Paul Bourke: http://paulbourke.net/miscellaneous/sphere_cylinder/

  int iterations = 3;
  int facet_count = (int)(pow(4, iterations) * 8);

  if (m_cVertices != facet_count * 3) {
    if (m_sphereVertices) {
      free(m_sphereVertices);
      m_sphereVertices = NULL;
    }

    m_cVertices = facet_count * 3;


    class Facet3
    {
    public:
      Facet3()
      {

      }
      ~Facet3()
      {

      }
      Vector3 p1;
      Vector3 p2;
      Vector3 p3;
    };

    std::vector<Facet3> f = std::vector<Facet3>(facet_count);

    int i, it;
    float a;
    Vector3 p[6] = {
        Vector3::Create(0,0,1),
        Vector3::Create(0,0,-1),
        Vector3::Create(-1,-1,0),
        Vector3::Create(1,-1,0),
        Vector3::Create(1,1,0),
        Vector3::Create(-1,1,0)
    };

    Vector3 pa, pb, pc;
    int nt = 0, ntold;

    /* Create the level 0 object */
    a = 1.0f / sqrtf(2.0f);
    for (i = 0; i < 6; i++) {
      p[i].x *= a;
      p[i].y *= a;
    }
    f[0].p1 = p[0]; f[0].p2 = p[3]; f[0].p3 = p[4];
    f[1].p1 = p[0]; f[1].p2 = p[4]; f[1].p3 = p[5];
    f[2].p1 = p[0]; f[2].p2 = p[5]; f[2].p3 = p[2];
    f[3].p1 = p[0]; f[3].p2 = p[2]; f[3].p3 = p[3];
    f[4].p1 = p[1]; f[4].p2 = p[4]; f[4].p3 = p[3];
    f[5].p1 = p[1]; f[5].p2 = p[5]; f[5].p3 = p[4];
    f[6].p1 = p[1]; f[6].p2 = p[2]; f[6].p3 = p[5];
    f[7].p1 = p[1]; f[7].p2 = p[3]; f[7].p3 = p[2];
    nt = 8;

    /* Bisect each edge and move to the surface of a unit sphere */
    for (it = 0; it < iterations; it++) {
      ntold = nt;
      for (i = 0; i < ntold; i++) {
        pa.x = (f[i].p1.x + f[i].p2.x) / 2;
        pa.y = (f[i].p1.y + f[i].p2.y) / 2;
        pa.z = (f[i].p1.z + f[i].p2.z) / 2;
        pb.x = (f[i].p2.x + f[i].p3.x) / 2;
        pb.y = (f[i].p2.y + f[i].p3.y) / 2;
        pb.z = (f[i].p2.z + f[i].p3.z) / 2;
        pc.x = (f[i].p3.x + f[i].p1.x) / 2;
        pc.y = (f[i].p3.y + f[i].p1.y) / 2;
        pc.z = (f[i].p3.z + f[i].p1.z) / 2;
        pa.normalize();
        pb.normalize();
        pc.normalize();
        f[nt].p1 = f[i].p1; f[nt].p2 = pa; f[nt].p3 = pc; nt++;
        f[nt].p1 = pa; f[nt].p2 = f[i].p2; f[nt].p3 = pb; nt++;
        f[nt].p1 = pb; f[nt].p2 = f[i].p3; f[nt].p3 = pc; nt++;
        f[i].p1 = pa;
        f[i].p2 = pb;
        f[i].p3 = pc;
      }
    }

    m_sphereVertices = (float*)malloc(sizeof(float) * m_cVertices * 3);
    assert(m_sphereVertices != NULL);
    float* pDest = m_sphereVertices;
    for (int facet_index = 0; facet_index < facet_count; facet_index++) {
      *pDest++ = f[facet_index].p1.x;
      *pDest++ = f[facet_index].p1.y;
      *pDest++ = f[facet_index].p1.z;
      *pDest++ = f[facet_index].p2.x;
      *pDest++ = f[facet_index].p2.y;
      *pDest++ = f[facet_index].p2.z;
      *pDest++ = f[facet_index].p3.x;
      *pDest++ = f[facet_index].p3.y;
      *pDest++ = f[facet_index].p3.z;
    }
  }
}
