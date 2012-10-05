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

KRMaterial::KRMaterial(KRContext &context, const char *szName) : KRResource(context, szName) {
    strcpy(m_szName, szName);
    m_pAmbientMap = NULL;
    m_pDiffuseMap = NULL;
    m_pSpecularMap = NULL;
    m_pNormalMap = NULL;
    m_pReflectionMap = NULL;
    m_ambientColor = KRVector3::Zero();
    m_diffuseColor = KRVector3::One();
    m_specularColor = KRVector3::One();
    m_reflectionColor = KRVector3::One();
    m_tr = (GLfloat)1.0f;
    m_ns = (GLfloat)0.0f;
    m_ambientMap = "";
    m_diffuseMap = "";
    m_specularMap = "";
    m_normalMap = "";
    m_reflectionMap = "";
    m_ambientMapOffset = KRVector2(0.0f, 0.0f);
    m_specularMapOffset = KRVector2(0.0f, 0.0f);
    m_diffuseMapOffset = KRVector2(0.0f, 0.0f);
    m_ambientMapScale = KRVector2(1.0f, 1.0f);
    m_specularMapScale = KRVector2(1.0f, 1.0f);
    m_diffuseMapScale = KRVector2(1.0f, 1.0f);
    m_reflectionMapOffset = KRVector2(0.0f, 0.0f);
    m_reflectionMapScale = KRVector2(1.0f, 1.0f);
    m_reflectionFactor = 0.0f;
    m_alpha_mode = KRMATERIAL_ALPHA_MODE_OPAQUE;
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
        fprintf(f, "ka %f %f %f\n", m_ambientColor.x, m_ambientColor.y, m_ambientColor.z);
        fprintf(f, "kd %f %f %f\n", m_diffuseColor.x, m_diffuseColor.y, m_diffuseColor.z);
        fprintf(f, "ks %f %f %f\n", m_specularColor.x, m_specularColor.y, m_specularColor.z);
        fprintf(f, "kr %f %f %f\n", m_reflectionColor.x, m_reflectionColor.y, m_reflectionColor.z);
        fprintf(f, "Tr %f\n", m_tr);
        fprintf(f, "Ns %f\n", m_ns);
        fprintf(f, "reflectionFactor %f\n", m_reflectionFactor);
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
        if(m_reflectionMap.size()) {
            fprintf(f, "map_Reflection %s.pvr -s %f %f -o %f %f\n", m_reflectionMap.c_str(), m_reflectionMapScale.x, m_reflectionMapScale.y, m_reflectionMapOffset.x, m_reflectionMapOffset.y);
        }
        switch(m_alpha_mode) {
            case KRMATERIAL_ALPHA_MODE_OPAQUE:
                fprintf(f, "alpha_mode opaque");
                break;
            case KRMATERIAL_ALPHA_MODE_TEST:
                fprintf(f, "alpha_mode test");
                break;
            case KRMATERIAL_ALPHA_MODE_BLENDONESIDE:
                fprintf(f, "alpha_mode blendoneside");
                break;
            case KRMATERIAL_ALPHA_MODE_BLENDTWOSIDE:
                fprintf(f, "alpha_mode blendtwoside");
                break;
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

void KRMaterial::setReflectionMap(std::string texture_name, KRVector2 texture_scale, KRVector2 texture_offset) {
    m_reflectionMap = texture_name;
    m_reflectionMapScale = texture_scale;
    m_reflectionMapOffset = texture_offset;
}

void KRMaterial::setAlphaMode(KRMaterial::alpha_mode_type alpha_mode) {
    m_alpha_mode = alpha_mode;
}

KRMaterial::alpha_mode_type KRMaterial::getAlphaMode() {
    return m_alpha_mode;
}

void KRMaterial::setAmbient(const KRVector3 &c) {
    m_ambientColor = c;
}

void KRMaterial::setDiffuse(const KRVector3 &c) {
    m_diffuseColor = c;
}

void KRMaterial::setSpecular(const KRVector3 &c) {
    m_specularColor = c;
}

void KRMaterial::setReflection(const KRVector3 &c) {
    m_reflectionColor = c;
}

void KRMaterial::setTransparency(GLfloat a) {
    if(a < 1.0f && m_alpha_mode == KRMaterial::KRMATERIAL_ALPHA_MODE_OPAQUE) {
        setAlphaMode(KRMaterial::KRMATERIAL_ALPHA_MODE_BLENDONESIDE);
    }
    m_tr = a;
}

void KRMaterial::setShininess(GLfloat s) {
    m_ns = s;
}

void KRMaterial::setReflectionFactor(GLfloat r) {
    m_reflectionFactor = r;
}

bool KRMaterial::isTransparent() {
    return m_tr < 1.0 || m_alpha_mode == KRMATERIAL_ALPHA_MODE_BLENDONESIDE || m_alpha_mode == KRMATERIAL_ALPHA_MODE_BLENDTWOSIDE;
}

#if TARGET_OS_IPHONE
bool KRMaterial::bind(KRMaterial **prevBoundMaterial, char *szPrevShaderKey, KRCamera *pCamera, KRMat4 &matModelToView, KRMat4 &mvpMatrix, KRVector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers, KRContext *pContext, KRTexture *pLightMap, KRNode::RenderPass renderPass) {
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
    if(!m_pReflectionMap && m_reflectionMap.size()) {
        m_pReflectionMap = pContext->getTextureManager()->getTexture(m_reflectionMap.c_str());
    }
    
    if(!bSameMaterial) { 
        KRVector2 default_scale = KRVector2(1.0f, 1.0f);
        KRVector2 default_offset = KRVector2(0.0f, 0.0f);
        
        bool bDiffuseMap = m_pDiffuseMap != NULL && pCamera->bEnableDiffuseMap;
        bool bNormalMap = m_pNormalMap != NULL && pCamera->bEnableNormalMap;
        bool bSpecMap = m_pSpecularMap != NULL && pCamera->bEnableSpecMap;
        bool bReflectionMap = m_pReflectionMap != NULL && pCamera->bEnableReflectionMap;
        bool bAlphaTest = (m_alpha_mode == KRMATERIAL_ALPHA_MODE_TEST) && bDiffuseMap;
        bool bAlphaBlend = (m_alpha_mode == KRMATERIAL_ALPHA_MODE_BLENDONESIDE) || (m_alpha_mode == KRMATERIAL_ALPHA_MODE_BLENDTWOSIDE);
        
        KRShader *pShader = pContext->getShaderManager()->getShader("ObjectShader", pCamera, bDiffuseMap, bNormalMap, bSpecMap, cShadowBuffers, bLightMap, m_diffuseMapScale != default_scale && bDiffuseMap, m_specularMapScale != default_scale && bSpecMap, m_normalMapScale != default_scale && bNormalMap, m_diffuseMapOffset != default_offset && bDiffuseMap, m_specularMapOffset != default_offset && bSpecMap, m_normalMapOffset != default_offset && bNormalMap, bAlphaTest, bAlphaBlend, renderPass);

        bool bSameShader = strcmp(pShader->getKey(), szPrevShaderKey) == 0;
        if(!bSameShader) {
            if(!pShader->bind(pCamera, matModelToView, mvpMatrix, lightDirection, pShadowMatrices, shadowDepthTextures, cShadowBuffers, renderPass)) {
                return false;
            }
            
            strcpy(szPrevShaderKey, pShader->getKey());
        }
        GLDEBUG(glUniform1f(pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_MATERIAL_SHININESS], pCamera->bDebugSuperShiny ? 20.0 : m_ns ));
        
        bool bSameAmbient = false;
        bool bSameDiffuse = false;
        bool bSameSpecular = false;
        bool bSameReflection = false;
        bool bSameAmbientScale = false;
        bool bSameDiffuseScale = false;
        bool bSameSpecularScale = false;
        bool bSameReflectionScale = false;
        bool bSameNormalScale = false;
        bool bSameAmbientOffset = false;
        bool bSameDiffuseOffset = false;
        bool bSameSpecularOffset = false;
        bool bSameReflectionOffset = false;
        bool bSameNormalOffset = false;
        
        if(*prevBoundMaterial && bSameShader) {
            bSameAmbient = (*prevBoundMaterial)->m_ambientColor == m_ambientColor;
            bSameDiffuse = (*prevBoundMaterial)->m_diffuseColor == m_diffuseColor;
            bSameSpecular = (*prevBoundMaterial)->m_specularColor == m_specularColor;
            bSameReflection = (*prevBoundMaterial)->m_reflectionColor == m_reflectionColor;
            bSameAmbientScale = (*prevBoundMaterial)->m_ambientMapScale == m_ambientMapScale;
            bSameDiffuseScale = (*prevBoundMaterial)->m_diffuseMapScale == m_diffuseMapScale;
            bSameSpecularScale = (*prevBoundMaterial)->m_specularMapScale == m_specularMapScale;
            bSameReflectionScale = (*prevBoundMaterial)->m_reflectionMapScale == m_reflectionMapScale;
            bSameNormalScale = (*prevBoundMaterial)->m_normalMapScale == m_normalMapScale;
            bSameAmbientOffset = (*prevBoundMaterial)->m_ambientMapOffset == m_ambientMapOffset;
            bSameDiffuseOffset = (*prevBoundMaterial)->m_diffuseMapOffset == m_diffuseMapOffset;
            bSameSpecularOffset = (*prevBoundMaterial)->m_specularMapOffset == m_specularMapOffset;
            bSameReflectionOffset = (*prevBoundMaterial)->m_reflectionMapOffset == m_reflectionMapOffset;
            bSameNormalOffset = (*prevBoundMaterial)->m_normalMapOffset == m_normalMapOffset;
        }
        
        if(!bSameAmbient) {
            GLDEBUG(glUniform3f(
                pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_MATERIAL_AMBIENT],
                m_ambientColor.x + pCamera->dAmbientR,
                m_ambientColor.y + pCamera->dAmbientG,
                m_ambientColor.z + pCamera->dAmbientB
            ));
        }
        
        if(!bSameDiffuse) {
            if(renderPass == KRNode::RENDER_PASS_FORWARD_OPAQUE) {
                // We pre-multiply the light color with the material color in the forward renderer
                GLDEBUG(glUniform3f(
                    pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_MATERIAL_DIFFUSE],
                    m_diffuseColor.x * pCamera->dSunR,
                    m_diffuseColor.y * pCamera->dSunG,
                    m_diffuseColor.z * pCamera->dSunB
                ));
            } else {
                GLDEBUG(glUniform3f(
                    pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_MATERIAL_DIFFUSE],
                    m_diffuseColor.x,
                    m_diffuseColor.y,
                    m_diffuseColor.z
                ));
            }
        }
        
        if(!bSameSpecular) {
            if(renderPass == KRNode::RENDER_PASS_FORWARD_OPAQUE) {
                // We pre-multiply the light color with the material color in the forward renderer
                GLDEBUG(glUniform3f(
                    pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_MATERIAL_SPECULAR],
                    m_specularColor.x * pCamera->dSunR,
                    m_specularColor.y * pCamera->dSunG,
                    m_specularColor.z * pCamera->dSunB
                ));
            } else {
                GLDEBUG(glUniform3f(
                    pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_MATERIAL_SPECULAR],
                    m_specularColor.x,
                    m_specularColor.y,
                    m_specularColor.z
                ));
            }
        }
        
        if(!bSameReflection) {
            GLDEBUG(glUniform3f(
                pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_MATERIAL_REFLECTION],
                m_reflectionColor.x,
                m_reflectionColor.y,
                m_reflectionColor.z
                ));
        }
        
        if(bDiffuseMap && !bSameDiffuseScale && m_diffuseMapScale != default_scale) {
            GLDEBUG(glUniform2f(
                pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_DIFFUSETEXTURE_SCALE],
                m_diffuseMapScale.x,
                m_diffuseMapScale.y
            ));
        }
        
        if(bSpecMap && !bSameSpecularScale && m_specularMapScale != default_scale) {
            GLDEBUG(glUniform2f(
                pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_SPECULARTEXTURE_SCALE],
                m_specularMapScale.x,
                m_specularMapScale.y
            ));
        }
        
        if(bReflectionMap && !bSameReflectionScale && m_reflectionMapScale != default_scale) {
            GLDEBUG(glUniform2f(
                pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_REFLECTIONTEXTURE_SCALE],
                m_reflectionMapScale.x,
                m_reflectionMapScale.y
            ));
        }
        
        if(bNormalMap && !bSameNormalScale && m_normalMapScale != default_scale) {
            GLDEBUG(glUniform2f(
                pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_NORMALTEXTURE_SCALE],
                m_normalMapScale.x,
                m_normalMapScale.y
            ));
        }
        
        if(bDiffuseMap && !bSameDiffuseOffset && m_diffuseMapOffset != default_offset) {
            GLDEBUG(glUniform2f(
                pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_DIFFUSETEXTURE_OFFSET],
                m_diffuseMapOffset.x,
                m_diffuseMapOffset.y
            ));
        }
        
        if(bSpecMap && !bSameSpecularOffset && m_specularMapOffset != default_offset) {
            GLDEBUG(glUniform2f(
                pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_SPECULARTEXTURE_OFFSET],
                m_specularMapOffset.x,
                m_specularMapOffset.y
            ));
        }
        
        if(bReflectionMap && !bSameReflectionOffset && m_reflectionMapOffset != default_offset) {
            GLDEBUG(glUniform2f(
                pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_REFLECTIONTEXTURE_OFFSET],
                m_reflectionMapOffset.x,
                m_reflectionMapOffset.y
            ));
        }
        
        if(bNormalMap && !bSameNormalOffset && m_normalMapOffset != default_offset) {
            GLDEBUG(glUniform2f(
                pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_NORMALTEXTURE_OFFSET],
                m_normalMapOffset.x,
                m_normalMapOffset.y
            ));
        }
        
        GLDEBUG(glUniform1f(pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_MATERIAL_ALPHA], m_tr));
        GLDEBUG(glUniform1f(pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_MATERIAL_REFLECTIVITY], m_reflectionFactor));
        
        if(bDiffuseMap) {
            m_pContext->getTextureManager()->selectTexture(0, m_pDiffuseMap, 2048);
        }
        
        if(bSpecMap) {
            m_pContext->getTextureManager()->selectTexture(1, m_pSpecularMap, 2048);
        }

        if(bNormalMap) {
            m_pContext->getTextureManager()->selectTexture(2, m_pNormalMap, 2048);
        }
        
        if(bReflectionMap && (renderPass == KRNode::RENDER_PASS_FORWARD_OPAQUE || renderPass == KRNode::RENDER_PASS_DEFERRED_OPAQUE)) {
            // GL_TEXTURE7 is used for reading the depth buffer in gBuffer pass 2 and re-used for the reflection map in gBuffer Pass 3 and in forward rendering
            m_pContext->getTextureManager()->selectTexture(7, m_pReflectionMap, 2048);
        }
        
        *prevBoundMaterial = this;
    } // if(!bSameMaterial)
    
    return true;
}

#endif

char *KRMaterial::getName() {
    return m_szName;
}
