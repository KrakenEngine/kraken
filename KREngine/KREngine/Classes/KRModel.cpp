//
//  KRModel.cpp
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

#import <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>


#include "KRModel.h"

#include "KRVector3.h"
#import "KRShader.h"
#import "KRShaderManager.h"
#import "KRContext.h"

KRModel::KRModel(KRContext &context, std::string name, KRDataBlock *data) : KRContextObject(context) {
    m_name = name;
    m_hasTransparency = false;
    m_materials.clear();
    m_uniqueMaterials.clear();
    m_pMesh = new KRMesh(*m_pContext, name);
    m_pMesh->loadPack(data);
}

std::string KRModel::getName() {
    return m_name;
}

KRModel::~KRModel() {

}
#if TARGET_OS_IPHONE

void KRModel::render(KRCamera *pCamera, KRContext *pContext, KRMat4 &matModelToView, KRMat4 &mvpMatrix, KRVector3 &cameraPosition, KRVector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers, KRTexture *pLightMap, KRNode::RenderPass renderPass) {
    
    //fprintf(stderr, "Rendering model: %s\n", m_name.c_str());
    if(renderPass != KRNode::RENDER_PASS_FLARES) {
    
        if(m_materials.size() == 0) {
            vector<KRMesh::Submesh *> submeshes = m_pMesh->getSubmeshes();
            
            for(std::vector<KRMesh::Submesh *>::iterator itr = submeshes.begin(); itr != submeshes.end(); itr++) {
                const char *szMaterialName = (*itr)->szMaterialName;
                KRMaterial *pMaterial = pContext->getMaterialManager()->getMaterial(szMaterialName);
                m_materials.push_back(pMaterial);
                if(pMaterial) {
                    m_uniqueMaterials.insert(pMaterial);
                } else {
                    fprintf(stderr, "Missing material: %s\n", szMaterialName);
                }
            }
            
            m_hasTransparency = false;
            for(std::set<KRMaterial *>::iterator mat_itr = m_uniqueMaterials.begin(); mat_itr != m_uniqueMaterials.end(); mat_itr++) {
                if((*mat_itr)->isTransparent()) {
                    m_hasTransparency = true;
                    break;
                }
            }
        }
        
        KRMaterial *pPrevBoundMaterial = NULL;
        char szPrevShaderKey[128];
        szPrevShaderKey[0] = '\0';
        int cSubmeshes = m_pMesh->getSubmeshes().size();
        if(renderPass == KRNode::RENDER_PASS_SHADOWMAP) {
            for(int iSubmesh=0; iSubmesh<cSubmeshes; iSubmesh++) {
                KRMaterial *pMaterial = m_materials[iSubmesh];
                
                if(pMaterial != NULL) {

                    if(!pMaterial->isTransparent()) {
                        // Exclude transparent and semi-transparent meshes from shadow maps
                        m_pMesh->renderSubmesh(iSubmesh);
                    }
                }
                
            }
        } else {
            // Apply submeshes in per-material batches to reduce number of state changes
            for(std::set<KRMaterial *>::iterator mat_itr = m_uniqueMaterials.begin(); mat_itr != m_uniqueMaterials.end(); mat_itr++) {
                for(int iSubmesh=0; iSubmesh<cSubmeshes; iSubmesh++) {
                    KRMaterial *pMaterial = m_materials[iSubmesh];
                    
                    if(pMaterial != NULL && pMaterial == (*mat_itr)) {
                        if((!pMaterial->isTransparent() && renderPass != KRNode::RENDER_PASS_FORWARD_TRANSPARENT) || (pMaterial->isTransparent() && renderPass == KRNode::RENDER_PASS_FORWARD_TRANSPARENT)) {
                            if(pMaterial->bind(&pPrevBoundMaterial, szPrevShaderKey, pCamera, matModelToView, mvpMatrix, cameraPosition, lightDirection, pShadowMatrices, shadowDepthTextures, cShadowBuffers, pContext, pLightMap, renderPass)) {
                            
                                switch(pMaterial->getAlphaMode()) {
                                    case KRMaterial::KRMATERIAL_ALPHA_MODE_OPAQUE: // Non-transparent materials
                                    case KRMaterial::KRMATERIAL_ALPHA_MODE_TEST: // Alpha in diffuse texture is interpreted as punch-through when < 0.5
                                        m_pMesh->renderSubmesh(iSubmesh);
                                        break;
                                    case KRMaterial::KRMATERIAL_ALPHA_MODE_BLENDONESIDE: // Blended alpha with backface culling
                                        m_pMesh->renderSubmesh(iSubmesh);
                                        break;
                                    case KRMaterial::KRMATERIAL_ALPHA_MODE_BLENDTWOSIDE: // Blended alpha rendered in two passes.  First pass renders backfaces; second pass renders frontfaces.
                                        // Render back faces first
                                        GLDEBUG(glCullFace(GL_BACK));
                                        m_pMesh->renderSubmesh(iSubmesh);
                                        
                                        // Render front faces second
                                        GLDEBUG(glCullFace(GL_BACK));
                                        m_pMesh->renderSubmesh(iSubmesh);
                                        break;
                                }
                            }
                            
                           
                        }
                    }
                }
            }
        }
    }
}

#endif

KRMesh *KRModel::getMesh() {
    return m_pMesh;
}

bool KRModel::hasTransparency() {
    return m_hasTransparency;
}

