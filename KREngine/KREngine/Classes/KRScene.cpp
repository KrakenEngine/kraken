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

#import "KRScene.h"

KRScene::KRScene(std::string name) : KRResource(name) {
    m_pExtents = NULL;
    m_pRootNode = new KRNode("scene_root");
}
KRScene::~KRScene() {
    for(vector<KRInstance *>::iterator itr = m_instances.begin(); itr != m_instances.end(); ++itr){
        delete *itr;
    }
    m_instances.empty();
    
    delete m_pRootNode;
    m_pRootNode = NULL;
    
    clearExtents();
}
KRInstance *KRScene::addInstance(std::string instance_name, std::string model_name, KRMat4 modelMatrix, std::string shadow_map) {
    clearExtents();
    KRInstance *pInstance = new KRInstance(instance_name, model_name, modelMatrix, shadow_map);
    m_instances.push_back(pInstance);
    return pInstance;
}

#if TARGET_OS_IPHONE

void KRScene::render(KRCamera *pCamera, KRModelManager *pModelManager, KRBoundingVolume &frustrumVolume, KRMaterialManager *pMaterialManager, bool bRenderShadowMap, KRMat4 &viewMatrix, KRVector3 &cameraPosition, KRVector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers, KRShaderManager *pShaderManager, KRTextureManager *pTextureManager) {
    
    if(cShadowBuffers > 0 && !bRenderShadowMap) {
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, shadowDepthTextures[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    
    if(cShadowBuffers > 1 && !bRenderShadowMap) {
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, shadowDepthTextures[1]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    
    if(cShadowBuffers > 2 && !bRenderShadowMap) {
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, shadowDepthTextures[2]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    
    
    for(vector<KRInstance *>::iterator itr = m_instances.begin(); itr != m_instances.end(); ++itr){
        KRInstance *pInstance = *itr;
        
        if(pInstance->getExtents(pModelManager).test_intersect(frustrumVolume) || bRenderShadowMap) {
            pInstance->render(pCamera, pModelManager, pMaterialManager, bRenderShadowMap, viewMatrix, cameraPosition, lightDirection, pShadowMatrices, shadowDepthTextures, cShadowBuffers, pShaderManager, pTextureManager);
        }
    }
}

#endif

void KRScene::calcExtents(KRModelManager *pModelManager) {
    clearExtents();
    for(vector<KRInstance *>::iterator itr = m_instances.begin(); itr != m_instances.end(); ++itr){
        KRInstance *pInstance = *itr;
        if(m_pExtents) {
            *m_pExtents = m_pExtents->get_union(pInstance->getExtents(pModelManager));
        } else {
            m_pExtents = new KRBoundingVolume(pInstance->getExtents(pModelManager));
        }
    }
}

KRBoundingVolume KRScene::getExtents(KRModelManager *pModelManager) {
    if(!m_pExtents) {
        calcExtents(pModelManager);
    }
    return *m_pExtents;
}

void KRScene::clearExtents() {
    if(m_pExtents) {
        delete m_pExtents;
        m_pExtents = NULL;
    }
}


std::string KRScene::getExtension() {
    return "krscene";
}

KRNode *KRScene::getRootNode() {
    return m_pRootNode;
}

bool KRScene::save(const std::string& path) {
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLNode *scene_node = doc.InsertEndChild( doc.NewElement( "scene" ));
    m_pRootNode->saveXML(scene_node);
    doc.SaveFile(path.c_str());
    return true;
}


KRScene *KRScene::LoadXML(const std::string& path)
{
    tinyxml2::XMLDocument doc;
    doc.LoadFile(path.c_str());
    KRScene *new_scene = new KRScene(KRResource::GetFileBase(path));
    
    return new_scene;
}
