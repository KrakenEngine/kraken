//
//  KRMaterial.cpp
//  gldemo
//
//  Created by Kearwood Gilbert on 10-10-24.
//  Copyright (c) 2010 Kearwood Software. All rights reserved.
//

#include "KRMaterial.h"

KRMaterial::KRMaterial() {
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

void KRMaterial::bind(GLuint program) {
    
    GLuint uniform_material_ambient = glGetUniformLocation(program, "material_ambient");
    GLuint uniform_material_diffuse = glGetUniformLocation(program, "material_diffuse");
    GLuint uniform_material_specular = glGetUniformLocation(program, "material_specular");
    
    glUniform3f(
        uniform_material_ambient,
        m_ka_r, // iMaterial % 2 ? (GLfloat)0.9f : (GLfloat)0.5f,
        m_ka_g, // iMaterial % 4 ? (GLfloat)0.9f : (GLfloat)0.5f,
        m_ka_b  // iMaterial % 8 ? (GLfloat)0.9f : (GLfloat)0.5f,
    );
    
    glUniform3f(
        uniform_material_diffuse,
        m_kd_r + 1.0f, // 1.0 added so there will not be complete darkness
        m_kd_g + 1.0f, // 1.0 added so there will not be complete darkness
        m_kd_b + 1.0f  // 1.0 added so there will not be complete darkness
    );
    
    glUniform3f(
        uniform_material_specular,
        m_ks_r, // iMaterial % 2 ? (GLfloat)0.9f : (GLfloat)0.5f,
        m_ks_g, // iMaterial % 4 ? (GLfloat)0.9f : (GLfloat)0.5f,
        m_ks_b  // iMaterial % 8 ? (GLfloat)0.9f : (GLfloat)0.5f,
    );
    
    
    
    int iTextureName = 0;
    if(m_pDiffuseMap != NULL) {
        iTextureName = m_pDiffuseMap->getName();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, iTextureName);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    
    iTextureName = 0;
    if(m_pSpecularMap != NULL) {
        iTextureName = m_pSpecularMap->getName();
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, iTextureName);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    
    iTextureName = 0;
    if(m_pNormalMap != NULL) {
        iTextureName = m_pNormalMap->getName();
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, iTextureName);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }




//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

/*
 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
 */
/*
 if (_anisotropySupported)
 glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, _anisotropyTexParam);
 */
/*
 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
 */


    
    
}