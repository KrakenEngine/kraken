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

#include "KREngine-common.h"

#include "KRMaterial.h"
#include "KRTextureManager.h"

#include "KRcontext.h"

KRMaterial::KRMaterial(KRContext &context, const char *szName) : KRResource(context, szName) {
    m_name = szName;
    m_pAmbientMap = NULL;
    m_pDiffuseMap = NULL;
    m_pSpecularMap = NULL;
    m_pNormalMap = NULL;
    m_pReflectionMap = NULL;
    m_pReflectionCube = NULL;
    m_ambientColor = KRVector3::Zero();
    m_diffuseColor = KRVector3::One();
    m_specularColor = KRVector3::One();
    m_reflectionColor = KRVector3::Zero();
    m_tr = (GLfloat)1.0f;
    m_ns = (GLfloat)0.0f;
    m_ambientMap = "";
    m_diffuseMap = "";
    m_specularMap = "";
    m_normalMap = "";
    m_reflectionMap = "";
    m_reflectionCube = "";
    m_ambientMapOffset = KRVector2(0.0f, 0.0f);
    m_specularMapOffset = KRVector2(0.0f, 0.0f);
    m_diffuseMapOffset = KRVector2(0.0f, 0.0f);
    m_ambientMapScale = KRVector2(1.0f, 1.0f);
    m_specularMapScale = KRVector2(1.0f, 1.0f);
    m_diffuseMapScale = KRVector2(1.0f, 1.0f);
    m_reflectionMapOffset = KRVector2(0.0f, 0.0f);
    m_reflectionMapScale = KRVector2(1.0f, 1.0f);
    m_alpha_mode = KRMATERIAL_ALPHA_MODE_OPAQUE;
}

KRMaterial::~KRMaterial() {
    
}

std::string KRMaterial::getExtension() {
    return "mtl";
}

bool KRMaterial::save(KRDataBlock &data) {
    std::stringstream stream;
    stream.precision(std::numeric_limits<long double>::digits10);
    stream.setf(std::ios::fixed,std::ios::floatfield);
    
    stream << "newmtl " << m_name;
    stream << "\nka " << m_ambientColor.x << " " << m_ambientColor.y << " " << m_ambientColor.z;
    stream << "\nkd " << m_diffuseColor.x << " " << m_diffuseColor.y << " " << m_diffuseColor.z;
    stream << "\nks " << m_specularColor.x << " " << m_specularColor.y << " " << m_specularColor.z;
    stream << "\nkr " << m_reflectionColor.x << " " << m_reflectionColor.y << " " << m_reflectionColor.z;
    stream << "\nTr " << m_tr;
    stream << "\nNs " << m_ns;
    if(m_ambientMap.size()) {
        stream << "\nmap_Ka " << m_ambientMap << ".pvr -s " << m_ambientMapScale.x << " " << m_ambientMapScale.y << " -o " << m_ambientMapOffset.x << " " << m_ambientMapOffset.y;
    } else {
        stream << "\n# map_Ka filename.pvr -s 1.0 1.0 -o 0.0 0.0";
    }
    if(m_diffuseMap.size()) {
        stream << "\nmap_Kd " << m_diffuseMap << ".pvr -s " << m_diffuseMapScale.x << " " << m_diffuseMapScale.y << " -o " << m_diffuseMapOffset.x << " " << m_diffuseMapOffset.y;
    } else {
        stream << "\n# map_Kd filename.pvr -s 1.0 1.0 -o 0.0 0.0";
    }
    if(m_specularMap.size()) {
        stream << "\nmap_Ks " << m_specularMap << ".pvr -s " << m_specularMapScale.x << " " << m_specularMapScale.y << " -o " << m_specularMapOffset.x << " " << m_specularMapOffset.y << "\n";
    } else {
        stream << "\n# map_Ks filename.pvr -s 1.0 1.0 -o 0.0 0.0";
    }
    if(m_normalMap.size()) {
        stream << "\nmap_Normal " << m_normalMap << ".pvr -s " << m_normalMapScale.x << " " << m_normalMapScale.y << " -o " << m_normalMapOffset.x << " " << m_normalMapOffset.y;
    } else {
        stream << "\n# map_Normal filename.pvr -s 1.0 1.0 -o 0.0 0.0";
    }
    if(m_reflectionMap.size()) {
        stream << "\nmap_Reflection " << m_reflectionMap << ".pvr -s " << m_reflectionMapScale.x << " " << m_reflectionMapScale.y << " -o " << m_reflectionMapOffset.x << " " << m_reflectionMapOffset.y;
    } else {
        stream << "\n# map_Reflection filename.pvr -s 1.0 1.0 -o 0.0 0.0";
    }
    if(m_reflectionCube.size()) {
        stream << "map_ReflectionCube " << m_reflectionCube << ".pvr";
    } else {
        stream << "\n# map_ReflectionCube cubemapname";
    }
    switch(m_alpha_mode) {
        case KRMATERIAL_ALPHA_MODE_OPAQUE:
            stream << "\nalpha_mode opaque";
            break;
        case KRMATERIAL_ALPHA_MODE_TEST:
            stream << "\nalpha_mode test";
            break;
        case KRMATERIAL_ALPHA_MODE_BLENDONESIDE:
            stream << "\nalpha_mode blendoneside";
            break;
        case KRMATERIAL_ALPHA_MODE_BLENDTWOSIDE:
            stream << "\nalpha_mode blendtwoside";
            break;
    }
    stream << "\n# alpha_mode opaque, test, blendoneside, or blendtwoside";
    
    stream << "\n";
    data.append(stream.str());

    return true;
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

void KRMaterial::setReflectionCube(std::string texture_name) {
    m_reflectionCube = texture_name;
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

bool KRMaterial::isTransparent() {
    return m_tr < 1.0 || m_alpha_mode == KRMATERIAL_ALPHA_MODE_BLENDONESIDE || m_alpha_mode == KRMATERIAL_ALPHA_MODE_BLENDTWOSIDE;
}

bool KRMaterial::bind(KRMaterial **prevBoundMaterial, char *szPrevShaderKey, KRCamera *pCamera, std::vector<KRLight *> &lights, const std::vector<KRBone *> &bones, const KRViewport &viewport, const KRMat4 &matModel, KRTexture *pLightMap, KRNode::RenderPass renderPass) {
    bool bSameMaterial = *prevBoundMaterial == this;
    bool bLightMap = pLightMap && pCamera->settings.bEnableLightMap;
    
    if(!m_pAmbientMap && m_ambientMap.size()) {
        m_pAmbientMap = getContext().getTextureManager()->getTexture(m_ambientMap);
    }
    if(!m_pDiffuseMap && m_diffuseMap.size()) {
        m_pDiffuseMap = getContext().getTextureManager()->getTexture(m_diffuseMap);
    }
    if(!m_pNormalMap && m_normalMap.size()) {
        m_pNormalMap = getContext().getTextureManager()->getTexture(m_normalMap);
    }
    if(!m_pSpecularMap && m_specularMap.size()) {
        m_pSpecularMap = getContext().getTextureManager()->getTexture(m_specularMap);
    }
    if(!m_pReflectionMap && m_reflectionMap.size()) {
        m_pReflectionMap = getContext().getTextureManager()->getTexture(m_reflectionMap);
    }
    if(!m_pReflectionCube && m_reflectionCube.size()) {
        m_pReflectionCube = getContext().getTextureManager()->getTextureCube(m_reflectionCube.c_str());
    }
    
    if(!bSameMaterial) { 
        KRVector2 default_scale = KRVector2(1.0f, 1.0f);
        KRVector2 default_offset = KRVector2(0.0f, 0.0f);
        
        bool bHasReflection = m_reflectionColor != KRVector3(0.0f, 0.0f, 0.0f);
        bool bDiffuseMap = m_pDiffuseMap != NULL && pCamera->settings.bEnableDiffuseMap;
        bool bNormalMap = m_pNormalMap != NULL && pCamera->settings.bEnableNormalMap;
        bool bSpecMap = m_pSpecularMap != NULL && pCamera->settings.bEnableSpecMap;
        bool bReflectionMap = m_pReflectionMap != NULL && pCamera->settings.bEnableReflectionMap && pCamera->settings.bEnableReflection && bHasReflection;
        bool bReflectionCubeMap = m_pReflectionCube != NULL && pCamera->settings.bEnableReflection && bHasReflection;
        bool bAlphaTest = (m_alpha_mode == KRMATERIAL_ALPHA_MODE_TEST) && bDiffuseMap;
        bool bAlphaBlend = (m_alpha_mode == KRMATERIAL_ALPHA_MODE_BLENDONESIDE) || (m_alpha_mode == KRMATERIAL_ALPHA_MODE_BLENDTWOSIDE);
        
        
        KRShader *pShader = getContext().getShaderManager()->getShader("ObjectShader", pCamera, lights, bones.size(), bDiffuseMap, bNormalMap, bSpecMap, bReflectionMap, bReflectionCubeMap, bLightMap, m_diffuseMapScale != default_scale && bDiffuseMap, m_specularMapScale != default_scale && bSpecMap, m_reflectionMapScale != default_scale && bReflectionMap, m_normalMapScale != default_scale && bNormalMap, m_diffuseMapOffset != default_offset && bDiffuseMap, m_specularMapOffset != default_offset && bSpecMap, m_reflectionMapOffset != default_offset && bReflectionMap, m_normalMapOffset != default_offset && bNormalMap, bAlphaTest, bAlphaBlend, renderPass);

        bool bSameShader = strcmp(pShader->getKey(), szPrevShaderKey) == 0;
        if(!bSameShader) {
            if(!getContext().getShaderManager()->selectShader(*pCamera, pShader, viewport, matModel, lights, 0, renderPass)) {
                return false;
            }
            
            strcpy(szPrevShaderKey, pShader->getKey());
        }
        GLDEBUG(glUniform1f(pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_MATERIAL_SHININESS], pCamera->settings.bDebugSuperShiny ? 20.0 : m_ns ));
        
        // Bind bones
        if(pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_BONE_TRANSFORMS] != -1) {
            GLfloat bone_mats[256 * 16];
            GLfloat *bone_mat_component = bone_mats;
            for(int bone_index=0; bone_index < bones.size(); bone_index++) {
                KRBone *bone = bones[bone_index];
                
//                KRVector3 initialRotation = bone->getInitialLocalRotation();
//                KRVector3 rotation = bone->getLocalRotation();
//                KRVector3 initialTranslation = bone->getInitialLocalTranslation();
//                KRVector3 translation = bone->getLocalTranslation();
//                KRVector3 initialScale = bone->getInitialLocalScale();
//                KRVector3 scale = bone->getLocalScale();
//                
                //printf("%s - delta rotation: %.4f %.4f %.4f\n", bone->getName().c_str(), (rotation.x - initialRotation.x) * 180.0 / M_PI, (rotation.y - initialRotation.y) * 180.0 / M_PI, (rotation.z - initialRotation.z) * 180.0 / M_PI);
                //printf("%s - delta translation: %.4f %.4f %.4f\n", bone->getName().c_str(), translation.x - initialTranslation.x, translation.y - initialTranslation.y, translation.z - initialTranslation.z);
//                printf("%s - delta scale: %.4f %.4f %.4f\n", bone->getName().c_str(), scale.x - initialScale.x, scale.y - initialScale.y, scale.z - initialScale.z);
                
                KRMat4 t = bone->getInverseBindPoseMatrix() * bone->getModelMatrix();
                for(int i=0; i < 16; i++) {
                    *bone_mat_component++ = t[i];
                }
            }
            glUniformMatrix4fv(pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_BONE_TRANSFORMS], bones.size(), GL_FALSE, bone_mats);
        }
        
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
            (m_ambientColor + pCamera->settings.ambient_intensity).setUniform(pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_MATERIAL_AMBIENT]);
        }
        
        if(!bSameDiffuse) {
            if(renderPass == KRNode::RENDER_PASS_FORWARD_OPAQUE) {
                // We pre-multiply the light color with the material color in the forward renderer
                KRVector3(m_diffuseColor.x * pCamera->settings.light_intensity.x, m_diffuseColor.y * pCamera->settings.light_intensity.y, m_diffuseColor.z * pCamera->settings.light_intensity.z).setUniform(pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_MATERIAL_DIFFUSE]);
            } else {
                m_diffuseColor.setUniform(pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_MATERIAL_DIFFUSE]);
            }
        }
        
        if(!bSameSpecular) {
            if(renderPass == KRNode::RENDER_PASS_FORWARD_OPAQUE) {
                // We pre-multiply the light color with the material color in the forward renderer
                KRVector3(m_specularColor.x * pCamera->settings.light_intensity.x, m_specularColor.y * pCamera->settings.light_intensity.y, m_specularColor.z * pCamera->settings.light_intensity.z).setUniform(pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_MATERIAL_SPECULAR]);
            } else {
                m_specularColor.setUniform(pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_MATERIAL_SPECULAR]);
            }
        }
        
        if(!bSameReflection) {
            m_reflectionColor.setUniform(pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_MATERIAL_REFLECTION]);
        }
        
        if(bDiffuseMap && !bSameDiffuseScale && m_diffuseMapScale != default_scale) {
            m_diffuseMapScale.setUniform(pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_DIFFUSETEXTURE_SCALE]);
        }
        
        if(bSpecMap && !bSameSpecularScale && m_specularMapScale != default_scale) {
            m_specularMapScale.setUniform(pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_SPECULARTEXTURE_SCALE]);
        }
        
        if(bReflectionMap && !bSameReflectionScale && m_reflectionMapScale != default_scale) {
            m_reflectionMapScale.setUniform(pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_REFLECTIONTEXTURE_SCALE]);
        }
        
        if(bNormalMap && !bSameNormalScale && m_normalMapScale != default_scale) {
            m_normalMapScale.setUniform(pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_NORMALTEXTURE_SCALE]);
        }
        
        if(bDiffuseMap && !bSameDiffuseOffset && m_diffuseMapOffset != default_offset) {
            m_diffuseMapOffset.setUniform(pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_DIFFUSETEXTURE_OFFSET]);
        }
        
        if(bSpecMap && !bSameSpecularOffset && m_specularMapOffset != default_offset) {
            m_specularMapOffset.setUniform(pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_SPECULARTEXTURE_OFFSET]);
        }
        
        if(bReflectionMap && !bSameReflectionOffset && m_reflectionMapOffset != default_offset) {
            m_reflectionMapOffset.setUniform(pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_REFLECTIONTEXTURE_OFFSET]);
        }
        
        if(bNormalMap && !bSameNormalOffset && m_normalMapOffset != default_offset) {
            m_normalMapOffset.setUniform(pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_NORMALTEXTURE_OFFSET]);
        }
        
        GLDEBUG(glUniform1f(pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_MATERIAL_ALPHA], m_tr));
        
        if(bDiffuseMap) {
            m_pContext->getTextureManager()->selectTexture(0, m_pDiffuseMap);
        }
        
        if(bSpecMap) {
            m_pContext->getTextureManager()->selectTexture(1, m_pSpecularMap);
        }

        if(bNormalMap) {
            m_pContext->getTextureManager()->selectTexture(2, m_pNormalMap);
        }
        
        if(bReflectionCubeMap && (renderPass == KRNode::RENDER_PASS_FORWARD_OPAQUE || renderPass == KRNode::RENDER_PASS_DEFERRED_OPAQUE)) {
            m_pContext->getTextureManager()->selectTexture(4, m_pReflectionCube);
        }
        
        if(bReflectionMap && (renderPass == KRNode::RENDER_PASS_FORWARD_OPAQUE || renderPass == KRNode::RENDER_PASS_DEFERRED_OPAQUE)) {
            // GL_TEXTURE7 is used for reading the depth buffer in gBuffer pass 2 and re-used for the reflection map in gBuffer Pass 3 and in forward rendering
            m_pContext->getTextureManager()->selectTexture(7, m_pReflectionMap);
        }
        
        *prevBoundMaterial = this;
    } // if(!bSameMaterial)
    
    return true;
}

const std::string &KRMaterial::getName() const
{
    return m_name;
}

