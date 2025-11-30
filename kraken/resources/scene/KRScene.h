//
//  KRScene.h
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

#include "nodes/KRModel.h"
#include "resources/mesh/KRMesh.h"
#include "nodes/KRCamera.h"
#include "resources/mesh/KRMeshManager.h"
#include "nodes/KRNode.h"
#include "nodes/KRLocator.h"
#include "nodes/KRAmbientZone.h"
#include "nodes/KRReverbZone.h"
#include "KROctree.h"
class KRModel;
class KRLight;
class KRSurface;
class KRRenderGraph;

using std::vector;

class KRScene : public KRResource
{
public:
  KRScene(KRContext& context, std::string name);
  virtual ~KRScene();



  virtual std::string getExtension();
  virtual bool save(mimir::Block& data);

  static KRScene* Load(KRContext& context, const std::string& name, mimir::Block* data);

  KRNode* getRootNode();
  KRLight* getFirstLight();

  kraken_stream_level getStreamLevel();

  bool lineCast(const hydra::Vector3& v0, const hydra::Vector3& v1, hydra::HitInfo& hitinfo, unsigned int layer_mask);
  bool rayCast(const hydra::Vector3& v0, const hydra::Vector3& dir, hydra::HitInfo& hitinfo, unsigned int layer_mask);
  bool sphereCast(const hydra::Vector3& v0, const hydra::Vector3& v1, float radius, hydra::HitInfo& hitinfo, unsigned int layer_mask);

  void renderFrame(VkCommandBuffer& commandBuffer, KRSurface& surface, KRRenderGraph& renderGraph, float deltaTime);
  void render(KRNode::RenderInfo& ri);

  void updateOctree(const KRViewport& viewport);
  void buildOctreeForTheFirstTime();

  void notify_sceneGraphCreate(KRNode* pNode);
  void notify_sceneGraphDelete(KRNode* pNode);
  void notify_sceneGraphModify(KRNode* pNode);

  void physicsUpdate(float deltaTime);
  void addDefaultLights();

  hydra::AABB getRootOctreeBounds();

  std::set<KRAmbientZone*>& getAmbientZones();
  std::set<KRReverbZone*>& getReverbZones();
  std::set<KRLocator*>& getLocators();
  std::set<KRLight*>& getLights();

private:
  void render(KRNode::RenderInfo& ri, std::list<KRResourceRequest>& resourceRequests, KROctreeNode* pOctreeNode, std::vector<KROctreeNode*>& remainingOctrees, std::vector<KROctreeNode*>& remainingOctreesTestResults, std::vector<KROctreeNode*>& remainingOctreesTestResultsOnly);
  void render_occlusionResultsPass(KRNode::RenderInfo& ri, KROctreeNode* pOctreeNode, std::vector<KROctreeNode*>& remainingOctrees, bool bOcclusionTestResultsOnly);


  KRNode* m_pRootNode;
  KRLight* m_pFirstLight;

  std::set<KRNode*> m_newNodes;
  std::set<KRNode*> m_modifiedNodes;



  std::set<KRNode*> m_physicsNodes;
  std::set<KRAmbientZone*> m_ambientZoneNodes;
  std::set<KRReverbZone*> m_reverbZoneNodes;
  std::set<KRLocator*> m_locatorNodes;
  std::set<KRLight*> m_lights;

  KROctree m_nodeTree;

public:

  template <class T> T* find()
  {
    if (m_pRootNode) return m_pRootNode->find<T>();
    return NULL;
  }

  template <class T> T* find(const std::string& name)
  {
    if (m_pRootNode) return m_pRootNode->find<T>(name);
    return NULL;
  }
};
