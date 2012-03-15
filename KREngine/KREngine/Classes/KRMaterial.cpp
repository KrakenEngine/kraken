//
//  KRMaterial.cpp
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

#include "KRMaterial.h"

KRMaterial::KRMaterial(char *szName, KRShaderManager *pShaderManager) {
    strcpy(m_szName, szName);
    m_pAmbientMap = NULL;
    m_pDiffuseMap = NULL;
    m_pSpecularMap = NULL;
    m_pNormalMap = NULL;
    m_ka_r = (GLfloat)0.0f;
    m_ka_g = (GLfloat)0.0f;
    m_ka_b = (GLfloat)0.0f;
    m_kd_r = (GLfloat)1.0f;
    m_kd_g = (GLfloat)1.0f;
    m_kd_b = (GLfloat)1.0f;
    m_ks_r = (GLfloat)1.0f;
    m_ks_g = (GLfloat)1.0f;
    m_ks_b = (GLfloat)1.0f;
    m_tr = (GLfloat)0.0f;
    m_ns = (GLfloat)0.0f;
    
    m_pShaderManager = pShaderManager;
}

KRMaterial::~KRMaterial() {
    
}

void KRMaterial::setAmbientMap(KRTexture *pTexture) {
    m_pAmbientMap = pTexture;
}

void KRMaterial::setDiffuseMap(KRTexture *pTexture) {
    m_pDiffuseMap = pTexture;
}

void KRMaterial::setSpecularMap(KRTexture *pTexture) {
    m_pSpecularMap = pTexture;
}

void KRMaterial::setNormalMap(KRTexture *pTexture) {
    m_pNormalMap = pTexture;
}

void KRMaterial::setAmbient(GLfloat r, GLfloat g, GLfloat b) {
    m_ka_r = r;
    m_ka_g = g;
    m_ka_b = b;
}

void KRMaterial::setDiffuse(GLfloat r, GLfloat g, GLfloat b) {
    m_kd_r = r;
    m_kd_g = g;
    m_kd_b = b;
}

void KRMaterial::setSpecular(GLfloat r, GLfloat g, GLfloat b) {
    m_ks_r = r;
    m_ks_g = g;
    m_ks_b = b;
}

void KRMaterial::setTransparency(GLfloat a) {
    m_tr = a;
}

void KRMaterial::setShininess(GLfloat s) {
    m_ns = s;
}

bool KRMaterial::isTransparent() {
    return m_tr != 0.0;
}

void KRMaterial::bind(KRMaterial **prevBoundMaterial, char *szPrevShaderKey, KRCamera *pCamera, KRMat4 &mvpMatrix, Vector3 &cameraPosition, Vector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers) {
    bool bSameMaterial = *prevBoundMaterial == this;
    
    bool bDiffuseMap = m_pDiffuseMap != NULL && pCamera->bEnableDiffuseMap;
    bool bNormalMap = m_pNormalMap != NULL && pCamera->bEnableNormalMap;
    bool bSpecMap = m_pSpecularMap != NULL && pCamera->bEnableSpecMap;
    
    if(!bSameMaterial) { 
        KRShader *pShader = m_pShaderManager->getShader(pCamera, bDiffuseMap, bNormalMap, bSpecMap, cShadowBuffers);

        bool bSameShader = strcmp(pShader->getKey(), szPrevShaderKey) == 0;
        if(!bSameShader) {
            pShader->bind(pCamera, mvpMatrix, cameraPosition, lightDirection, pShadowMatrices, shadowDepthTextures, cShadowBuffers);
            glUniform1f(pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_MATERIAL_SHININESS], pCamera->bDebugSuperShiny ? 20.0 : m_ns);
            strcpy(szPrevShaderKey, pShader->getKey());
        }
        
        glUniform3f(
            pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_MATERIAL_AMBIENT],
            m_ka_r + pCamera->dAmbientR,
            m_ka_g + pCamera->dAmbientG,
            m_ka_b + pCamera->dAmbientB
        );
        
        glUniform3f(
            pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_MATERIAL_DIFFUSE],
            m_kd_r * pCamera->dSunR,
            m_kd_g * pCamera->dSunG,
            m_kd_b * pCamera->dSunB
        );
        
        glUniform3f(
            pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_MATERIAL_SPECULAR],
            m_ks_r * pCamera->dSunR,
            m_ks_g * pCamera->dSunG,
            m_ks_b * pCamera->dSunB
        );
        
        glUniform1f(pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_MATERIAL_ALPHA], 1.0f - m_tr);


        bool bSameDiffuseMap = false;
        bool bSameSpecMap = false;
        bool bSameNormalMap = false;
        if(*prevBoundMaterial) {
            if((*prevBoundMaterial)->m_pDiffuseMap == m_pDiffuseMap) {
                bSameDiffuseMap = true;
            }
            if((*prevBoundMaterial)->m_pSpecularMap == m_pSpecularMap) {
                bSameSpecMap = true;
            }
            if((*prevBoundMaterial)->m_pNormalMap == m_pNormalMap) {
                bSameNormalMap = true;
            }
        }
        if(bDiffuseMap && !bSameDiffuseMap) {
            int iTextureName = m_pDiffuseMap->getName();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, iTextureName);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }
        
        if(bSpecMap && !bSameSpecMap) {
            int iTextureName = m_pSpecularMap->getName();
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, iTextureName);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }

        if(bNormalMap && !bSameNormalMap) {
            int iTextureName = m_pNormalMap->getName();
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, iTextureName);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }
        
        *prevBoundMaterial = this;
    } // if(!bSameMaterial)
}

char *KRMaterial::getName() {
    return m_szName;
}
