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

#include <iostream>

#import "KRVector3.h"
#import "KRMat4.h"
#import "tinyxml2.h"

#import "KRDirectionalLight.h"

#import "KRScene.h"
#import "KRNode.h"
#import "KRStockGeometry.h"

KRScene::KRScene(KRContext &context, std::string name) : KRResource(context, name) {
    m_pContext = &context;
    m_pFirstDirectionalLight = NULL;
    m_pRootNode = new KRNode(*this, "scene_root");
    
    sun_yaw = 4.333; // TODO - Remove temporary testing code
    sun_pitch = 0.55;
    m_skyBoxName = "";
}
KRScene::~KRScene() {
    delete m_pRootNode;
    m_pRootNode = NULL;
}

#if TARGET_OS_IPHONE

void KRScene::render(KRCamera *pCamera, std::set<KRAABB> &visibleBounds, KRContext *pContext, const KRViewport &viewport, KRVector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers, KRNode::RenderPass renderPass, std::set<KRAABB> &newVisibleBounds) {
    
    updateOctree();
    pCamera->setSkyBox(m_skyBoxName); // This is temporary until the camera is moved into the scene graph
    
    if(renderPass != KRNode::RENDER_PASS_SHADOWMAP) {
    
        if(cShadowBuffers > 0) {
            m_pContext->getTextureManager()->selectTexture(3, NULL, 0);
            GLDEBUG(glActiveTexture(GL_TEXTURE3));
            GLDEBUG(glBindTexture(GL_TEXTURE_2D, shadowDepthTextures[0]));
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        }
        
        if(cShadowBuffers > 1) {
            m_pContext->getTextureManager()->selectTexture(4, NULL, 0);
            GLDEBUG(glActiveTexture(GL_TEXTURE4));
            GLDEBUG(glBindTexture(GL_TEXTURE_2D, shadowDepthTextures[1]));
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        }
        
        if(cShadowBuffers > 2) {
            m_pContext->getTextureManager()->selectTexture(5, NULL, 0);
            GLDEBUG(glActiveTexture(GL_TEXTURE5));
            GLDEBUG(glBindTexture(GL_TEXTURE_2D, shadowDepthTextures[2]));
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        }
    }
        
    KRVector3 forward_render_light_direction = lightDirection;
    KRDirectionalLight *directional_light = getFirstDirectionalLight();
    if(directional_light) {
        forward_render_light_direction = directional_light->getWorldLightDirection();
        KRVector3 sun_color = directional_light->getColor() * (directional_light->getIntensity() / 100.0f);
        pCamera->dSunR = sun_color.x;
        pCamera->dSunG = sun_color.y;
        pCamera->dSunB = sun_color.z;
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
            render(*octree_itr, visibleBounds, pCamera, pContext, viewport, forward_render_light_direction, pShadowMatrices, shadowDepthTextures, cShadowBuffers, renderPass, newRemainingOctrees, newRemainingOctreesTestResults, remainingOctreesTestResultsOnly, newVisibleBounds, false, false);
        }
        for(std::vector<KROctreeNode *>::iterator octree_itr = remainingOctreesTestResults.begin(); octree_itr != remainingOctreesTestResults.end(); octree_itr++) {
            render(*octree_itr, visibleBounds, pCamera, pContext, viewport, forward_render_light_direction, pShadowMatrices, shadowDepthTextures, cShadowBuffers, renderPass, newRemainingOctrees, newRemainingOctreesTestResults, remainingOctreesTestResultsOnly, newVisibleBounds, true, false);
        }
        remainingOctrees = newRemainingOctrees;
        remainingOctreesTestResults = newRemainingOctreesTestResults;
    }
    
    newRemainingOctrees.clear();
    newRemainingOctreesTestResults.clear();
    for(std::vector<KROctreeNode *>::iterator octree_itr = remainingOctreesTestResultsOnly.begin(); octree_itr != remainingOctreesTestResultsOnly.end(); octree_itr++) {
        render(*octree_itr, visibleBounds, pCamera, pContext, viewport, forward_render_light_direction, pShadowMatrices, shadowDepthTextures, cShadowBuffers, renderPass, newRemainingOctrees, newRemainingOctreesTestResults, remainingOctreesTestResultsOnly, newVisibleBounds, true, true);
    }
    
    std::set<KRNode *> outerNodes = std::set<KRNode *>(m_nodeTree.getOuterSceneNodes()); // HACK - Copying the std::set as it is potentially modified as KRNode's update their bounds during the iteration.  This is very expensive and will be eliminated in the future.
    for(std::set<KRNode *>::iterator itr=outerNodes.begin(); itr != outerNodes.end(); itr++) {
        (*itr)->render(pCamera, pContext, viewport, lightDirection, pShadowMatrices, shadowDepthTextures, cShadowBuffers, renderPass);
    }
}

void KRScene::render(KROctreeNode *pOctreeNode, std::set<KRAABB> &visibleBounds, KRCamera *pCamera, KRContext *pContext, const KRViewport &viewport, KRVector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers, KRNode::RenderPass renderPass, std::vector<KROctreeNode *> &remainingOctrees, std::vector<KROctreeNode *> &remainingOctreesTestResults, std::vector<KROctreeNode *> &remainingOctreesTestResultsOnly, std::set<KRAABB> &newVisibleBounds, bool bOcclusionResultsPass, bool bOcclusionTestResultsOnly)
{    
    if(pOctreeNode) {
        
        KRAABB octreeBounds = pOctreeNode->getBounds();
        
        if(bOcclusionResultsPass) {
            // ----====---- Occlusion results pass ----====----
            if(pOctreeNode->m_occlusionTested) {
                GLuint params = 0;
                GLDEBUG(glGetQueryObjectuivEXT(pOctreeNode->m_occlusionQuery, GL_QUERY_RESULT_EXT, &params));
                if(params) {
                    newVisibleBounds.insert(octreeBounds); // Record the actual tests that succeeded during this frame
                    visibleBounds.insert(octreeBounds); // Update the list of tests that we won't repeat for subsequent passes during this frame
                    if(!bOcclusionTestResultsOnly) {
                        // Schedule a pass to perform the rendering
                        remainingOctrees.push_back(pOctreeNode);
                    }
                }
                
                GLDEBUG(glDeleteQueriesEXT(1, &pOctreeNode->m_occlusionQuery));
                pOctreeNode->m_occlusionTested = false;
                pOctreeNode->m_occlusionQuery = 0;
            }
        } else {


      KRMat4 projectionMatrix;
      if(renderPass != KRNode::RENDER_PASS_SHADOWMAP) {
          projectionMatrix = pCamera->getProjectionMatrix();
      }
            
            float min_coverage = 0.0f; // 1.0f / 1024.0f / 768.0f; // FINDME - HACK - Need to dynamically select the absolute minimum based on the render buffer size
            
        float lod_coverage = pOctreeNode->getBounds().coverage(viewport.getViewProjectionMatrix(), pCamera->getViewportSize()); // This also checks the view frustrum culling
        if(lod_coverage > min_coverage) {

                // ----====---- Rendering and occlusion test pass ----====----
                bool bVisible = false;
                bool bNeedOcclusionTest = true;
          
                if(!bVisible) {
                    // Assume bounding boxes are visible without occlusion test queries if the camera is inside the box.
                    // The near clipping plane of the camera is taken into consideration by expanding the match area
                    KRAABB cameraExtents = KRAABB(viewport.getCameraPosition() - KRVector3(pCamera->getPerspectiveNearZ()), viewport.getCameraPosition() + KRVector3(pCamera->getPerspectiveNearZ()));
                    bVisible = octreeBounds.intersects(cameraExtents);
                    if(bVisible) {
                        newVisibleBounds.insert(octreeBounds); // Record the actual tests that succeeded during this frame
                        visibleBounds.insert(octreeBounds); // Update the list of tests that we won't repeat for subsequent passes during this frame
                        bNeedOcclusionTest = false;
                    }
                }
                
                if(!bVisible) {
                    // Check if an occlusion query from the prior pass has returned true
                    bVisible = newVisibleBounds.find(octreeBounds) != newVisibleBounds.end();
                    if(bVisible) {
                        bNeedOcclusionTest = false;
                    }
                }
                
                if(!bVisible) {
                    // Take advantage of temporal consistency of visible elements from frame to frame
                    // If the previous frame rendered this octree, then attempt to render it in this frame without performing a pre-occlusion test
                    bVisible = visibleBounds.find(octreeBounds) != visibleBounds.end();
                    // We don't set bNeedOcclusionTest to false here, as we need to perform an occlusion test to record if this octree node was visible for the next frame
                }
                
                if(!bVisible) {
                    // Optimization: If this is an empty octree node with only a single child node, then immediately try to render the child node without an occlusion test for this higher level, as it would be more expensive than the occlusion test for the child
                    if(pOctreeNode->getSceneNodes().empty()) {
                        int child_count = 0;
                        for(int i=0; i<8; i++) {
                            if(pOctreeNode->getChildren()[i] != NULL) child_count++;
                        }
                        if(child_count == 1) bVisible = true;
                    }
                }
                
                if(bNeedOcclusionTest) {
                    pOctreeNode->beginOcclusionQuery();
                    
                    KRShader *pVisShader = m_pContext->getShaderManager()->getShader("occlusion_test", pCamera, false, false, false, 0, false, false, false, false, false, false, false, false, false, false, false, false, false, KRNode::RENDER_PASS_FORWARD_TRANSPARENT);
                    
                    KRMat4 projectionMatrix = pCamera->getProjectionMatrix();
                    
                    m_pContext->getModelManager()->bindVBO((void *)KRENGINE_VBO_3D_CUBE, KRENGINE_VBO_3D_CUBE_SIZE, true, false, false, false, false);
                    KRMat4 matModel = KRMat4();
                    matModel.scale(octreeBounds.size() / 2.0f);
                    matModel.translate(octreeBounds.center());
                    KRMat4 mvpmatrix = matModel * viewport.getViewProjectionMatrix();
                    
                    // Enable additive blending
                    if(renderPass != KRNode::RENDER_PASS_FORWARD_TRANSPARENT && renderPass != KRNode::RENDER_PASS_FLARES) {
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
                    
                    if(pVisShader->bind(viewport, matModel, lightDirection, pShadowMatrices, shadowDepthTextures, 0, KRNode::RENDER_PASS_FORWARD_TRANSPARENT)) {
                        GLDEBUG(glDrawArrays(GL_TRIANGLE_STRIP, 0, 14));
                    }
                    
                    if(renderPass == KRNode::RENDER_PASS_FORWARD_OPAQUE ||
                       renderPass == KRNode::RENDER_PASS_DEFERRED_GBUFFER ||
                       renderPass == KRNode::RENDER_PASS_DEFERRED_OPAQUE ||
                       renderPass == KRNode::RENDER_PASS_SHADOWMAP) {
                        
                        // Re-enable z-buffer write
                        GLDEBUG(glDepthMask(GL_TRUE));
                    }
                    
                    pOctreeNode->endOcclusionQuery();
                    
                    if(renderPass != KRNode::RENDER_PASS_FORWARD_TRANSPARENT && renderPass != KRNode::RENDER_PASS_FLARES) {
                        GLDEBUG(glDisable(GL_BLEND));
                    } else if(renderPass == KRNode::RENDER_PASS_FORWARD_TRANSPARENT) {
                        GLDEBUG(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
                    } else {
                        GLDEBUG(glBlendFunc(GL_ONE, GL_ONE));
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
                    
                    for(std::set<KRNode *>::iterator itr=pOctreeNode->getSceneNodes().begin(); itr != pOctreeNode->getSceneNodes().end(); itr++) {
                        //assert(pOctreeNode->getBounds().contains((*itr)->getBounds()));  // Sanity check
                        (*itr)->render(pCamera, pContext, viewport, lightDirection, pShadowMatrices, shadowDepthTextures, cShadowBuffers, renderPass);
                    }
                    
                    const int *childOctreeOrder = renderPass == KRNode::RENDER_PASS_FORWARD_TRANSPARENT || renderPass == KRNode::RENDER_PASS_FLARES ? viewport.getBackToFrontOrder() : viewport.getFrontToBackOrder();
                    
                    for(int i=0; i<8; i++) {
                        render(pOctreeNode->getChildren()[childOctreeOrder[i]], visibleBounds, pCamera, pContext, viewport, lightDirection, pShadowMatrices, shadowDepthTextures, cShadowBuffers, renderPass, remainingOctrees, remainingOctreesTestResults, remainingOctreesTestResultsOnly, newVisibleBounds, false, false);
                    }
                }
            }
            
        }
    }
//  fprintf(stderr, "Octree culled: (%f, %f, %f) - (%f, %f, %f)\n", pOctreeNode->getBounds().min.x, pOctreeNode->getBounds().min.y, pOctreeNode->getBounds().min.z, pOctreeNode->getBounds().max.x, pOctreeNode->getBounds().max.y, pOctreeNode->getBounds().max.z);
}

#endif

std::string KRScene::getExtension() {
    return "krscene";
}

KRNode *KRScene::getRootNode() {
    return m_pRootNode;
}

bool KRScene::save(const std::string& path) {
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement *scene_node =  doc.NewElement( "scene" );
    doc.InsertEndChild(scene_node);
    m_pRootNode->saveXML(scene_node);
    scene_node->SetAttribute("skybox", m_skyBoxName.c_str());  // This is temporary until the camera is moved into the scene graph
    doc.SaveFile(path.c_str());
    return true;
}


KRDirectionalLight *KRScene::findFirstDirectionalLight(KRNode &node) {
    KRDirectionalLight *pLight = dynamic_cast<KRDirectionalLight *>(&node);
    if(pLight) {
        return pLight;
    } else {
        const std::vector<KRNode *> children = node.getChildren();
        for(std::vector<KRNode *>::const_iterator itr=children.begin(); itr < children.end(); ++itr) {
            pLight = findFirstDirectionalLight(*(*itr));
            if(pLight) {
                return pLight;
            }
        }
    }
    return NULL;
}

KRScene *KRScene::Load(KRContext &context, const std::string &name, KRDataBlock *data)
{
    data->append((void *)"\0", 1); // Ensure data is null terminated, to read as a string safely
    tinyxml2::XMLDocument doc;
    doc.Parse((char *)data->getStart());
    KRScene *new_scene = new KRScene(context, name);
    
    tinyxml2::XMLElement *scene_element = doc.RootElement();
    const char *szSkyBoxName = scene_element->Attribute("skybox");
    new_scene->m_skyBoxName = szSkyBoxName ? szSkyBoxName : "";  // This is temporary until the camera is moved into the scene graph
    
    KRNode *n = KRNode::LoadXML(*new_scene, scene_element->FirstChildElement());
    if(n) {
        new_scene->getRootNode()->addChild(n);
    }
    
    delete data;
    return new_scene;
}

KRDirectionalLight *KRScene::getFirstDirectionalLight()
{
    if(m_pFirstDirectionalLight == NULL) {
        m_pFirstDirectionalLight = findFirstDirectionalLight(*m_pRootNode);
    }
    return m_pFirstDirectionalLight;
}

void KRScene::notify_sceneGraphCreate(KRNode *pNode)
{
    m_nodeTree.add(pNode);
//    m_newNodes.insert(pNode);
}

void KRScene::notify_sceneGraphDelete(KRNode *pNode)
{
    m_nodeTree.remove(pNode);
//    
//    m_modifiedNodes.erase(pNode);
//    if(!m_newNodes.erase(pNode)) {
//        m_nodeTree.remove(pNode);
//    }
}

void KRScene::notify_sceneGraphModify(KRNode *pNode)
{
    m_nodeTree.update(pNode);
//    m_modifiedNodes.insert(pNode);
}

void KRScene::updateOctree()
{
//    for(std::set<KRNode *>::iterator itr=m_newNodes.begin(); itr != m_newNodes.end(); itr++) {
//        m_nodeTree.add(*itr);
//    }
//    for(std::set<KRNode *>::iterator itr=m_modifiedNodes.begin(); itr != m_modifiedNodes.end(); itr++) {
//        m_nodeTree.update(*itr);
//    }
//    m_newNodes.clear();
//    m_modifiedNodes.clear();
}
#if TARGET_OS_IPHONE

#endif
