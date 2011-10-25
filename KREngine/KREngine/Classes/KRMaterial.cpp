//
//  KRMaterial.cpp
//  gldemo
//
//  Created by Kearwood Gilbert on 10-10-24.
//  Copyright (c) 2010 Kearwood Software. All rights reserved.
//

#include "KRMaterial.h"

KRMaterial::KRMaterial(KRShaderManager *pShaderManager) {
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

void KRMaterial::bind(KRCamera *pCamera, KRMat4 &mvpMatrix, Vector3 &cameraPosition, Vector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers) {
    
    bool bDiffuseMap = m_pDiffuseMap != NULL && pCamera->bEnableDiffuseMap;
    bool bNormalMap = m_pNormalMap != NULL && pCamera->bEnableNormalMap;
    bool bSpecMap = m_pSpecularMap != NULL && pCamera->bEnableSpecMap;
    
    KRShader *pShader = m_pShaderManager->getShader(pCamera, bDiffuseMap, bNormalMap, bSpecMap, cShadowBuffers);
    pShader->bind(pCamera, mvpMatrix, cameraPosition, lightDirection, pShadowMatrices, shadowDepthTextures, cShadowBuffers);
    
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
    glUniform1f(pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_MATERIAL_SHININESS], pCamera->bDebugSuperShiny ? 20.0 : m_ns);

    int iTextureName = 0;
    if(bDiffuseMap) {
        iTextureName = m_pDiffuseMap->getName();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, iTextureName);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    
    iTextureName = 0;
    if(bSpecMap) {
        iTextureName = m_pSpecularMap->getName();
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, iTextureName);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    
    iTextureName = 0;
    if(bNormalMap) {
        iTextureName = m_pNormalMap->getName();
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, iTextureName);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
}