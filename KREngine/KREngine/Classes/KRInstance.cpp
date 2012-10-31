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
#import "KRModel.h"
#include <assert.h>

KRInstance::KRInstance(KRScene &scene, std::string instance_name, std::string model_name, std::string light_map, float lod_min_coverage) : KRNode(scene, instance_name) {
    m_lightMap = light_map;
    m_pLightMap = NULL;
    m_model_name = model_name;
    m_min_lod_coverage = lod_min_coverage;
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
    e->SetAttribute("lod_min_coverage", m_min_lod_coverage);
    return e;
}


KRMat4 &KRInstance::getModelMatrix() {
    calcModelMatrix();
    return m_modelMatrix;
}

void KRInstance::loadModel() {
    if(m_models.size() == 0) {
        m_models = m_pContext->getModelManager()->getModel(m_model_name.c_str()); // The model manager returns the LOD levels in sorted order, with the highest detail first
        if(m_models.size() > 0) {
            getScene().notify_sceneGraphModify(this);
        }
//        if(m_pModel == NULL) {
//            fprintf(stderr, "KREngine - Model not found: %s\n", m_model_name.c_str());
//        }
    }
}

#if TARGET_OS_IPHONE

void KRInstance::render(KRCamera *pCamera, KRContext *pContext, const KRViewport &viewport, KRVector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers, KRNode::RenderPass renderPass) {

    calcModelMatrix();
    
    KRNode::render(pCamera, pContext, viewport, lightDirection, pShadowMatrices, shadowDepthTextures, cShadowBuffers, renderPass);
    
    if(renderPass != KRNode::RENDER_PASS_DEFERRED_LIGHTS && (renderPass != KRNode::RENDER_PASS_FORWARD_TRANSPARENT || this->hasTransparency()) && renderPass != KRNode::RENDER_PASS_FLARES) {
        // Don't render meshes on second pass of the deferred lighting renderer, as only lights will be applied
    
        loadModel();
        
        if(m_models.size() > 0) {
            KRMat4 projectionMatrix;
            if(renderPass != KRNode::RENDER_PASS_SHADOWMAP) {
                projectionMatrix = pCamera->getProjectionMatrix();
            }
            KRMat4 matMVP = m_modelMatrix * viewport.getViewProjectionMatrix();
            float lod_coverage = getBounds().coverage(matMVP, pCamera->getViewportSize()); // This also checks the view frustrum culling
            if(lod_coverage > m_min_lod_coverage) {
                
                // ---===--- Select the best LOD model based on screen coverage ---===---
                std::vector<KRModel *>::iterator itr=m_models.begin();
                KRModel *pModel = *itr++;
                
                while(itr != m_models.end()) {
                    KRModel *pLODModel = *itr++;
                    if((float)pLODModel->getLODCoverage() / 100.0f > lod_coverage) {
                        pModel = pLODModel;
                    } else {
                        break;
                    }
                }
                
                if(m_pLightMap == NULL && m_lightMap.size()) {
                    m_pLightMap = pContext->getTextureManager()->getTexture(m_lightMap.c_str());
                }
                
                if(cShadowBuffers == 0 && m_pLightMap && pCamera->bEnableLightMap && renderPass != RENDER_PASS_SHADOWMAP) {
                    m_pContext->getTextureManager()->selectTexture(3, m_pLightMap, 2048);
                }
                
                pModel->render(pCamera, pContext, viewport, m_modelMatrix, lightDirection, pShadowMatrices, shadowDepthTextures, cShadowBuffers, m_pLightMap, renderPass);
            }
        }
    }
}

#endif

bool KRInstance::hasTransparency() {
    if(m_models.size() > 0) {
        return m_models[0]->hasTransparency();
    } else {
        return false;
    }
}

KRAABB KRInstance::getBounds() {
    calcModelMatrix();
    loadModel();
    
    KRVector3 meshMin, meshMax;
    if(m_models.size() > 0) {
        meshMin = m_models[0]->getMinPoint();
        meshMax = m_models[0]->getMaxPoint();
    } else {
        meshMin = -KRVector3::Max();
        meshMax = KRVector3::Max();
    }
    
    KRVector3 min, max;
    for(int iCorner=0; iCorner < 8; iCorner++) {
        KRVector3 cornerVertex = KRVector3(
           (iCorner & 1) == 0 ? meshMin.x : meshMax.x,
           (iCorner & 2) == 0 ? meshMin.y : meshMax.y,
           (iCorner & 4) == 0 ? meshMin.z : meshMax.z);
        
        cornerVertex = KRMat4::Dot(m_modelMatrix, cornerVertex);
        if(iCorner == 0) {
            // Prime with first point
            min = cornerVertex;
            max = cornerVertex;
        } else {
            
            if(cornerVertex.x < min.x) {
                min.x = cornerVertex.x;
            }
            if(cornerVertex.y < min.y) {
                min.y = cornerVertex.y;
            }
            if(cornerVertex.z < min.z) {
                min.z = cornerVertex.z;
            }
            if(cornerVertex.x > max.x) {
                max.x = cornerVertex.x;
            }
            if(cornerVertex.y > max.y) {
                max.y = cornerVertex.y;
            }
            if(cornerVertex.z > max.z) {
                max.z = cornerVertex.z;
            }
        }
    }
    
    return KRAABB(min, max);
}

void KRInstance::calcModelMatrix()
{
    m_modelMatrix = KRMat4();
//    m_modelMatrix.scale(m_localScale);
//    m_modelMatrix.rotate(m_localRotation.x, X_AXIS);
//    m_modelMatrix.rotate(m_localRotation.y, Y_AXIS);
//    m_modelMatrix.rotate(m_localRotation.z, Z_AXIS);
//    m_modelMatrix.translate(m_localTranslation);
}
