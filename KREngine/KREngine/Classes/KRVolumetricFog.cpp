//
//  KRVolumetricFog.cpp
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
#import "KRVolumetricFog.h"
#import "KRContext.h"
#import "KRModel.h"
#import "KRShader.h"
#include <assert.h>

KRVolumetricFog::KRVolumetricFog(KRScene &scene, std::string instance_name, std::string model_name, float lod_min_coverage) : KRNode(scene, instance_name) {
    m_model_name = model_name;
    m_min_lod_coverage = lod_min_coverage;
}

KRVolumetricFog::~KRVolumetricFog() {
    
}

std::string KRVolumetricFog::getElementName() {
    return "volumetric_fog";
}

tinyxml2::XMLElement *KRVolumetricFog::saveXML( tinyxml2::XMLNode *parent)
{
    tinyxml2::XMLElement *e = KRNode::saveXML(parent);
    e->SetAttribute("mesh_name", m_model_name.c_str());
    e->SetAttribute("lod_min_coverage", m_min_lod_coverage);
    return e;
}

void KRVolumetricFog::loadModel() {
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

void KRVolumetricFog::render(KRCamera *pCamera, KRContext *pContext, const KRViewport &viewport, const KRViewport *pShadowViewports, KRVector3 &lightDirection, GLuint *shadowDepthTextures, int cShadowBuffers, KRNode::RenderPass renderPass) {
    
    
    KRNode::render(pCamera, pContext, viewport, pShadowViewports, lightDirection, shadowDepthTextures, cShadowBuffers, renderPass);
    
    if(renderPass == KRNode::RENDER_PASS_ADDITIVE_PARTICLES) {
        // Don't render meshes on second pass of the deferred lighting renderer, as only lights will be applied
        
        loadModel();
        
        KRAABB bounds = getBounds();
        
        if(m_models.size() > 0 && cShadowBuffers > 0) {
            float lod_coverage = bounds.coverage(viewport.getViewProjectionMatrix(), viewport.getSize()); // This also checks the view frustrum culling
            if(lod_coverage > m_min_lod_coverage) {
                
                // ---===--- Select the best LOD model based on screen coverage ---===---
                std::vector<KRModel *>::iterator itr=m_models.begin();
                KRModel *pModel = *itr++;
                
                while(itr != m_models.end()) {
                    KRModel *pLODModel = *itr++;
                    if((float)pLODModel->getLODCoverage() / 100.0f > lod_coverage && pLODModel->getLODCoverage() < pModel->getLODCoverage()) {
                        pModel = pLODModel;
                    } else {
                        break;
                    }
                }
                
                
                KRShader *pFogShader = m_pContext->getShaderManager()->getShader("volumetric_fog_inside", pCamera, false, false, false, 0, false, false, false, false, false, false, false, false, false, false, false, false, false, KRNode::RENDER_PASS_FORWARD_TRANSPARENT);
                
                if(pFogShader->bind(viewport, pShadowViewports, getModelMatrix(), lightDirection, shadowDepthTextures, cShadowBuffers, KRNode::RENDER_PASS_ADDITIVE_PARTICLES)) {
                    
                    KRAABB viewSpaceBounds = KRAABB(bounds.min, bounds.max, viewport.getViewProjectionMatrix());
                
                    // Enable z-buffer test
                    GLDEBUG(glEnable(GL_DEPTH_TEST));
                    GLDEBUG(glDepthFunc(GL_LEQUAL));
                    GLDEBUG(glDepthRangef(0.0, 1.0));
                    
                    // Enable backface culling
                    GLDEBUG(glCullFace(GL_BACK));
                    GLDEBUG(glEnable(GL_CULL_FACE));
                    
                    
                    int slice_count = 50;
                    
                    float slice_near = -100.0;
                    float slice_far = -1000.0;
                    float slice_spacing = (slice_far - slice_near) / slice_count;
                    
                    KRVector2(slice_near, slice_spacing).setUniform(pFogShader->m_uniforms[KRShader::KRENGINE_UNIFORM_SLICE_DEPTH_SCALE]);
                    
                    m_pContext->getModelManager()->bindVBO((void *)m_pContext->getModelManager()->getVolumetricLightingVertexes(), slice_count * 6 * sizeof(KRModelManager::VolumetricLightingVertexData), true, false, false, false, false);
                    GLDEBUG(glDrawArrays(GL_TRIANGLES, 0, slice_count*6));
                    
                    /*
                    
                    
                    float slice_near = viewSpaceBounds.max.z;
                    float slice_far = viewSpaceBounds.min.z;
                    slice_near = -1.0;
                    slice_far = 1.0;
                    float slice_spacing = (slice_far - slice_near) / slice_count;
                    //                    slice_spacing = 1.0f / slice_count;
//                    slice_near = 0.0f;
                    for(int slice=0; slice < slice_count; slice++) {
                        KRVector2(slice_near + slice * slice_spacing, slice_spacing).setUniform(pFogShader->m_uniforms[KRShader::KRENGINE_UNIFORM_SLICE_DEPTH_SCALE]);
                        int mesh_count = pModel->getSubmeshes().size();
                        for(int iMesh=0; iMesh < mesh_count; iMesh++) {
                            pModel->renderSubmesh(iMesh);
                        }
                    }
                     */
                    
                }
            }
        }
    }
}

#endif

bool KRVolumetricFog::hasTransparency() {
    if(m_models.size() > 0) {
        return m_models[0]->hasTransparency();
    } else {
        return false;
    }
}

KRAABB KRVolumetricFog::getBounds() {
    loadModel();
    if(m_models.size() > 0) {
        return KRAABB(m_models[0]->getMinPoint(), m_models[0]->getMaxPoint(), getModelMatrix());
    } else {
        return KRAABB::Infinite();
    }
}

