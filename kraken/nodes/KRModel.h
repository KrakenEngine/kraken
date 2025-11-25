//
//  KRModel.h
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

#include "resources/mesh/KRMesh.h"
#include "KRModel.h"
#include "KRCamera.h"
#include "resources/mesh/KRMeshManager.h"
#include "KRNode.h"
#include "KRContext.h"
#include "resources/mesh/KRMesh.h"
#include "resources/mesh/KRMeshBinding.h"
#include "resources/texture/KRTexture.h"
#include "resources/texture/KRTextureBinding.h"
#include "KRBone.h"

class KRModel : public KRNode
{

public:
  static const int kMeshLODCount = 8;

  static void InitNodeInfo(KrNodeInfo* nodeInfo);

  KRModel(KRScene& scene, std::string name);
  virtual ~KRModel();
  
  KrResult update(const KrNodeInfo* nodeInfo) override;

  virtual std::string getElementName();
  virtual tinyxml2::XMLElement* saveXML(tinyxml2::XMLNode* parent) override;
  virtual void loadXML(tinyxml2::XMLElement* e) override;

  virtual void render(KRNode::RenderInfo& ri) override;
  virtual void getResourceBindings(std::list<KRResourceBinding*>& bindings) override;
  virtual void preStream(const KRViewport& viewport, std::list<KRResourceRequest>& resourceRequests) override;

  virtual hydra::AABB getBounds();

  void setRimColor(const hydra::Vector3& rim_color);
  void setRimPower(float rim_power);
  hydra::Vector3 getRimColor();
  float getRimPower();

  void setLightMap(const std::string& name);
  std::string getLightMap();

  virtual kraken_stream_level getStreamLevel(const KRViewport& viewport);

private:

  std::array<KRMeshBinding, kMeshLODCount> m_meshes;
  
  KRNODE_PROPERTY(KRTextureBinding, m_lightMap, KRTexture::TEXTURE_USAGE_LIGHT_MAP, "light_map");
  KRNODE_PROPERTY(float, m_min_lod_coverage, 0.f, "min_lod_coverage");
  KRNODE_PROPERTY(bool, m_receivesShadow, true, "receives_shadow");
  KRNODE_PROPERTY(bool, m_faces_camera, false, "faces_camera");
  KRNODE_PROPERTY(float, m_rim_power, 0.f, "rim_power");
  KRNODE_PROPERTY(hydra::Vector3, m_rim_color, hydra::Vector3({ 0.f, 0.f, 0.f }), "rim_color");

  std::array<std::vector<KRBone*>, kMeshLODCount> m_bones; // Connects model to set of bones
  hydra::Matrix4 m_boundsCachedMat;
  hydra::AABB m_boundsCached;

  void loadModel();
  

private:
  bool getShaderValue(ShaderValue value, hydra::Vector3* output) const override;
  bool getShaderValue(ShaderValue value, float* output) const override;
};
