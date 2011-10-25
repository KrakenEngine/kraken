//
//  KRInstance.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 11-09-29.
//  Copyright 2011 Kearwood Software. All rights reserved.
//

#include <iostream>
#import "KRInstance.h"

KRInstance::KRInstance(KRModel *pModel, const KRMat4 modelMatrix) {
    m_pModel = pModel;
    m_modelMatrix = modelMatrix;
}

KRInstance::~KRInstance() {
    
}

KRMat4 &KRInstance::getModelMatrix() {
    return m_modelMatrix;
}
KRModel *KRInstance::getModel() {
    return m_pModel;
}

void KRInstance::render(KRCamera *pCamera, KRMaterialManager *pMaterialManager, bool bRenderShadowMap, KRMat4 &viewMatrix, Vector3 &cameraPosition, Vector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers) {
    
    KRMat4 projectionMatrix;
    if(!bRenderShadowMap) {
        projectionMatrix = pCamera->getProjectionMatrix();
    }
    KRMat4 mvpmatrix = m_modelMatrix * viewMatrix * projectionMatrix;
    
    // Transform location of camera to object space for calculation of specular halfVec
    KRMat4 inverseModelMatrix = m_modelMatrix;
    inverseModelMatrix.invert();
    Vector3 cameraPosObject = inverseModelMatrix.dot(cameraPosition);
    Vector3 lightDirObject = inverseModelMatrix.dot(lightDirection);
    
    m_pModel->render(pCamera, pMaterialManager, bRenderShadowMap, mvpmatrix, cameraPosObject, lightDirection, pShadowMatrices, shadowDepthTextures, cShadowBuffers);
    
}

KRBoundingVolume KRInstance::getExtents() {
    return KRBoundingVolume(Vector3(m_pModel->getMinX(), m_pModel->getMinY(), m_pModel->getMinZ()), Vector3(m_pModel->getMaxX(), m_pModel->getMaxY(), m_pModel->getMaxZ()), m_modelMatrix);
}