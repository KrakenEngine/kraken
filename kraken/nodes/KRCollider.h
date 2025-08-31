//
//  KRCollider.h
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


#define KRAKEN_COLLIDER_PHYSICS 1
#define KRAKEN_COLLIDER_AUDIO 2

#include "resources/mesh/KRMesh.h"
#include "KRModel.h"
#include "KRCamera.h"
#include "resources/mesh/KRMeshManager.h"
#include "KRNode.h"
#include "KRContext.h"
#include "resources/mesh/KRMesh.h"

class KRCollider : public KRNode
{

public:
  static void InitNodeInfo(KrNodeInfo* nodeInfo);

  KRCollider(KRScene& scene, std::string name);
  KRCollider(KRScene& scene, std::string collider_name, std::string model_name, unsigned int layer_mask, float audio_occlusion);
  virtual ~KRCollider();

  virtual std::string getElementName();
  virtual tinyxml2::XMLElement* saveXML(tinyxml2::XMLNode* parent);
  virtual void loadXML(tinyxml2::XMLElement* e);
  virtual hydra::AABB getBounds();

  bool lineCast(const hydra::Vector3& v0, const hydra::Vector3& v1, hydra::HitInfo& hitinfo, unsigned int layer_mask);
  bool rayCast(const hydra::Vector3& v0, const hydra::Vector3& v1, hydra::HitInfo& hitinfo, unsigned int layer_mask);
  bool sphereCast(const hydra::Vector3& v0, const hydra::Vector3& v1, float radius, hydra::HitInfo& hitinfo, unsigned int layer_mask);

  unsigned int getLayerMask();
  void setLayerMask(unsigned int layer_mask);

  float getAudioOcclusion();
  void setAudioOcclusion(float audio_occlusion);

  void render(RenderInfo& ri) override;

private:
  KRMesh* m_model;
  std::string m_model_name;

  unsigned int m_layer_mask;
  float m_audio_occlusion;

  void loadModel();
};
