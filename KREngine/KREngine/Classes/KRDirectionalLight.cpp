//
//  KRDirectionalLight.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-05.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include <iostream>

#import "KRDirectionalLight.h"

KRDirectionalLight::KRDirectionalLight(std::string name) : KRLight(name)
{

}

KRDirectionalLight::~KRDirectionalLight()
{
    
}

std::string KRDirectionalLight::getElementName() {
    return "directional_light";
}

#if TARGET_OS_IPHONE

void KRDirectionalLight::render(KRCamera *pCamera, KRContext *pContext, KRBoundingVolume &frustrumVolume, bool bRenderShadowMap, KRMat4 &viewMatrix, KRVector3 &cameraPosition, KRVector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers, int gBufferPass) {
    
    if(gBufferPass == 2) {
        // Lights are rendered on the second pass of the deferred renderer
        /*
        
        if(m_pModel == NULL) {
            m_pModel = pContext->getModelManager()->getModel(m_model_name.c_str());
        }
        
        if(m_pModel != NULL && (getExtents(pContext).test_intersect(frustrumVolume) || bRenderShadowMap)) {
            
            if(m_pLightMap == NULL && m_lightMap.size()) {
                m_pLightMap = pContext->getTextureManager()->getTexture(m_lightMap.c_str());
            }
            
            if(cShadowBuffers == 0 && m_pLightMap && pCamera->bEnableLightMap && !bRenderShadowMap) {
                int iTextureName = m_pLightMap->getName();
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, iTextureName);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            }
            
            KRMat4 projectionMatrix;
            if(!bRenderShadowMap) {
                projectionMatrix = pCamera->getProjectionMatrix();
            }
            KRMat4 mvpmatrix = m_modelMatrix * viewMatrix * projectionMatrix;
            KRMat4 matModelToView = viewMatrix * m_modelMatrix;
            matModelToView.transpose();
            matModelToView.invert();
            
            // Transform location of camera to object space for calculation of specular halfVec
            KRMat4 inverseModelMatrix = m_modelMatrix;
            inverseModelMatrix.invert();
            KRVector3 cameraPosObject = inverseModelMatrix.dot(cameraPosition);
            KRVector3 lightDirObject = inverseModelMatrix.dot(lightDirection);
            
            m_pModel->render(pCamera, pContext, bRenderShadowMap, matModelToView, mvpmatrix, cameraPosObject, lightDirection, pShadowMatrices, shadowDepthTextures, cShadowBuffers, m_pLightMap, gBufferPass);
            
        }
         */
        

    }
    
    KRNode::render(pCamera, pContext, frustrumVolume, bRenderShadowMap, viewMatrix, cameraPosition, lightDirection, pShadowMatrices, shadowDepthTextures, cShadowBuffers, gBufferPass);
}

#endif