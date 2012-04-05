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


KRModel::KRModel(std::string path, KRMaterialManager *pMaterialManager) {
    loadPack(path, pMaterialManager);
}

void KRModel::loadPack(std::string path, KRMaterialManager *pMaterialManager) {
    m_pMesh = new KRMesh(KRResource::GetFileBase(path));
    m_pMesh->loadPack(path);
    
    vector<KRMesh::Submesh *> submeshes = m_pMesh->getSubmeshes();
    
    for(std::vector<KRMesh::Submesh *>::iterator itr = submeshes.begin(); itr != submeshes.end(); itr++) {
        KRMaterial *pMaterial = pMaterialManager->getMaterial((*itr)->szMaterialName);
        m_materials.push_back(pMaterial);
        m_uniqueMaterials.insert(pMaterial);
    }
}

KRModel::~KRModel() {

}

void KRModel::render(KRCamera *pCamera, KRMaterialManager *pMaterialManager, bool bRenderShadowMap, KRMat4 &mvpMatrix, KRVector3 &cameraPosition, KRVector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers, KRShaderManager *pShaderManager, KRTextureManager *pTextureManager, KRTexture *pShadowMap) {
    KRMaterial *pPrevBoundMaterial = NULL;
    int iPrevBuffer = -1;
    char szPrevShaderKey[128];
    szPrevShaderKey[0] = '\0';
    int cSubmeshes = m_pMesh->getSubmeshes().size();
    if(bRenderShadowMap) {
        for(int iSubmesh=0; iSubmesh<cSubmeshes; iSubmesh++) {
            KRMaterial *pMaterial = m_materials[iSubmesh];
            
            if(pMaterial != NULL) {

                if(!pMaterial->isTransparent()) {
                    // Exclude transparent and semi-transparent meshes from shadow maps
                    m_pMesh->renderSubmesh(iSubmesh, &iPrevBuffer);
                }
            }
            
        }
    } else {
        // Apply submeshes in per-material batches to reduce number of state changes
        for(std::set<KRMaterial *>::iterator mat_itr = m_uniqueMaterials.begin(); mat_itr != m_uniqueMaterials.end(); mat_itr++) {
            for(int iSubmesh=0; iSubmesh<cSubmeshes; iSubmesh++) {
                KRMaterial *pMaterial = m_materials[iSubmesh];
                
                if(pMaterial != NULL && pMaterial == (*mat_itr)) {
                    pMaterial->bind(&pPrevBoundMaterial, szPrevShaderKey, pCamera, mvpMatrix, cameraPosition, lightDirection, pShadowMatrices, shadowDepthTextures, cShadowBuffers, pShaderManager, pTextureManager, pShadowMap);
                    m_pMesh->renderSubmesh(iSubmesh, &iPrevBuffer);
                }
            }
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

KRMesh *KRModel::getMesh() {
    return m_pMesh;
}

