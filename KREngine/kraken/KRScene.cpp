//
//  KRScene.cpp
//  KREngine
//
//  Copyright 2012 Kearwood Gilbert. All rights reserved.
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
#include "KRVector3.h"
#include "KRMat4.h"
#include "tinyxml2.h"

#include "KRLight.h"

#include "KRScene.h"
#include "KRNode.h"
#include "KRLODGroup.h"
#include "KRStockGeometry.h"
#include "KRDirectionalLight.h"
#include "KRSpotLight.h"
#include "KRPointLight.h"
#include "KRQuaternion.h"

const long KRENGINE_OCCLUSION_TEST_EXPIRY = 10;

KRScene::KRScene(KRContext &context, std::string name) : KRResource(context, name) {
    m_pFirstLight = NULL;
    m_pRootNode = new KRLODGroup(*this, "scene_root");
    notify_sceneGraphCreate(m_pRootNode);
    
    m_skyBoxName = name + "_skybox";
}

KRScene::~KRScene() {
    delete m_pRootNode;
    m_pRootNode = NULL;
}

void KRScene::renderFrame(float deltaTime, int width, int height) {
    getContext().startFrame(deltaTime);
    KRCamera *camera = find<KRCamera>("default_camera");
    if(camera == NULL) {
        // Add a default camera if none are present
        camera = new KRCamera(*this, "default_camera");
        m_pRootNode->addChild(camera);
    }
    
    // FINDME - This should be moved to de-couple Siren from the Rendering pipeline
    getContext().getAudioManager()->setEnableAudio(camera->settings.siren_enable);
    getContext().getAudioManager()->setEnableHRTF(camera->settings.siren_enable_hrtf);
    getContext().getAudioManager()->setEnableReverb(camera->settings.siren_enable_reverb);
    getContext().getAudioManager()->setReverbMaxLength(camera->settings.siren_reverb_max_length);
    getContext().getTextureManager()->setMaxAnisotropy(camera->settings.max_anisotropy);
    
    camera->renderFrame(deltaTime, width, height);
    getContext().endFrame(deltaTime);
    physicsUpdate(deltaTime);
}

std::set<KRAmbientZone *> &KRScene::getAmbientZones()
{
    // FINDME, TODO - To support large scenes with many reverb / ambient zones, this function should take a KRAABB and cull out any far away zones
    return m_ambientZoneNodes;
}

std::set<KRReverbZone *> &KRScene::getReverbZones()
{
    // FINDME, TODO - To support large scenes with many reverb / ambient zones, this function should take a KRAABB and cull out any far away zones
    return m_reverbZoneNodes;
}

std::set<KRLocator *> &KRScene::getLocators()
{
    return m_locatorNodes;
}

void KRScene::render(KRCamera *pCamera, unordered_map<KRAABB, int> &visibleBounds, const KRViewport &viewport, KRNode::RenderPass renderPass, bool new_frame) {
    if(new_frame) {
        // Expire cached occlusion test results.
        // Cached "failed" results are expired on the next frame (marked with .second of -1)
        // Cached "success" results are expired after KRENGINE_OCCLUSION_TEST_EXPIRY frames (marked with .second of the last frame
        std::set<KRAABB> expired_visible_bounds;
        for(unordered_map<KRAABB, int>::iterator visible_bounds_itr = visibleBounds.begin(); visible_bounds_itr != visibleBounds.end(); visible_bounds_itr++) {
            if((*visible_bounds_itr).second == -1 || (*visible_bounds_itr).second + KRENGINE_OCCLUSION_TEST_EXPIRY < getContext().getCurrentFrame()) {
                expired_visible_bounds.insert((*visible_bounds_itr).first);
            }
        }
        for(std::set<KRAABB>::iterator expired_visible_bounds_itr = expired_visible_bounds.begin(); expired_visible_bounds_itr != expired_visible_bounds.end(); expired_visible_bounds_itr++) {
            visibleBounds.erase(*expired_visible_bounds_itr);
        }
    }
    
    if(getFirstLight() == NULL) {
        addDefaultLights();
    }
    
    std::vector<KRPointLight *> point_lights;
    std::vector<KRDirectionalLight *>directional_lights;
    std::vector<KRSpotLight *>spot_lights;
    
    pCamera->settings.setSkyBox(m_skyBoxName); // This is temporary until the camera is moved into the scene graph
    
    
    std::set<KRNode *> outerNodes = std::set<KRNode *>(m_nodeTree.getOuterSceneNodes()); // HACK - Copying the std::set as it is potentially modified as KRNode's update their bounds during the iteration.  This is very expensive and will be eliminated in the future.
    
    // Get lights from outer nodes (directional lights, which have no bounds)
    for(std::set<KRNode *>::iterator itr=outerNodes.begin(); itr != outerNodes.end(); itr++) {
        KRNode *node = (*itr);
        KRPointLight *point_light = dynamic_cast<KRPointLight *>(node);
        if(point_light) {
            point_lights.push_back(point_light);
        }
        KRDirectionalLight *directional_light = dynamic_cast<KRDirectionalLight *>(node);
        if(directional_light) {
            directional_lights.push_back(directional_light);
        }
        KRSpotLight *spot_light = dynamic_cast<KRSpotLight *>(node);
        if(spot_light) {
            spot_lights.push_back(spot_light);
        }
    }
    
    // Render outer nodes
    for(std::set<KRNode *>::iterator itr=outerNodes.begin(); itr != outerNodes.end(); itr++) {
        KRNode *node = (*itr);
        node->render(pCamera, point_lights, directional_lights, spot_lights, viewport, renderPass);
    }
    
    std::vector<KROctreeNode *> remainingOctrees;
    std::vector<KROctreeNode *> remainingOctreesTestResults;
    std::vector<KROctreeNode *> remainingOctreesTestResultsOnly;
    if(m_nodeTree.getRootNode() != NULL) {
        remainingOctrees.push_back(m_nodeTree.getRootNode());
    }
    
    std::vector<KROctreeNode *> newRemainingOctrees;
    std::vector<KROctreeNode *> newRemainingOctreesTestResults;
    while((!remainingOctrees.empty() || !remainingOctreesTestResults.empty())) {
        newRemainingOctrees.clear();
        newRemainingOctreesTestResults.clear();
        for(std::vector<KROctreeNode *>::iterator octree_itr = remainingOctrees.begin(); octree_itr != remainingOctrees.end(); octree_itr++) {
            render(*octree_itr, visibleBounds, pCamera, point_lights, directional_lights, spot_lights, viewport, renderPass, newRemainingOctrees, newRemainingOctreesTestResults, remainingOctreesTestResultsOnly, false, false);
        }
        for(std::vector<KROctreeNode *>::iterator octree_itr = remainingOctreesTestResults.begin(); octree_itr != remainingOctreesTestResults.end(); octree_itr++) {
            render(*octree_itr, visibleBounds, pCamera, point_lights, directional_lights, spot_lights, viewport, renderPass, newRemainingOctrees, newRemainingOctreesTestResults, remainingOctreesTestResultsOnly, true, false);
        }
        remainingOctrees = newRemainingOctrees;
        remainingOctreesTestResults = newRemainingOctreesTestResults;
    }
    
    newRemainingOctrees.clear();
    newRemainingOctreesTestResults.clear();
    for(std::vector<KROctreeNode *>::iterator octree_itr = remainingOctreesTestResultsOnly.begin(); octree_itr != remainingOctreesTestResultsOnly.end(); octree_itr++) {
        render(*octree_itr, visibleBounds, pCamera, point_lights, directional_lights, spot_lights, viewport, renderPass, newRemainingOctrees, newRemainingOctreesTestResults, remainingOctreesTestResultsOnly, true, true);
    }
}

void KRScene::render(KROctreeNode *pOctreeNode, unordered_map<KRAABB, int> &visibleBounds, KRCamera *pCamera, std::vector<KRPointLight *> &point_lights, std::vector<KRDirectionalLight *> &directional_lights, std::vector<KRSpotLight *>&spot_lights, const KRViewport &viewport, KRNode::RenderPass renderPass, std::vector<KROctreeNode *> &remainingOctrees, std::vector<KROctreeNode *> &remainingOctreesTestResults, std::vector<KROctreeNode *> &remainingOctreesTestResultsOnly, bool bOcclusionResultsPass, bool bOcclusionTestResultsOnly)
{    
    if(pOctreeNode) {
        
        KRAABB octreeBounds = pOctreeNode->getBounds();
        
        if(bOcclusionResultsPass) {
            // ----====---- Occlusion results pass ----====----
            if(pOctreeNode->m_occlusionTested) {
                GLuint params = 0;
                GLDEBUG(glGetQueryObjectuivEXT(pOctreeNode->m_occlusionQuery, GL_QUERY_RESULT_EXT, &params));

                if(params) {
                    // Record the frame number that the test has passed on
                    visibleBounds[octreeBounds] = getContext().getCurrentFrame();

                    if(!bOcclusionTestResultsOnly) {
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
            
            if(viewport.visible(pOctreeNode->getBounds())) {

                // ----====---- Rendering and occlusion test pass ----====----
                bool bVisible = false;
                bool bNeedOcclusionTest = true;
                
                if(!pCamera->settings.getEnableRealtimeOcclusion()) {
                    bVisible = true;
                    bNeedOcclusionTest = false;
                }
          
                if(!bVisible) {
                    // Assume bounding boxes are visible without occlusion test queries if the camera is inside the box.
                    // The near clipping plane of the camera is taken into consideration by expanding the match area
                    KRAABB cameraExtents = KRAABB(viewport.getCameraPosition() - KRVector3(pCamera->settings.getPerspectiveNearZ()), viewport.getCameraPosition() + KRVector3(pCamera->settings.getPerspectiveNearZ()));
                    bVisible = octreeBounds.intersects(cameraExtents);
                    if(bVisible) {
                        // Record the frame number in which the camera was within the bounds
                        visibleBounds[octreeBounds] = getContext().getCurrentFrame();
                        bNeedOcclusionTest = false;
                    }
                }
                
                
                if(!bVisible) {
                    // Check if a previous occlusion query has returned true, taking advantage of temporal consistency of visible elements from frame to frame
                    // If the previous frame rendered this octree, then attempt to render it in this frame without performing a pre-occlusion test
                    unordered_map<KRAABB, int>::iterator match_itr = visibleBounds.find(octreeBounds);
                    if(match_itr != visibleBounds.end()) {
                        if((*match_itr).second == -1) {
                            // We have already tested these bounds with a negative result
                            bNeedOcclusionTest = false;
                        } else {
                            bVisible = true;
                            
                            // We set bNeedOcclusionTest to false only when the previous occlusion test is old and we need to perform an occlusion test to record if this octree node was visible for the next frame
                            bNeedOcclusionTest = false;
                        }
                    }
                    
                }
                
                if(!bVisible && bNeedOcclusionTest) {
                    // Optimization: If this is an empty octree node with only a single child node, then immediately try to render the child node without an occlusion test for this higher level, as it would be more expensive than the occlusion test for the child
                    if(pOctreeNode->getSceneNodes().empty()) {
                        int child_count = 0;
                        for(int i=0; i<8; i++) {
                            if(pOctreeNode->getChildren()[i] != NULL) child_count++;
                        }
                        if(child_count == 1) {
                            bVisible = true;
                            bNeedOcclusionTest = false;
                        }
                    }
                }
                
                if(bNeedOcclusionTest) {
                    pOctreeNode->beginOcclusionQuery();
                    


                    KRMat4 matModel = KRMat4();
                    matModel.scale(octreeBounds.size() * 0.5f);
                    matModel.translate(octreeBounds.center());
                    KRMat4 mvpmatrix = matModel * viewport.getViewProjectionMatrix();
                    

                    getContext().getModelManager()->bindVBO(getContext().getModelManager()->KRENGINE_VBO_3D_CUBE_VERTICES, getContext().getModelManager()->KRENGINE_VBO_3D_CUBE_INDEXES, getContext().getModelManager()->KRENGINE_VBO_3D_CUBE_ATTRIBS, true);
                    
                    // Enable additive blending
                    if(renderPass != KRNode::RENDER_PASS_FORWARD_TRANSPARENT && renderPass != KRNode::RENDER_PASS_ADDITIVE_PARTICLES && renderPass != KRNode::RENDER_PASS_VOLUMETRIC_EFFECTS_ADDITIVE) {
                        GLDEBUG(glEnable(GL_BLEND));
                    }
                    GLDEBUG(glBlendFunc(GL_ONE, GL_ONE));
                    
                    
                    if(renderPass == KRNode::RENDER_PASS_FORWARD_OPAQUE ||
                       renderPass == KRNode::RENDER_PASS_DEFERRED_GBUFFER ||
                       renderPass == KRNode::RENDER_PASS_DEFERRED_OPAQUE ||
                       renderPass == KRNode::RENDER_PASS_SHADOWMAP) {
                        
                        // Disable z-buffer write
                        GLDEBUG(glDepthMask(GL_FALSE));
                    }
                    
                    if(getContext().getShaderManager()->selectShader("occlusion_test", *pCamera, point_lights, directional_lights, spot_lights, 0, viewport, matModel, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, KRNode::RENDER_PASS_FORWARD_TRANSPARENT, KRVector3::Zero(), 0.0f)) {
                        GLDEBUG(glDrawArrays(GL_TRIANGLE_STRIP, 0, 14));
                        m_pContext->getModelManager()->log_draw_call(renderPass, "octree", "occlusion_test", 14);
                    }
                    
                    if(renderPass == KRNode::RENDER_PASS_FORWARD_OPAQUE ||
                       renderPass == KRNode::RENDER_PASS_DEFERRED_GBUFFER ||
                       renderPass == KRNode::RENDER_PASS_DEFERRED_OPAQUE ||
                       renderPass == KRNode::RENDER_PASS_SHADOWMAP) {
                        
                        // Re-enable z-buffer write
                        GLDEBUG(glDepthMask(GL_TRUE));
                    }
                    
                    pOctreeNode->endOcclusionQuery();
                    
                    if(renderPass != KRNode::RENDER_PASS_FORWARD_TRANSPARENT && renderPass != KRNode::RENDER_PASS_ADDITIVE_PARTICLES && renderPass != KRNode::RENDER_PASS_VOLUMETRIC_EFFECTS_ADDITIVE) {
                        GLDEBUG(glDisable(GL_BLEND));
                    } else if(renderPass == KRNode::RENDER_PASS_FORWARD_TRANSPARENT) {
                        GLDEBUG(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
                    } else {
                        GLDEBUG(glBlendFunc(GL_ONE, GL_ONE)); // RENDER_PASS_FORWARD_TRANSPARENT and RENDER_PASS_VOLUMETRIC_EFFECTS_ADDITIVE
                    }
                    
                    
                    if(bVisible) {
                        // Schedule a pass to get the result of the occlusion test only for future frames and passes, without rendering the model or recurring further
                        remainingOctreesTestResultsOnly.push_back(pOctreeNode);
                    } else {
                        // Schedule a pass to get the result of the occlusion test and continue recursion and rendering if test is true
                        remainingOctreesTestResults.push_back(pOctreeNode);
                    }
                }
                
                if(bVisible) {
                    
                    // Add lights that influence this octree level and its children to the stack
                    int directional_light_count = 0;
                    int spot_light_count = 0;
                    int point_light_count = 0;
                    for(std::set<KRNode *>::iterator itr=pOctreeNode->getSceneNodes().begin(); itr != pOctreeNode->getSceneNodes().end(); itr++) {
                        KRNode *node = (*itr);
                        KRDirectionalLight *directional_light = dynamic_cast<KRDirectionalLight *>(node);
                        if(directional_light) {
                            directional_lights.push_back(directional_light);
                            directional_light_count++;
                        }
                        KRSpotLight *spot_light = dynamic_cast<KRSpotLight *>(node);
                        if(spot_light) {
                            spot_lights.push_back(spot_light);
                            spot_light_count++;
                        }
                        KRPointLight *point_light = dynamic_cast<KRPointLight *>(node);
                        if(point_light) {
                            point_lights.push_back(point_light);
                            point_light_count++;
                        }
                    }
                    
                    // Render objects that are at this octree level
                    for(std::set<KRNode *>::iterator itr=pOctreeNode->getSceneNodes().begin(); itr != pOctreeNode->getSceneNodes().end(); itr++) {
                        //assert(pOctreeNode->getBounds().contains((*itr)->getBounds()));  // Sanity check
                        (*itr)->render(pCamera, point_lights, directional_lights, spot_lights, viewport, renderPass);
                    }
                    
                    // Render child octrees
                    const int *childOctreeOrder = renderPass == KRNode::RENDER_PASS_FORWARD_TRANSPARENT || renderPass == KRNode::RENDER_PASS_ADDITIVE_PARTICLES || renderPass == KRNode::RENDER_PASS_VOLUMETRIC_EFFECTS_ADDITIVE ? viewport.getBackToFrontOrder() : viewport.getFrontToBackOrder();
                    
                    for(int i=0; i<8; i++) {
                        render(pOctreeNode->getChildren()[childOctreeOrder[i]], visibleBounds, pCamera, point_lights, directional_lights, spot_lights, viewport, renderPass, remainingOctrees, remainingOctreesTestResults, remainingOctreesTestResultsOnly, false, false);
                    }
                    
                    // Remove lights added at this octree level from the stack
                    while(directional_light_count--) {
                        directional_lights.pop_back();
                    }
                    while(spot_light_count--) {
                        spot_lights.pop_back();
                    }
                    while(point_light_count--) {
                        point_lights.pop_back();
                    }
                }
            }
            
        }
    }
//  fprintf(stderr, "Octree culled: (%f, %f, %f) - (%f, %f, %f)\n", pOctreeNode->getBounds().min.x, pOctreeNode->getBounds().min.y, pOctreeNode->getBounds().min.z, pOctreeNode->getBounds().max.x, pOctreeNode->getBounds().max.y, pOctreeNode->getBounds().max.z);
}

std::string KRScene::getExtension() {
    return "krscene";
}

KRNode *KRScene::getRootNode() {
    return m_pRootNode;
}

bool KRScene::save(KRDataBlock &data) {
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement *scene_node =  doc.NewElement( "scene" );
    doc.InsertEndChild(scene_node);
    m_pRootNode->saveXML(scene_node);
    scene_node->SetAttribute("skybox", m_skyBoxName.c_str());  // This is temporary until the camera is moved into the scene graph
    
    tinyxml2::XMLPrinter p;
    doc.Print(&p);
    data.append((void *)p.CStr(), strlen(p.CStr())+1);
    
    return true;
}

KRScene *KRScene::Load(KRContext &context, const std::string &name, KRDataBlock *data)
{
    std::string xml_string = data->getString();
    delete data;
    tinyxml2::XMLDocument doc;
    doc.Parse(xml_string.c_str());
    KRScene *new_scene = new KRScene(context, name);
    
    tinyxml2::XMLElement *scene_element = doc.RootElement();
    const char *szSkyBoxName = scene_element->Attribute("skybox");
    new_scene->m_skyBoxName = szSkyBoxName ? szSkyBoxName : "";  // This is temporary until the camera is moved into the scene graph
    
    KRNode *n = KRNode::LoadXML(*new_scene, scene_element->FirstChildElement());
    if(n) {
        new_scene->getRootNode()->addChild(n);
    }
    
    
    return new_scene;
}



KRLight *KRScene::getFirstLight()
{
    if(m_pFirstLight == NULL) {
        m_pFirstLight = find<KRLight>();
    }
    return m_pFirstLight;
}

void KRScene::notify_sceneGraphCreate(KRNode *pNode)
{
//    m_nodeTree.add(pNode);
//    if(pNode->hasPhysics()) {
//        m_physicsNodes.insert(pNode);
//    }
    m_newNodes.insert(pNode);
}

void KRScene::notify_sceneGraphModify(KRNode *pNode)
{
    //    m_nodeTree.update(pNode);
    m_modifiedNodes.insert(pNode);
}

void KRScene::notify_sceneGraphDelete(KRNode *pNode)
{
    m_nodeTree.remove(pNode);
    m_physicsNodes.erase(pNode);
    KRAmbientZone *AmbientZoneNode = dynamic_cast<KRAmbientZone *>(pNode);
    if(AmbientZoneNode) {
        m_ambientZoneNodes.erase(AmbientZoneNode);
    }
    KRReverbZone *ReverbZoneNode = dynamic_cast<KRReverbZone *>(pNode);
    if(ReverbZoneNode) {
        m_reverbZoneNodes.erase(ReverbZoneNode);
    }
    KRLocator *locator = dynamic_cast<KRLocator *>(pNode);
    if(locator) {
        m_locatorNodes.erase(locator);
    }
    m_modifiedNodes.erase(pNode);
    if(!m_newNodes.erase(pNode)) {
        m_nodeTree.remove(pNode);
    }
}

void KRScene::updateOctree(const KRViewport &viewport)
{
    m_pRootNode->updateLODVisibility(viewport);
    
    std::set<KRNode *> newNodes = std::move(m_newNodes);
    std::set<KRNode *> modifiedNodes = std::move(m_modifiedNodes);
    m_newNodes.clear();
    m_modifiedNodes.clear();
    
    for(std::set<KRNode *>::iterator itr=newNodes.begin(); itr != newNodes.end(); itr++) {
        KRNode *node = *itr;
        m_nodeTree.add(node);
        if(node->hasPhysics()) {
            m_physicsNodes.insert(node);
        }
        KRAmbientZone *ambientZoneNode = dynamic_cast<KRAmbientZone *>(node);
        if(ambientZoneNode) {
            m_ambientZoneNodes.insert(ambientZoneNode);
        }
        KRReverbZone *reverbZoneNode = dynamic_cast<KRReverbZone *>(node);
        if(reverbZoneNode) {
            m_reverbZoneNodes.insert(reverbZoneNode);
        }
        KRLocator *locatorNode = dynamic_cast<KRLocator *>(node);
        if(locatorNode) {
            m_locatorNodes.insert(locatorNode);
        }
        
    }
    for(std::set<KRNode *>::iterator itr=modifiedNodes.begin(); itr != modifiedNodes.end(); itr++) {
        KRNode *node = *itr;
        if(node->lodIsVisible()) {
            m_nodeTree.update(node);
        }
        if(node->hasPhysics()) {
            m_physicsNodes.insert(node);
        } else if(!node->hasPhysics()) {
            m_physicsNodes.erase(node);
        }
    }
}

void KRScene::physicsUpdate(float deltaTime)
{
    for(std::set<KRNode *>::iterator itr=m_physicsNodes.begin(); itr != m_physicsNodes.end(); itr++) {
        (*itr)->physicsUpdate(deltaTime);
    }
}

void KRScene::addDefaultLights()
{
    KRDirectionalLight *light1 = new KRDirectionalLight(*this, "default_light1");
    
    light1->setLocalRotation((KRQuaternion(KRVector3(0.0, M_PI * 0.10, 0.0)) * KRQuaternion(KRVector3(0.0, 0.0, -M_PI * 0.15))).eulerXYZ());
    m_pRootNode->addChild(light1);
}

KRAABB KRScene::getRootOctreeBounds()
{
    if(m_nodeTree.getRootNode()) {
        return m_nodeTree.getRootNode()->getBounds();
    } else {
        return KRAABB(-KRVector3::One(), KRVector3::One());
    }
}


bool KRScene::lineCast(const KRVector3 &v0, const KRVector3 &v1, KRHitInfo &hitinfo, unsigned int layer_mask)
{
    return m_nodeTree.lineCast(v0, v1, hitinfo, layer_mask);
}

bool KRScene::rayCast(const KRVector3 &v0, const KRVector3 &dir, KRHitInfo &hitinfo, unsigned int layer_mask)
{
    return m_nodeTree.rayCast(v0, dir, hitinfo, layer_mask);
}

bool KRScene::sphereCast(const KRVector3 &v0, const KRVector3 &v1, float radius, KRHitInfo &hitinfo, unsigned int layer_mask)
{
    return m_nodeTree.sphereCast(v0, v1, radius, hitinfo, layer_mask);
}


