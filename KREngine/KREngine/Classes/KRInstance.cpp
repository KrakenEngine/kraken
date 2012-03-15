//
//  KRInstance.cpp
//  KREngine
//
//  Copyright 2011 Kearwood Gilbert. All rights reserved.
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
    KRMesh *pMesh = m_pModel->getMesh();
    return KRBoundingVolume(Vector3(pMesh->getMinX(), pMesh->getMinY(), pMesh->getMinZ()), Vector3(pMesh->getMaxX(), pMesh->getMaxY(), pMesh->getMaxZ()), m_modelMatrix);
}