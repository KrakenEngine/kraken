//
//  KRScene.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 11-09-29.
//  Copyright 2011 Kearwood Software. All rights reserved.
//

#include <iostream>

#import "KRVector3.h"
#import "KRMat4.h"

#import "KRScene.h"

KRScene::KRScene() {
    m_pExtents = NULL;
}
KRScene::~KRScene() {
    for(vector<KRInstance *>::iterator itr = m_instances.begin(); itr != m_instances.end(); ++itr){
        delete *itr;
    }
    m_instances.empty();
    clearExtents();
}
KRInstance *KRScene::addInstance(KRModel *pModel, KRMat4 modelMatrix) {
    clearExtents();
    KRInstance *pInstance = new KRInstance(pModel, modelMatrix);
    m_instances.push_back(pInstance);
    return pInstance;
}
void KRScene::render(KRCamera *pCamera, KRBoundingVolume &frustrumVolume, KRMaterialManager *pMaterialManager, bool bRenderShadowMap, KRMat4 &viewMatrix, Vector3 &cameraPosition, Vector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers) {
    
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
        
        if(pInstance->getExtents().test_intersect(frustrumVolume) || bRenderShadowMap) {
            pInstance->render(pCamera, pMaterialManager, bRenderShadowMap, viewMatrix, cameraPosition, lightDirection, pShadowMatrices, shadowDepthTextures, cShadowBuffers);
        }
    }
}

void KRScene::calcExtents() {
    clearExtents();
    for(vector<KRInstance *>::iterator itr = m_instances.begin(); itr != m_instances.end(); ++itr){
        KRInstance *pInstance = *itr;
        if(m_pExtents) {
            *m_pExtents = m_pExtents->get_union(pInstance->getExtents());
        } else {
            m_pExtents = new KRBoundingVolume(pInstance->getExtents());
        }
    }
}

KRBoundingVolume KRScene::getExtents() {
    if(!m_pExtents) {
        calcExtents();
    }
    return *m_pExtents;
}

void KRScene::clearExtents() {
    if(m_pExtents) {
        delete m_pExtents;
        m_pExtents = NULL;
    }
}
