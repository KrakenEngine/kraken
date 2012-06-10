//
//  KRInstance.cpp
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
#import "KRInstance.h"
#import "KRContext.h"
#include <assert.h>

KRInstance::KRInstance(std::string instance_name, std::string model_name, const KRMat4 modelMatrix, std::string light_map) : KRNode(instance_name) {
    m_modelMatrix = modelMatrix;
    m_lightMap = light_map;
    m_pLightMap = NULL;
    m_pModel = NULL;
    m_model_name = model_name;
}

KRInstance::~KRInstance() {
    
}

std::string KRInstance::getElementName() {
    return "mesh";
}

tinyxml2::XMLElement *KRInstance::saveXML( tinyxml2::XMLNode *parent)
{
    tinyxml2::XMLElement *e = KRNode::saveXML(parent);
    e->SetAttribute("mesh_name", m_model_name.c_str());
    e->SetAttribute("light_map", m_lightMap.c_str());
    return e;
}


KRMat4 &KRInstance::getModelMatrix() {
    return m_modelMatrix;
}

#if TARGET_OS_IPHONE

void KRInstance::render(KRCamera *pCamera, KRContext *pContext, KRBoundingVolume &frustrumVolume, bool bRenderShadowMap, KRMat4 &viewMatrix, KRVector3 &cameraPosition, KRVector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers, int gBufferPass) {
    
    if(gBufferPass != 2) {
        // Don't render meshes on second pass of the deferred lighting renderer, as only lights will be applied
    
    
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
            KRVector3 cameraPosObject = KRMat4::Dot(inverseModelMatrix, cameraPosition);
            KRVector3 lightDirObject = KRMat4::Dot(inverseModelMatrix, lightDirection);
            
            m_pModel->render(pCamera, pContext, bRenderShadowMap, matModelToView, mvpmatrix, cameraPosObject, lightDirection, pShadowMatrices, shadowDepthTextures, cShadowBuffers, m_pLightMap, gBufferPass);
                
        }
        

    }
    
    KRNode::render(pCamera, pContext, frustrumVolume, bRenderShadowMap, viewMatrix, cameraPosition, lightDirection, pShadowMatrices, shadowDepthTextures, cShadowBuffers, gBufferPass);
}

#endif

void KRInstance::calcExtents(KRContext *pContext) {
    KRNode::calcExtents(pContext);
    if(m_pModel == NULL) {
        m_pModel = pContext->getModelManager()->getModel(m_model_name.c_str());
    }
    assert(m_pModel != NULL);
    
    KRMesh *pMesh = m_pModel->getMesh();
    KRBoundingVolume mesh_bounds = KRBoundingVolume(KRVector3(pMesh->getMinX(), pMesh->getMinY(), pMesh->getMinZ()), KRVector3(pMesh->getMaxX(), pMesh->getMaxY(), pMesh->getMaxZ()), m_modelMatrix);
    if(m_pExtents) {
        *m_pExtents = m_pExtents->get_union(mesh_bounds);
    } else {
        m_pExtents = new KRBoundingVolume(mesh_bounds);
    }
}