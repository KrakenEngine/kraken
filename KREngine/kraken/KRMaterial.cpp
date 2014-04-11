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

bool KRMaterial::needsVertexTangents()
{
    return m_normalMap.size() > 0;
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
        stream << "\nmap_ReflectionCube " << m_reflectionCube << ".pvr";
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

kraken_stream_level KRMaterial::getStreamLevel(bool prime, float lodCoverage)
{
    kraken_stream_level stream_level = kraken_stream_level::STREAM_LEVEL_IN_HQ;
    
    getTextures();
    
    if(m_pAmbientMap) {
        stream_level = KRMIN(stream_level, m_pNormalMap->getStreamLevel(prime, lodCoverage, KRTexture::TEXTURE_USAGE_AMBIENT_MAP));
    }

    if(m_pDiffuseMap) {
        stream_level = KRMIN(stream_level, m_pDiffuseMap->getStreamLevel(prime, lodCoverage, KRTexture::TEXTURE_USAGE_DIFFUSE_MAP));
    }

    if(m_pNormalMap) {
        stream_level = KRMIN(stream_level, m_pNormalMap->getStreamLevel(prime, lodCoverage, KRTexture::TEXTURE_USAGE_NORMAL_MAP));
    }

    if(m_pSpecularMap) {
        stream_level = KRMIN(stream_level, m_pSpecularMap->getStreamLevel(prime, lodCoverage, KRTexture::TEXTURE_USAGE_SPECULAR_MAP));
    }

    if(m_pReflectionMap) {
        stream_level = KRMIN(stream_level, m_pReflectionMap->getStreamLevel(prime, lodCoverage, KRTexture::TEXTURE_USAGE_REFLECTION_MAP));
    }

    if(m_pReflectionCube) {
        stream_level = KRMIN(stream_level, m_pReflectionCube->getStreamLevel(prime, lodCoverage, KRTexture::TEXTURE_USAGE_REFECTION_CUBE));
    }
    
    return stream_level;
}

void KRMaterial::getTextures()
{
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
}

bool KRMaterial::bind(KRCamera *pCamera, std::vector<KRPointLight *> &point_lights, std::vector<KRDirectionalLight *> &directional_lights, std::vector<KRSpotLight *>&spot_lights, const std::vector<KRBone *> &bones, const std::vector<KRMat4> &bind_poses, const KRViewport &viewport, const KRMat4 &matModel, KRTexture *pLightMap, KRNode::RenderPass renderPass, const KRVector3 &rim_color, float rim_power, float lod_coverage) {
    bool bLightMap = pLightMap && pCamera->settings.bEnableLightMap;
    
    getTextures();
    
    KRVector2 default_scale = KRVector2::One();
    KRVector2 default_offset = KRVector2::Zero();
    
    bool bHasReflection = m_reflectionColor != KRVector3::Zero();
    bool bDiffuseMap = m_pDiffuseMap != NULL && pCamera->settings.bEnableDiffuseMap;
    bool bNormalMap = m_pNormalMap != NULL && pCamera->settings.bEnableNormalMap;
    bool bSpecMap = m_pSpecularMap != NULL && pCamera->settings.bEnableSpecMap;
    bool bReflectionMap = m_pReflectionMap != NULL && pCamera->settings.bEnableReflectionMap && pCamera->settings.bEnableReflection && bHasReflection;
    bool bReflectionCubeMap = m_pReflectionCube != NULL && pCamera->settings.bEnableReflection && bHasReflection;
    bool bAlphaTest = (m_alpha_mode == KRMATERIAL_ALPHA_MODE_TEST) && bDiffuseMap;
    bool bAlphaBlend = (m_alpha_mode == KRMATERIAL_ALPHA_MODE_BLENDONESIDE) || (m_alpha_mode == KRMATERIAL_ALPHA_MODE_BLENDTWOSIDE);
    
    KRShader *pShader = getContext().getShaderManager()->getShader("ObjectShader", pCamera, point_lights, directional_lights, spot_lights, bones.size(), bDiffuseMap, bNormalMap, bSpecMap, bReflectionMap, bReflectionCubeMap, bLightMap, m_diffuseMapScale != default_scale && bDiffuseMap, m_specularMapScale != default_scale && bSpecMap, m_normalMapScale != default_scale && bNormalMap, m_reflectionMapScale != default_scale && bReflectionMap, m_diffuseMapOffset != default_offset && bDiffuseMap, m_specularMapOffset != default_offset && bSpecMap, m_normalMapOffset != default_offset && bNormalMap, m_reflectionMapOffset != default_offset && bReflectionMap, bAlphaTest, bAlphaBlend, renderPass, rim_power != 0.0f);

    if(!getContext().getShaderManager()->selectShader(*pCamera, pShader, viewport, matModel, point_lights, directional_lights, spot_lights, 0, renderPass, rim_color, rim_power)) {
        return false;
    }
    
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
            
            KRMat4 skin_bone_bind_pose = bind_poses[bone_index];
            KRMat4 active_mat = bone->getActivePoseMatrix();
            KRMat4 inv_bind_mat = bone->getInverseBindPoseMatrix();
            KRMat4 inv_bind_mat2 = KRMat4::Invert(bind_poses[bone_index]);
            KRMat4 t = (inv_bind_mat * active_mat);
            KRMat4 t2 = inv_bind_mat2 * bone->getModelMatrix();
            for(int i=0; i < 16; i++) {
                *bone_mat_component++ = t[i];
            }
        }
        if(pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_BONE_TRANSFORMS] != -1) {
            glUniformMatrix4fv(pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_BONE_TRANSFORMS], bones.size(), GL_FALSE, bone_mats);
        }
    }

    
    pShader->setUniform(KRShader::KRENGINE_UNIFORM_MATERIAL_AMBIENT, m_ambientColor + pCamera->settings.ambient_intensity);
    
    if(renderPass == KRNode::RENDER_PASS_FORWARD_OPAQUE) {
        // We pre-multiply the light color with the material color in the forward renderer
        pShader->setUniform(KRShader::KRENGINE_UNIFORM_MATERIAL_DIFFUSE, KRVector3(m_diffuseColor.x * pCamera->settings.light_intensity.x, m_diffuseColor.y * pCamera->settings.light_intensity.y, m_diffuseColor.z * pCamera->settings.light_intensity.z));
    } else {
        pShader->setUniform(KRShader::KRENGINE_UNIFORM_MATERIAL_DIFFUSE, m_diffuseColor);
    }
    
    if(renderPass == KRNode::RENDER_PASS_FORWARD_OPAQUE) {
        // We pre-multiply the light color with the material color in the forward renderer
        pShader->setUniform(KRShader::KRENGINE_UNIFORM_MATERIAL_SPECULAR, KRVector3(m_specularColor.x * pCamera->settings.light_intensity.x, m_specularColor.y * pCamera->settings.light_intensity.y, m_specularColor.z * pCamera->settings.light_intensity.z));
    } else {
        pShader->setUniform(KRShader::KRENGINE_UNIFORM_MATERIAL_SPECULAR, m_specularColor);
    }
    
    pShader->setUniform(KRShader::KRENGINE_UNIFORM_MATERIAL_SHININESS, m_ns);
    pShader->setUniform(KRShader::KRENGINE_UNIFORM_MATERIAL_REFLECTION, m_reflectionColor);
    pShader->setUniform(KRShader::KRENGINE_UNIFORM_DIFFUSETEXTURE_SCALE, m_diffuseMapScale);
    pShader->setUniform(KRShader::KRENGINE_UNIFORM_SPECULARTEXTURE_SCALE, m_specularMapScale);
    pShader->setUniform(KRShader::KRENGINE_UNIFORM_REFLECTIONTEXTURE_SCALE, m_reflectionMapScale);
    pShader->setUniform(KRShader::KRENGINE_UNIFORM_NORMALTEXTURE_SCALE, m_normalMapScale);
    pShader->setUniform(KRShader::KRENGINE_UNIFORM_DIFFUSETEXTURE_OFFSET, m_diffuseMapOffset);
    pShader->setUniform(KRShader::KRENGINE_UNIFORM_SPECULARTEXTURE_OFFSET, m_specularMapOffset);
    pShader->setUniform(KRShader::KRENGINE_UNIFORM_REFLECTIONTEXTURE_OFFSET, m_reflectionMapOffset);
    pShader->setUniform(KRShader::KRENGINE_UNIFORM_NORMALTEXTURE_OFFSET, m_normalMapOffset);

    pShader->setUniform(KRShader::KRENGINE_UNIFORM_MATERIAL_ALPHA, m_tr);
    
    if(bDiffuseMap) {
        m_pContext->getTextureManager()->selectTexture(0, m_pDiffuseMap, lod_coverage, KRTexture::TEXTURE_USAGE_DIFFUSE_MAP);
    }
    
    if(bSpecMap) {
        m_pContext->getTextureManager()->selectTexture(1, m_pSpecularMap, lod_coverage, KRTexture::TEXTURE_USAGE_SPECULAR_MAP);
    }

    if(bNormalMap) {
        m_pContext->getTextureManager()->selectTexture(2, m_pNormalMap, lod_coverage, KRTexture::TEXTURE_USAGE_NORMAL_MAP);
    }
    
    if(bReflectionCubeMap && (renderPass == KRNode::RENDER_PASS_FORWARD_OPAQUE || renderPass == KRNode::RENDER_PASS_DEFERRED_OPAQUE)) {
        m_pContext->getTextureManager()->selectTexture(4, m_pReflectionCube, lod_coverage, KRTexture::TEXTURE_USAGE_REFECTION_CUBE);
    }
    
    if(bReflectionMap && (renderPass == KRNode::RENDER_PASS_FORWARD_OPAQUE || renderPass == KRNode::RENDER_PASS_DEFERRED_OPAQUE)) {
        // GL_TEXTURE7 is used for reading the depth buffer in gBuffer pass 2 and re-used for the reflection map in gBuffer Pass 3 and in forward rendering
        m_pContext->getTextureManager()->selectTexture(7, m_pReflectionMap, lod_coverage, KRTexture::TEXTURE_USAGE_REFLECTION_MAP);
    }

    
    return true;
}

const std::string &KRMaterial::getName() const
{
    return m_name;
}

