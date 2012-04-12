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
#include "KRTextureManager.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#import "KRcontext.h"

KRMaterial::KRMaterial(const char *szName) : KRResource(szName) {
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
    m_ambientMap = "";
    m_diffuseMap = "";
    m_specularMap = "";
    m_normalMap = "";
    m_ambientMapOffset = KRVector2(0.0f, 0.0f);
    m_specularMapOffset = KRVector2(0.0f, 0.0f);
    m_diffuseMapOffset = KRVector2(0.0f, 0.0f);
    m_ambientMapScale = KRVector2(1.0f, 1.0f);
    m_specularMapScale = KRVector2(1.0f, 1.0f);
    m_diffuseMapScale = KRVector2(1.0f, 1.0f);
}

KRMaterial::~KRMaterial() {
    
}

std::string KRMaterial::getExtension() {
    return "mtl";
}
bool KRMaterial::save(const std::string& path) {
    FILE *f = fopen(path.c_str(), "w+");
    if(f == NULL) {
        return false;
    } else {

        fprintf(f, "newmtl %s\n", m_szName);
        fprintf(f, "ka %f %f %f\n", m_ka_r, m_ka_g, m_ka_b);
        fprintf(f, "kd %f %f %f\n", m_kd_r, m_kd_g, m_kd_b);
        fprintf(f, "ks %f %f %f\n", m_ks_r, m_ks_g, m_ks_b);
        fprintf(f, "Tr %f\n", m_tr);
        fprintf(f, "Ns %f\n", m_ns);
        if(m_ambientMap.size()) {
            fprintf(f, "map_Ka %s.pvr -s %f %f -o %f %f\n", m_ambientMap.c_str(), m_ambientMapScale.x, m_ambientMapScale.y, m_ambientMapOffset.x, m_ambientMapOffset.y);
        }
        if(m_diffuseMap.size()) {
            fprintf(f, "map_Kd %s.pvr -s %f %f -o %f %f\n", m_diffuseMap.c_str(), m_diffuseMapScale.x, m_diffuseMapScale.y, m_diffuseMapOffset.x, m_diffuseMapOffset.y);
        }
        if(m_specularMap.size()) {
            fprintf(f, "map_Ks %s.pvr -s %f %f -o %f %f\n", m_specularMap.c_str(), m_specularMapScale.x, m_specularMapScale.y, m_specularMapOffset.x, m_specularMapOffset.y);
        }
        if(m_normalMap.size()) {
            fprintf(f, "map_Normal %s.pvr -s %f %f -o %f %f\n", m_normalMap.c_str(), m_normalMapScale.x, m_normalMapScale.y, m_normalMapOffset.x, m_normalMapOffset.y);
        }
        fclose(f);
        return true;
    }
}

void KRMaterial::setAmbientMap(std::string texture_name, KRVector2 texture_scale, KRVector2 texture_offset) {
    m_ambientMap = texture_name;
    m_ambientMapScale = texture_scale;
    m_ambientMapOffset = texture_offset;
}

void KRMaterial::setDiffuseMap(std::string texture_name, KRVector2 texture_scale, KRVector2 texture_offset) {
    m_diffuseMap = texture_name;
    m_diffuseMapScale = texture_scale;
    m_diffuseMapOffset = texture_offset;
}

void KRMaterial::setSpecularMap(std::string texture_name, KRVector2 texture_scale, KRVector2 texture_offset) {
    m_specularMap = texture_name;
    m_specularMapScale = texture_scale;
    m_specularMapOffset = texture_offset;
}

void KRMaterial::setNormalMap(std::string texture_name, KRVector2 texture_scale, KRVector2 texture_offset) {
    m_normalMap = texture_name;
    m_normalMapScale = texture_scale;
    m_normalMapOffset = texture_offset;
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
#if TARGET_OS_IPHONE
void KRMaterial::bind(KRMaterial **prevBoundMaterial, char *szPrevShaderKey, KRCamera *pCamera, KRMat4 &mvpMatrix, KRVector3 &cameraPosition, KRVector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers, KRContext *pContext, KRTexture *pLightMap) {
    bool bSameMaterial = *prevBoundMaterial == this;
    bool bLightMap = pLightMap && pCamera->bEnableLightMap;
    
    if(!m_pAmbientMap && m_ambientMap.size()) {
        m_pAmbientMap = pContext->getTextureManager()->getTexture(m_ambientMap.c_str());
    }
    if(!m_pDiffuseMap && m_diffuseMap.size()) {
        m_pDiffuseMap = pContext->getTextureManager()->getTexture(m_diffuseMap.c_str());
    }
    if(!m_pNormalMap && m_normalMap.size()) {
        m_pNormalMap = pContext->getTextureManager()->getTexture(m_normalMap.c_str());
    }
    if(!m_pSpecularMap && m_specularMap.size()) {
        m_pSpecularMap = pContext->getTextureManager()->getTexture(m_specularMap.c_str());
    }
    
    

    
    
    if(!bSameMaterial) { 
        KRVector2 default_scale = KRVector2(1.0f, 1.0f);
        KRVector2 default_offset = KRVector2(0.0f, 0.0f);
        
        bool bDiffuseMap = m_pDiffuseMap != NULL && pCamera->bEnableDiffuseMap;
        bool bNormalMap = m_pNormalMap != NULL && pCamera->bEnableNormalMap;
        bool bSpecMap = m_pSpecularMap != NULL && pCamera->bEnableSpecMap;
        
        KRShader *pShader = pContext->getShaderManager()->getShader(pCamera, bDiffuseMap, bNormalMap, bSpecMap, cShadowBuffers, bLightMap, m_diffuseMapScale != default_scale && bDiffuseMap, m_specularMapScale != default_scale && bSpecMap, m_normalMapScale != default_scale && bNormalMap, m_diffuseMapOffset != default_offset && bDiffuseMap, m_specularMapOffset != default_offset && bSpecMap, m_normalMapOffset != default_offset && bNormalMap);

        bool bSameShader = strcmp(pShader->getKey(), szPrevShaderKey) == 0;
        if(!bSameShader) {
            pShader->bind(pCamera, mvpMatrix, cameraPosition, lightDirection, pShadowMatrices, shadowDepthTextures, cShadowBuffers);
            glUniform1f(pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_MATERIAL_SHININESS], pCamera->bDebugSuperShiny ? 20.0 : m_ns );
            strcpy(szPrevShaderKey, pShader->getKey());
        }
        
        bool bSameAmbient = false;
        bool bSameDiffuse = false;
        bool bSameSpecular = false;
        bool bSameAmbientScale = false;
        bool bSameDiffuseScale = false;
        bool bSameSpecularScale = false;
        bool bSameNormalScale = false;
        bool bSameAmbientOffset = false;
        bool bSameDiffuseOffset = false;
        bool bSameSpecularOffset = false;
        bool bSameNormalOffset = false;
        
        if(*prevBoundMaterial && bSameShader) {
            bSameAmbient = (*prevBoundMaterial)->m_ka_r == m_ka_r && (*prevBoundMaterial)->m_ka_g == m_ka_g && (*prevBoundMaterial)->m_ka_b == m_ka_b;
            bSameDiffuse = (*prevBoundMaterial)->m_kd_r == m_kd_r && (*prevBoundMaterial)->m_kd_g == m_kd_g && (*prevBoundMaterial)->m_kd_b == m_kd_b;
            bSameSpecular = (*prevBoundMaterial)->m_ks_r == m_ks_r && (*prevBoundMaterial)->m_ks_g == m_ks_g && (*prevBoundMaterial)->m_ks_b == m_ks_b;
            bSameAmbientScale = (*prevBoundMaterial)->m_ambientMapScale == m_ambientMapScale;
            bSameDiffuseScale = (*prevBoundMaterial)->m_diffuseMapScale == m_diffuseMapScale;
            bSameSpecularScale = (*prevBoundMaterial)->m_specularMapScale == m_specularMapScale;
            bSameNormalScale = (*prevBoundMaterial)->m_normalMapScale == m_normalMapScale;
            bSameAmbientOffset = (*prevBoundMaterial)->m_ambientMapOffset == m_ambientMapOffset;
            bSameDiffuseOffset = (*prevBoundMaterial)->m_diffuseMapOffset == m_diffuseMapOffset;
            bSameSpecularOffset = (*prevBoundMaterial)->m_specularMapOffset == m_specularMapOffset;
            bSameNormalOffset = (*prevBoundMaterial)->m_normalMapOffset == m_normalMapOffset;
        }
        
        if(!bSameAmbient) {
            glUniform3f(
                pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_MATERIAL_AMBIENT],
                m_ka_r + pCamera->dAmbientR,
                m_ka_g + pCamera->dAmbientG,
                m_ka_b + pCamera->dAmbientB
            );
        }
        
        if(!bSameDiffuse) {
            glUniform3f(
                pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_MATERIAL_DIFFUSE],
                m_kd_r * pCamera->dSunR,
                m_kd_g * pCamera->dSunG,
                m_kd_b * pCamera->dSunB
            );
        }
        
        if(!bSameSpecular) {
            glUniform3f(
                pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_MATERIAL_SPECULAR],
                m_ks_r * pCamera->dSunR,
                m_ks_g * pCamera->dSunG,
                m_ks_b * pCamera->dSunB
            );
        }
        
        if(bDiffuseMap && !bSameDiffuseScale && m_diffuseMapScale != default_scale) {
            glUniform2f(
                pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_DIFFUSETEXTURE_SCALE],
                m_diffuseMapScale.x,
                m_diffuseMapScale.y
            );
        }
        
        if(bSpecMap && !bSameSpecularScale && m_specularMapScale != default_scale) {
            glUniform2f(
                pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_SPECULARTEXTURE_SCALE],
                m_specularMapScale.x,
                m_specularMapScale.y
            );
        }
        
        if(bNormalMap && !bSameNormalScale && m_normalMapScale != default_scale) {
            glUniform2f(
                pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_NORMALTEXTURE_SCALE],
                m_normalMapScale.x,
                m_normalMapScale.y
            );
        }
        
        if(bDiffuseMap && !bSameDiffuseOffset && m_diffuseMapOffset != default_offset) {
            glUniform2f(
                pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_DIFFUSETEXTURE_OFFSET],
                m_diffuseMapOffset.x,
                m_diffuseMapOffset.y
            );
        }
        
        if(bSpecMap && !bSameSpecularOffset && m_specularMapOffset != default_offset) {
            glUniform2f(
                pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_SPECULARTEXTURE_OFFSET],
                m_specularMapOffset.x,
                m_specularMapOffset.y
            );
        }
        
        if(bNormalMap && !bSameNormalOffset && m_normalMapOffset != default_offset) {
            glUniform2f(
                pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_NORMALTEXTURE_OFFSET],
                m_normalMapOffset.x,
                m_normalMapOffset.y
            );
        }
        
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
#endif

char *KRMaterial::getName() {
    return m_szName;
}
