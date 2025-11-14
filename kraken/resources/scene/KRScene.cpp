//
//  KRScene.cpp
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

#include "nodes/KRLight.h"

#include "KRScene.h"
#include "nodes/KRNode.h"
#include "nodes/KRDirectionalLight.h"
#include "nodes/KRSpotLight.h"
#include "nodes/KRPointLight.h"
#include "resources/audio/KRAudioManager.h"
#include "KRRenderPass.h"

using namespace mimir;
using namespace hydra;

KRScene::KRScene(KRContext& context, std::string name) : KRResource(context, name)
{
  m_pFirstLight = NULL;
  m_pRootNode = new KRNode(*this, "scene_root");
  notify_sceneGraphCreate(m_pRootNode);
}

KRScene::~KRScene()
{
  delete m_pRootNode;
  m_pRootNode = NULL;
}

void KRScene::renderFrame(VkCommandBuffer& commandBuffer, KRSurface& surface, KRRenderGraph& renderGraph, float deltaTime)
{
  getContext().startFrame(deltaTime);
  KRCamera* camera = find<KRCamera>("default_camera");
  if (camera == NULL) {
    // Add a default camera if none are present
    camera = new KRCamera(*this, "default_camera");
    m_pRootNode->appendChild(camera);
  }

  // FINDME - This should be moved to de-couple Siren from the Rendering pipeline
  getContext().getAudioManager()->setEnableAudio(camera->settings.siren_enable);
  getContext().getAudioManager()->setEnableHRTF(camera->settings.siren_enable_hrtf);
  getContext().getAudioManager()->setEnableReverb(camera->settings.siren_enable_reverb);
  getContext().getAudioManager()->setReverbMaxLength(camera->settings.siren_reverb_max_length);
  getContext().getTextureManager()->setMaxAnisotropy(camera->settings.max_anisotropy);

  camera->renderFrame(commandBuffer, surface, renderGraph);
  getContext().endFrame(deltaTime);
  physicsUpdate(deltaTime);
}

std::set<KRAmbientZone*>& KRScene::getAmbientZones()
{
  // FINDME, TODO - To support large scenes with many reverb / ambient zones, this function should take a KRAABB and cull out any far away zones
  return m_ambientZoneNodes;
}

std::set<KRReverbZone*>& KRScene::getReverbZones()
{
  // FINDME, TODO - To support large scenes with many reverb / ambient zones, this function should take a KRAABB and cull out any far away zones
  return m_reverbZoneNodes;
}

std::set<KRLocator*>& KRScene::getLocators()
{
  return m_locatorNodes;
}

std::set<KRLight*>& KRScene::getLights()
{
  return m_lights;
}

void KRScene::render(KRNode::RenderInfo& ri)
{

  // ----------  Start: Vulkan Debug Code ----------
  /*
  if (ri.renderPass->getType() == RenderPassType::RENDER_PASS_FORWARD_OPAQUE) {
    KRMesh* sphereMesh = getContext().getMeshManager()->getMesh("__sphere");
    if (sphereMesh && sphereMesh->isReady()) {
      PipelineInfo info{};
      std::string shader_name("vulkan_test");
      info.shader_name = &shader_name;
      info.pCamera = ri.camera;
      info.renderPass = ri.renderPass;
      info.rasterMode = RasterMode::kOpaque;
      info.vertexAttributes = sphereMesh->getVertexAttributes();
      info.modelFormat = sphereMesh->getModelFormat();
      KRPipeline* testPipeline = m_pContext->getPipelineManager()->getPipeline(*ri.surface, info);
      testPipeline->bind(ri, Matrix4());
      sphereMesh->renderNoMaterials(ri.commandBuffer, info.renderPass, "Vulkan Test", "vulkan_test", 1.0);
    }
  }
  */
  // ----------  End: Vulkan Debug Code ----------

  if (getFirstLight() == NULL) {
    addDefaultLights();
  }

  std::set<KRNode*> outerNodes = std::set<KRNode*>(m_nodeTree.getOuterSceneNodes()); // HACK - Copying the std::set as it is potentially modified as KRNode's update their bounds during the iteration.  This is very expensive and will be eliminated in the future.

  // Get lights from outer nodes (directional lights, which have no bounds)
  for (std::set<KRNode*>::iterator itr = outerNodes.begin(); itr != outerNodes.end(); itr++) {
    KRNode* node = (*itr);
    KRPointLight* point_light = dynamic_cast<KRPointLight*>(node);
    if (point_light) {
      ri.point_lights.push_back(point_light);
    }
    KRDirectionalLight* directional_light = dynamic_cast<KRDirectionalLight*>(node);
    if (directional_light) {
      ri.directional_lights.push_back(directional_light);
    }
    KRSpotLight* spot_light = dynamic_cast<KRSpotLight*>(node);
    if (spot_light) {
      ri.spot_lights.push_back(spot_light);
    }
  }

  // Render outer nodes
  for (std::set<KRNode*>::iterator itr = outerNodes.begin(); itr != outerNodes.end(); itr++) {
    KRNode* node = (*itr);
    if (ri.renderPass->getType() == RenderPassType::RENDER_PASS_PRESTREAM) {
      if ((*itr)->getLODVisibility() >= KRNode::LOD_VISIBILITY_PRESTREAM) {
        node->preStream(*ri.viewport);
      }
    } else {
      if ((*itr)->getLODVisibility() > KRNode::LOD_VISIBILITY_PRESTREAM) {
        node->render(ri);
      }
    }
  }

  std::vector<KROctreeNode*> remainingOctrees;
  std::vector<KROctreeNode*> remainingOctreesTestResults;
  std::vector<KROctreeNode*> remainingOctreesTestResultsOnly;
  if (m_nodeTree.getRootNode() != NULL) {
    remainingOctrees.push_back(m_nodeTree.getRootNode());
  }

  std::vector<KROctreeNode*> newRemainingOctrees;
  std::vector<KROctreeNode*> newRemainingOctreesTestResults;
  while ((!remainingOctrees.empty() || !remainingOctreesTestResults.empty())) {
    newRemainingOctrees.clear();
    newRemainingOctreesTestResults.clear();
    for (std::vector<KROctreeNode*>::iterator octree_itr = remainingOctrees.begin(); octree_itr != remainingOctrees.end(); octree_itr++) {
      render(ri, *octree_itr, newRemainingOctrees, newRemainingOctreesTestResults, remainingOctreesTestResultsOnly, false, false);
    }
    for (std::vector<KROctreeNode*>::iterator octree_itr = remainingOctreesTestResults.begin(); octree_itr != remainingOctreesTestResults.end(); octree_itr++) {
      render(ri, *octree_itr, newRemainingOctrees, newRemainingOctreesTestResults, remainingOctreesTestResultsOnly, true, false);
    }
    remainingOctrees = newRemainingOctrees;
    remainingOctreesTestResults = newRemainingOctreesTestResults;
  }

  newRemainingOctrees.clear();
  newRemainingOctreesTestResults.clear();
  for (std::vector<KROctreeNode*>::iterator octree_itr = remainingOctreesTestResultsOnly.begin(); octree_itr != remainingOctreesTestResultsOnly.end(); octree_itr++) {
    render(ri, *octree_itr, newRemainingOctrees, newRemainingOctreesTestResults, remainingOctreesTestResultsOnly, true, true);
  }
}

void KRScene::render(KRNode::RenderInfo& ri, KROctreeNode* pOctreeNode, std::vector<KROctreeNode*>& remainingOctrees, std::vector<KROctreeNode*>& remainingOctreesTestResults, std::vector<KROctreeNode*>& remainingOctreesTestResultsOnly, bool bOcclusionResultsPass, bool bOcclusionTestResultsOnly)
{
  unordered_map<AABB, int>& visibleBounds = ri.viewport->getVisibleBounds();
  if (pOctreeNode) {

    AABB octreeBounds = pOctreeNode->getBounds();

    if (bOcclusionResultsPass) {
      // ----====---- Occlusion results pass ----====----
      if (pOctreeNode->m_occlusionTested) {
        int params = 0;
        GLDEBUG(glGetQueryObjectuivEXT(pOctreeNode->m_occlusionQuery, GL_QUERY_RESULT_EXT, &params));

        if (params) {
          // Record the frame number that the test has passed on
          visibleBounds[octreeBounds] = getContext().getCurrentFrame();

          if (!bOcclusionTestResultsOnly) {
            // Schedule a pass to perform the rendering
            remainingOctrees.push_back(pOctreeNode);
          }
        } else {
          // Record -1 to indicate that the visibility test had failed
          visibleBounds[octreeBounds] = -1;
        }

        GLDEBUG(glDeleteQueriesEXT(1, &pOctreeNode->m_occlusionQuery));
        pOctreeNode->m_occlusionTested = false;
        pOctreeNode->m_occlusionQuery = 0;
      }
    } else {
      bool in_viewport = false;
      if (ri.renderPass->getType() == RenderPassType::RENDER_PASS_PRESTREAM) {
        // When pre-streaming, objects are streamed in behind and in-front of the camera
        AABB viewportExtents = AABB::Create(ri.viewport->getCameraPosition() - Vector3::Create(ri.camera->settings.getPerspectiveFarZ()), ri.viewport->getCameraPosition() + Vector3::Create(ri.camera->settings.getPerspectiveFarZ()));
        in_viewport = octreeBounds.intersects(viewportExtents);
      } else {
        in_viewport = ri.viewport->visible(pOctreeNode->getBounds());
      }
      if (in_viewport) {

        // ----====---- Rendering and occlusion test pass ----====----
        bool bVisible = false;
        bool bNeedOcclusionTest = true;

        if (!ri.camera->settings.getEnableRealtimeOcclusion()) {
          bVisible = true;
          bNeedOcclusionTest = false;
        }

        if (!bVisible) {
          // Assume bounding boxes are visible without occlusion test queries if the camera is inside the box.
          // The near clipping plane of the camera is taken into consideration by expanding the match area
          AABB cameraExtents = AABB::Create(ri.viewport->getCameraPosition() - Vector3::Create(ri.camera->settings.getPerspectiveNearZ()), ri.viewport->getCameraPosition() + Vector3::Create(ri.camera->settings.getPerspectiveNearZ()));
          bVisible = octreeBounds.intersects(cameraExtents);
          if (bVisible) {
            // Record the frame number in which the camera was within the bounds
            visibleBounds[octreeBounds] = getContext().getCurrentFrame();
            bNeedOcclusionTest = false;
          }
        }


        if (!bVisible) {
          // Check if a previous occlusion query has returned true, taking advantage of temporal consistency of visible elements from frame to frame
          // If the previous frame rendered this octree, then attempt to render it in this frame without performing a pre-occlusion test
          unordered_map<AABB, int>::iterator match_itr = visibleBounds.find(octreeBounds);
          if (match_itr != visibleBounds.end()) {
            if ((*match_itr).second == -1) {
              // We have already tested these bounds with a negative result
              bNeedOcclusionTest = false;
            } else {
              bVisible = true;

              // We set bNeedOcclusionTest to false only when the previous occlusion test is old and we need to perform an occlusion test to record if this octree node was visible for the next frame
              bNeedOcclusionTest = false;
            }
          }

        }

        if (!bVisible && bNeedOcclusionTest) {
          // Optimization: If this is an empty octree node with only a single child node, then immediately try to render the child node without an occlusion test for this higher level, as it would be more expensive than the occlusion test for the child
          if (pOctreeNode->getSceneNodes().empty()) {
            int child_count = 0;
            for (int i = 0; i < 8; i++) {
              if (pOctreeNode->getChildren()[i] != NULL) child_count++;
            }
            if (child_count == 1) {
              bVisible = true;
              bNeedOcclusionTest = false;
            }
          }
        }

        if (bNeedOcclusionTest) {
          pOctreeNode->beginOcclusionQuery();

          Matrix4 matModel = Matrix4();
          matModel.scale(octreeBounds.size() * 0.5f);
          matModel.translate(octreeBounds.center());
          Matrix4 mvpmatrix = matModel * ri.viewport->getViewProjectionMatrix();

          KRMeshManager::KRVBOData& vertices = getContext().getMeshManager()->KRENGINE_VBO_DATA_3D_CUBE_VERTICES;

          getContext().getMeshManager()->bindVBO(ri.commandBuffer, &vertices, 1.0f);

          PipelineInfo info{};
          std::string shader_name("occlusion_test");
          info.shader_name = &shader_name;
          info.pCamera = ri.camera;
          info.point_lights = &ri.point_lights;
          info.directional_lights = &ri.directional_lights;
          info.spot_lights = &ri.spot_lights;
          info.renderPass = ri.renderPass;
          info.rasterMode = RasterMode::kAdditive;
          info.vertexAttributes = vertices.getVertexAttributes();
          info.modelFormat = ModelFormat::KRENGINE_MODEL_FORMAT_STRIP;

          KRPipeline* pPipeline = getContext().getPipelineManager()->getPipeline(*ri.surface, info);
          pPipeline->bind(ri, matModel);
          vkCmdDraw(ri.commandBuffer, 14, 1, 0, 0);
          m_pContext->getMeshManager()->log_draw_call(ri.renderPass->getType(), "octree", "occlusion_test", 14);

          pOctreeNode->endOcclusionQuery();

          if (bVisible) {
            // Schedule a pass to get the result of the occlusion test only for future frames and passes, without rendering the model or recurring further
            remainingOctreesTestResultsOnly.push_back(pOctreeNode);
          } else {
            // Schedule a pass to get the result of the occlusion test and continue recursion and rendering if test is true
            remainingOctreesTestResults.push_back(pOctreeNode);
          }
        }

        if (bVisible) {

          // Add lights that influence this octree level and its children to the stack
          int directional_light_count = 0;
          int spot_light_count = 0;
          int point_light_count = 0;
          for (std::set<KRNode*>::iterator itr = pOctreeNode->getSceneNodes().begin(); itr != pOctreeNode->getSceneNodes().end(); itr++) {
            KRNode* node = (*itr);
            KRDirectionalLight* directional_light = dynamic_cast<KRDirectionalLight*>(node);
            if (directional_light) {
              ri.directional_lights.push_back(directional_light);
              directional_light_count++;
            }
            KRSpotLight* spot_light = dynamic_cast<KRSpotLight*>(node);
            if (spot_light) {
              ri.spot_lights.push_back(spot_light);
              spot_light_count++;
            }
            KRPointLight* point_light = dynamic_cast<KRPointLight*>(node);
            if (point_light) {
              ri.point_lights.push_back(point_light);
              point_light_count++;
            }
          }

          // Render objects that are at this octree level
          for (std::set<KRNode*>::iterator itr = pOctreeNode->getSceneNodes().begin(); itr != pOctreeNode->getSceneNodes().end(); itr++) {
            //assert(pOctreeNode->getBounds().contains((*itr)->getBounds()));  // Sanity check
            if (ri.renderPass->getType() == RenderPassType::RENDER_PASS_PRESTREAM) {
              if ((*itr)->getLODVisibility() >= KRNode::LOD_VISIBILITY_PRESTREAM) {
                (*itr)->preStream(*ri.viewport);
              }
            } else {
              if ((*itr)->getLODVisibility() > KRNode::LOD_VISIBILITY_PRESTREAM)
              {
                (*itr)->render(ri);
              }
            }
          }

          // Render child octrees
          const int* childOctreeOrder = ri.renderPass->getType() == RenderPassType::RENDER_PASS_FORWARD_TRANSPARENT || ri.renderPass->getType() == RenderPassType::RENDER_PASS_ADDITIVE_PARTICLES || ri.renderPass->getType() == RenderPassType::RENDER_PASS_VOLUMETRIC_EFFECTS_ADDITIVE ? ri.viewport->getBackToFrontOrder() : ri.viewport->getFrontToBackOrder();

          for (int i = 0; i < 8; i++) {
            render(ri, pOctreeNode->getChildren()[childOctreeOrder[i]], remainingOctrees, remainingOctreesTestResults, remainingOctreesTestResultsOnly, false, false);
          }

          // Remove lights added at this octree level from the stack
          while (directional_light_count--) {
            ri.directional_lights.pop_back();
          }
          while (spot_light_count--) {
            ri.spot_lights.pop_back();
          }
          while (point_light_count--) {
            ri.point_lights.pop_back();
          }
        }
      }

    }
  }
  //  fprintf(stderr, "Octree culled: (%f, %f, %f) - (%f, %f, %f)\n", pOctreeNode->getBounds().min.x, pOctreeNode->getBounds().min.y, pOctreeNode->getBounds().min.z, pOctreeNode->getBounds().max.x, pOctreeNode->getBounds().max.y, pOctreeNode->getBounds().max.z);
}

std::string KRScene::getExtension()
{
  return "krscene";
}

KRNode* KRScene::getRootNode()
{
  return m_pRootNode;
}

bool KRScene::save(Block& data)
{
  tinyxml2::XMLDocument doc;
  tinyxml2::XMLElement* scene_node = doc.NewElement("scene");
  doc.InsertEndChild(scene_node);
  m_pRootNode->saveXML(scene_node);

  tinyxml2::XMLPrinter p;
  doc.Print(&p);
  data.append((void*)p.CStr(), strlen(p.CStr()) + 1);

  return true;
}

KRScene* KRScene::Load(KRContext& context, const std::string& name, Block* data)
{
  std::string xml_string = data->getString();
  delete data;
  tinyxml2::XMLDocument doc;
  doc.Parse(xml_string.c_str());
  KRScene* new_scene = new KRScene(context, name);

  tinyxml2::XMLElement* scene_element = doc.RootElement();

  KRNode* n = KRNode::LoadXML(*new_scene, scene_element->FirstChildElement());
  if (n) {
    new_scene->getRootNode()->appendChild(n);
  }


  return new_scene;
}



KRLight* KRScene::getFirstLight()
{
  if (m_pFirstLight == NULL) {
    m_pFirstLight = find<KRLight>();
  }
  return m_pFirstLight;
}

void KRScene::notify_sceneGraphCreate(KRNode* pNode)
{
  //    m_nodeTree.add(pNode);
  //    if(pNode->hasPhysics()) {
  //        m_physicsNodes.insert(pNode);
  //    }
  m_newNodes.insert(pNode);
}

void KRScene::notify_sceneGraphModify(KRNode* pNode)
{
  //    m_nodeTree.update(pNode);
  m_modifiedNodes.insert(pNode);
}

void KRScene::notify_sceneGraphDelete(KRNode* pNode)
{
  m_nodeTree.remove(pNode);
  m_physicsNodes.erase(pNode);
  KRAmbientZone* AmbientZoneNode = dynamic_cast<KRAmbientZone*>(pNode);
  if (AmbientZoneNode) {
    m_ambientZoneNodes.erase(AmbientZoneNode);
  }
  KRReverbZone* ReverbZoneNode = dynamic_cast<KRReverbZone*>(pNode);
  if (ReverbZoneNode) {
    m_reverbZoneNodes.erase(ReverbZoneNode);
  }
  KRLocator* locator = dynamic_cast<KRLocator*>(pNode);
  if (locator) {
    m_locatorNodes.erase(locator);
  }
  KRLight* light = dynamic_cast<KRLight*>(pNode);
  if (light) {
    m_lights.erase(light);
  }
  m_modifiedNodes.erase(pNode);
  if (!m_newNodes.erase(pNode)) {
    m_nodeTree.remove(pNode);
  }
}

void KRScene::updateOctree(const KRViewport& viewport)
{
  m_pRootNode->setLODVisibility(KRNode::LOD_VISIBILITY_VISIBLE);
  m_pRootNode->updateLODVisibility(viewport);

  std::set<KRNode*> newNodes = std::move(m_newNodes);
  std::set<KRNode*> modifiedNodes = std::move(m_modifiedNodes);
  m_newNodes.clear();
  m_modifiedNodes.clear();

  for (std::set<KRNode*>::iterator itr = newNodes.begin(); itr != newNodes.end(); itr++) {
    KRNode* node = *itr;
    m_nodeTree.add(node);
    if (node->hasPhysics()) {
      m_physicsNodes.insert(node);
    }
    KRAmbientZone* ambientZoneNode = dynamic_cast<KRAmbientZone*>(node);
    if (ambientZoneNode) {
      m_ambientZoneNodes.insert(ambientZoneNode);
    }
    KRReverbZone* reverbZoneNode = dynamic_cast<KRReverbZone*>(node);
    if (reverbZoneNode) {
      m_reverbZoneNodes.insert(reverbZoneNode);
    }
    KRLocator* locatorNode = dynamic_cast<KRLocator*>(node);
    if (locatorNode) {
      m_locatorNodes.insert(locatorNode);
    }
    KRLight* light = dynamic_cast<KRLight*>(node);
    if (light) {
      m_lights.insert(light);
    }
  }
  for (std::set<KRNode*>::iterator itr = modifiedNodes.begin(); itr != modifiedNodes.end(); itr++) {
    KRNode* node = *itr;
    if (node->getLODVisibility() >= KRNode::LOD_VISIBILITY_PRESTREAM) {
      m_nodeTree.update(node);
    }
    if (node->hasPhysics()) {
      m_physicsNodes.insert(node);
    } else if (!node->hasPhysics()) {
      m_physicsNodes.erase(node);
    }
  }
}

void KRScene::buildOctreeForTheFirstTime()
{
  std::set<KRNode*> newNodes = std::move(m_newNodes);
  m_newNodes.clear();
  for (std::set<KRNode*>::iterator itr = newNodes.begin(); itr != newNodes.end(); itr++) {
    KRNode* node = *itr;
    m_nodeTree.add(node);
    if (node->hasPhysics()) {
      m_physicsNodes.insert(node);
    }
    KRAmbientZone* ambientZoneNode = dynamic_cast<KRAmbientZone*>(node);
    if (ambientZoneNode) {
      m_ambientZoneNodes.insert(ambientZoneNode);
    }
    KRReverbZone* reverbZoneNode = dynamic_cast<KRReverbZone*>(node);
    if (reverbZoneNode) {
      m_reverbZoneNodes.insert(reverbZoneNode);
    }
    KRLocator* locatorNode = dynamic_cast<KRLocator*>(node);
    if (locatorNode) {
      m_locatorNodes.insert(locatorNode);
    }
    KRLight* light = dynamic_cast<KRLight*>(node);
    if (light) {
      m_lights.insert(light);
    }
  }
}

void KRScene::physicsUpdate(float deltaTime)
{
  for (std::set<KRNode*>::iterator itr = m_physicsNodes.begin(); itr != m_physicsNodes.end(); itr++) {
    (*itr)->physicsUpdate(deltaTime);
  }
}

void KRScene::addDefaultLights()
{
  KRDirectionalLight* light1 = new KRDirectionalLight(*this, "default_light1");

  light1->setLocalRotation((Quaternion::Create(Vector3::Create(0.0f, (float)M_PI * 0.10f, 0.0f)) * Quaternion::Create(Vector3::Create(0.0f, 0.0f, (float)-M_PI * 0.15f))).eulerXYZ());
  m_pRootNode->appendChild(light1);
}

AABB KRScene::getRootOctreeBounds()
{
  if (m_nodeTree.getRootNode()) {
    return m_nodeTree.getRootNode()->getBounds();
  } else {
    return AABB::Create(-Vector3::One(), Vector3::One());
  }
}


bool KRScene::lineCast(const Vector3& v0, const Vector3& v1, HitInfo& hitinfo, unsigned int layer_mask)
{
  return m_nodeTree.lineCast(v0, v1, hitinfo, layer_mask);
}

bool KRScene::rayCast(const Vector3& v0, const Vector3& dir, HitInfo& hitinfo, unsigned int layer_mask)
{
  return m_nodeTree.rayCast(v0, dir, hitinfo, layer_mask);
}

bool KRScene::sphereCast(const Vector3& v0, const Vector3& v1, float radius, HitInfo& hitinfo, unsigned int layer_mask)
{
  return m_nodeTree.sphereCast(v0, v1, radius, hitinfo, layer_mask);
}


kraken_stream_level KRScene::getStreamLevel()
{
  kraken_stream_level stream_level = kraken_stream_level::STREAM_LEVEL_IN_HQ;

  if (m_pRootNode) {
    KRViewport viewport; // This isn't used when prime is false
    stream_level = KRMIN(stream_level, m_pRootNode->getStreamLevel(viewport));
  }

  return stream_level;
}
