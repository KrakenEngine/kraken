//
//  KRSettings.h
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

#pragma once

#include "KREngine-common.h"

#include "KRPipeline.h"
#include "KRContextObject.h"
#include "resources/texture/KRTexture.h"
#include "resources/texture/KRTextureBinding.h"
#include "KRContext.h"
#include "KRViewport.h"
#include "KRRenderSettings.h"
#include "resources/mesh/KRMeshManager.h"

#define KRAKEN_FPS_AVERAGE_FRAME_COUNT 30

class KRModel;
class KRScene;
class KRViewport;
class KRSurface;
class KRRenderGraph;

class KRCamera : public KRNode
{
public:
  static void InitNodeInfo(KrNodeInfo* nodeInfo);
  KRCamera(KRScene& scene, std::string name);
  virtual ~KRCamera();

  KrResult update(const KrNodeInfo* nodeInfo) override;

  void renderFrame(VkCommandBuffer& commandBuffer, KRSurface& compositeSurface, KRRenderGraph& renderGraph);

  void preStream(const KRViewport& viewport, std::list<KRResourceRequest>& resourceRequests) final;
  void render(KRNode::RenderInfo& ri) final;

  KRRenderSettings settings;

  KRViewport* getViewport();


  virtual std::string getElementName() override;
  virtual tinyxml2::XMLElement* saveXML(tinyxml2::XMLNode* parent) override;
  virtual void loadXML(tinyxml2::XMLElement* e) override;

  std::string getDebugText();

  void flushSkybox();     // this will delete the skybox and cause the camera to reload a new skybox based on the settings

  void setFadeColor(const hydra::Vector4& fade_color);
  hydra::Vector4 getFadeColor();

  void setSkyBox(const std::string& skyBox);
  const std::string getSkyBox() const;

protected:
  bool getShaderValue(ShaderValue value, hydra::Vector4* output) const override;

private:
  void createBuffers(int renderBufferWidth, int renderBufferHeight);

  int volumetricBufferWidth, volumetricBufferHeight;

  int compositeFramebuffer, compositeDepthTexture, compositeColorTexture;
  int lightAccumulationBuffer, lightAccumulationTexture;


  int volumetricLightAccumulationBuffer, volumetricLightAccumulationTexture;

  void renderPost(RenderInfo& ri);
  void renderDebug(RenderInfo& ri);

  void destroyBuffers();

  KrSurfaceHandle m_surfaceHandle;

  KRTextureBinding m_skyBox;
  KRViewport m_viewport;

  float m_particlesAbsoluteTime;

  hydra::Vector4 m_fade_color;

  typedef struct
  {
    float x;
    float y;
    float z;
    float u;
    float v;
  } DebugTextVertexData;

  mimir::Block m_debug_text_vertices;
  KRMeshManager::KRVBOData m_debug_text_vbo_data;

  uint64_t m_last_frame_start;
  int m_frame_times[KRAKEN_FPS_AVERAGE_FRAME_COUNT];
  int m_frame_times_filled;
};
