//
//  KRMaterial.h
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

#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import <stdint.h>
#import <list>
#import <string>

using std::list;

#ifndef KRMATERIAL_H
#define KRMATRIAL_H

#import "KRTexture.h"
#import "KRShaderManager.h"
#import "KRShader.h"
#import "KRCamera.h"

class KRMaterial {
public:
    KRMaterial(char *szName, KRShaderManager *pShaderManager);
    ~KRMaterial();
    
    void setAmbientMap(KRTexture *pTexture);
    void setDiffuseMap(KRTexture *pTexture);
    void setSpecularMap(KRTexture *pTexture);
    void setNormalMap(KRTexture *pTexture);
    void setAmbient(GLfloat r, GLfloat g, GLfloat b);
    void setDiffuse(GLfloat r, GLfloat g, GLfloat b);    
    void setSpecular(GLfloat r, GLfloat g, GLfloat b);
    void setTransparency(GLfloat a);
    void setShininess(GLfloat s);
    
    void bind(KRMaterial **prevBoundMaterial, char *szPrevShaderKey, KRCamera *pCamera, KRMat4 &mvpMatrix, KRVector3 &cameraPosition, KRVector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers);
    bool isTransparent();
    char *getName();
    
private:
    char m_szName[64];
    
    KRTexture *m_pAmbientMap; // mtl map_Ka value
    KRTexture *m_pDiffuseMap; // mtl map_Kd value
    KRTexture *m_pSpecularMap; // mtl map_Ks value
    KRTexture *m_pNormalMap; // mtl map_Normal value
    
    GLfloat m_ka_r, m_ka_g, m_ka_b; // Ambient rgb
    GLfloat m_kd_r, m_kd_g, m_kd_b; // Diffuse rgb
    GLfloat m_ks_r, m_ks_g, m_ks_b; // Specular rgb
    
    GLfloat m_tr; // Transparency
    GLfloat m_ns; // Shininess
    
    KRShaderManager *m_pShaderManager;
};

#endif
