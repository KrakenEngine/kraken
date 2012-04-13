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

#import <stdint.h>
#import <list>
#import <string>

#import "KREngine-common.h"

using std::list;

#import "KRTexture.h"
#import "KRShaderManager.h"
#import "KRShader.h"
#import "KRCamera.h"
#import "KRResource.h"
#import "KRVector2.h"

#ifndef KRMATERIAL_H
#define KRMATERIAL_H



class KRTextureManager;
class KRContext;

class KRMaterial : public KRResource {
public:
    KRMaterial(const char *szName);
    ~KRMaterial();
    
    virtual std::string getExtension();
    virtual bool save(const std::string& path);
    
    
    void setAmbientMap(std::string texture_name, KRVector2 texture_scale, KRVector2 texture_offset);
    void setDiffuseMap(std::string texture_name, KRVector2 texture_scale, KRVector2 texture_offset);
    void setSpecularMap(std::string texture_name, KRVector2 texture_scale, KRVector2 texture_offset);
    void setNormalMap(std::string texture_name, KRVector2 texture_scale, KRVector2 texture_offset);
    void setAmbient(GLfloat r, GLfloat g, GLfloat b);
    void setDiffuse(GLfloat r, GLfloat g, GLfloat b);    
    void setSpecular(GLfloat r, GLfloat g, GLfloat b);
    void setTransparency(GLfloat a);
    void setShininess(GLfloat s);
    

    bool isTransparent();
    char *getName();
    
#if TARGET_OS_IPHONE
    void bind(KRMaterial **prevBoundMaterial, char *szPrevShaderKey, KRCamera *pCamera, KRMat4 &matModelToView, KRMat4 &mvpMatrix, KRVector3 &cameraPosition, KRVector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers, KRContext *pContext, KRTexture *pLightMap, int gBufferPass);
    
#endif
    
private:
    char m_szName[64];
    
    KRTexture *m_pAmbientMap; // mtl map_Ka value
    KRTexture *m_pDiffuseMap; // mtl map_Kd value
    KRTexture *m_pSpecularMap; // mtl map_Ks value
    KRTexture *m_pNormalMap; // mtl map_Normal value
    std::string m_ambientMap;
    std::string m_diffuseMap;
    std::string m_specularMap;
    std::string m_normalMap;
    
    KRVector2 m_ambientMapScale;
    KRVector2 m_ambientMapOffset;
    KRVector2 m_diffuseMapScale;
    KRVector2 m_diffuseMapOffset;
    KRVector2 m_specularMapScale;
    KRVector2 m_specularMapOffset;
    KRVector2 m_normalMapScale;
    KRVector2 m_normalMapOffset;
    
    GLfloat m_ka_r, m_ka_g, m_ka_b; // Ambient rgb
    GLfloat m_kd_r, m_kd_g, m_kd_b; // Diffuse rgb
    GLfloat m_ks_r, m_ks_g, m_ks_b; // Specular rgb
    
    GLfloat m_tr; // Transparency
    GLfloat m_ns; // Shininess
};

#endif
