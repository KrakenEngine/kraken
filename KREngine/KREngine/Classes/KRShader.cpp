//
//  KRShader.cpp
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

#include "KRShader.h"
#import "assert.h"
#include "KRLight.h"
#include "KRDirectionalLight.h"
#include "KRSpotLight.h"
#include "KRPointLight.h"

KRShader::KRShader(KRContext &context, char *szKey, std::string options, std::string vertShaderSource, const std::string fragShaderSource) : KRContextObject(context)
{
    strcpy(m_szKey, szKey);
    m_iProgram = 0;
    GLuint vertexShader = 0, fragShader = 0;
    try {
        const GLchar *vertSource[2] = {options.c_str(), vertShaderSource.c_str()};
        const GLchar *fragSource[2] = {options.c_str(), fragShaderSource.c_str()};
        
        // Create shader program.
        GLDEBUG(m_iProgram = glCreateProgram());
        
        // Create and compile vertex shader.
        GLDEBUG(vertexShader = glCreateShader(GL_VERTEX_SHADER));
        GLDEBUG(glShaderSource(vertexShader, 2, vertSource, NULL));
        GLDEBUG(glCompileShader(vertexShader));
        
        // Report any compile issues to stderr
        GLint logLength;
        GLDEBUG(glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &logLength));
        if (logLength > 0) {
            GLchar *log = (GLchar *)malloc(logLength);
            assert(log != NULL);
            GLDEBUG(glGetShaderInfoLog(vertexShader, logLength, &logLength, log));
            fprintf(stderr, "KREngine - Failed to compile vertex shader: %s\nShader compile log:\n%s", szKey, log);
            free(log);
        }

        
        // Create and compile vertex shader.
        GLDEBUG(fragShader = glCreateShader(GL_FRAGMENT_SHADER));
        GLDEBUG(glShaderSource(fragShader, 2, fragSource, NULL));
        GLDEBUG(glCompileShader(fragShader));
        
        // Report any compile issues to stderr
        GLDEBUG(glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &logLength));
        if (logLength > 0) {
            GLchar *log = (GLchar *)malloc(logLength);
            assert(log != NULL);
            GLDEBUG(glGetShaderInfoLog(fragShader, logLength, &logLength, log));
            fprintf(stderr, "KREngine - Failed to compile fragment shader: %s\nShader compile log:\n%s", szKey, log);
            free(log);
        }
        
        // Attach vertex shader to program.
        GLDEBUG(glAttachShader(m_iProgram, vertexShader));
        
        // Attach fragment shader to program.
        GLDEBUG(glAttachShader(m_iProgram, fragShader));
        
        // Bind attribute locations.
        // This needs to be done prior to linking.
        GLDEBUG(glBindAttribLocation(m_iProgram, KRModel::KRENGINE_ATTRIB_VERTEX, "vertex_position"));
        GLDEBUG(glBindAttribLocation(m_iProgram, KRModel::KRENGINE_ATTRIB_NORMAL, "vertex_normal"));
        GLDEBUG(glBindAttribLocation(m_iProgram, KRModel::KRENGINE_ATTRIB_TANGENT, "vertex_tangent"));
        GLDEBUG(glBindAttribLocation(m_iProgram, KRModel::KRENGINE_ATTRIB_TEXUVA, "vertex_uv"));
        GLDEBUG(glBindAttribLocation(m_iProgram, KRModel::KRENGINE_ATTRIB_TEXUVB, "vertex_lightmap_uv"));
        GLDEBUG(glBindAttribLocation(m_iProgram, KRModel::KRENGINE_ATTRIB_BONEINDEXES, "bone_indexes"));
        GLDEBUG(glBindAttribLocation(m_iProgram, KRModel::KRENGINE_ATTRIB_BONEWEIGHTS, "bone_weights"));
        
        // Link program.
        GLDEBUG(glLinkProgram(m_iProgram));
        
        GLint link_success = GL_FALSE;
        GLDEBUG(glGetProgramiv(m_iProgram, GL_LINK_STATUS, &link_success));
        
        if(link_success != GL_TRUE) {
            // Report any linking issues to stderr
            fprintf(stderr, "KREngine - Failed to link shader program: %s\n", szKey);
                    
            GLDEBUG(glGetProgramiv(m_iProgram, GL_INFO_LOG_LENGTH, &logLength));
            if (logLength > 0)
            {
                GLchar *log = (GLchar *)malloc(logLength);
                assert(log != NULL);
                GLDEBUG(glGetProgramInfoLog(m_iProgram, logLength, &logLength, log));
                fprintf(stderr, "Program link log:\n%s", log);
                free(log);
            }
            GLDEBUG(glDeleteProgram(m_iProgram));
            m_iProgram = 0;
        } else {
        
            // Get uniform locations
            
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_MATERIAL_AMBIENT] = glGetUniformLocation(m_iProgram, "material_ambient"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_MATERIAL_DIFFUSE] = glGetUniformLocation(m_iProgram, "material_diffuse"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_MATERIAL_SPECULAR] = glGetUniformLocation(m_iProgram, "material_specular"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_MATERIAL_REFLECTION] = glGetUniformLocation(m_iProgram, "material_reflection"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_MATERIAL_ALPHA] = glGetUniformLocation(m_iProgram, "material_alpha"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_MATERIAL_SHININESS] = glGetUniformLocation(m_iProgram, "material_shininess"));
            
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_LIGHT_POSITION] = glGetUniformLocation(m_iProgram, "light_position"));
            
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_LIGHT_COLOR] = glGetUniformLocation(m_iProgram, "light_color"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_LIGHT_INTENSITY] = glGetUniformLocation(m_iProgram, "light_intensity"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_LIGHT_DECAY_START] = glGetUniformLocation(m_iProgram, "light_decay_start"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_LIGHT_CUTOFF] = glGetUniformLocation(m_iProgram, "light_cutoff"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_LIGHT_DIRECTION_MODEL_SPACE] = glGetUniformLocation(m_iProgram, "light_direction_model_space"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_LIGHT_DIRECTION_VIEW_SPACE] = glGetUniformLocation(m_iProgram, "light_direction_view_space"));
            
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_FLARE_SIZE] = glGetUniformLocation(m_iProgram, "flare_size"));

            
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_VIEW_SPACE_MODEL_ORIGIN] = glGetUniformLocation(m_iProgram, "view_space_model_origin"));
            
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_MVP] = glGetUniformLocation(m_iProgram, "mvp_matrix"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_INVMVP] = glGetUniformLocation(m_iProgram, "inv_mvp_matrix"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_INVP] = glGetUniformLocation(m_iProgram, "inv_projection_matrix"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_INVMVP_NO_TRANSLATE] = glGetUniformLocation(m_iProgram, "inv_mvp_matrix_no_translate"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_MODEL_VIEW_INVERSE_TRANSPOSE] = glGetUniformLocation(m_iProgram, "model_view_inverse_transpose_matrix"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_MODEL_INVERSE_TRANSPOSE] = glGetUniformLocation(m_iProgram, "model_inverse_transpose_matrix"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_MODEL_VIEW] = glGetUniformLocation(m_iProgram, "model_view_matrix"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_MODEL_MATRIX] = glGetUniformLocation(m_iProgram, "model_matrix"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_PROJECTION_MATRIX] = glGetUniformLocation(m_iProgram, "projection_matrix"));
            
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_SHADOWMVP1] = glGetUniformLocation(m_iProgram, "shadow_mvp1"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_SHADOWMVP2] = glGetUniformLocation(m_iProgram, "shadow_mvp2"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_SHADOWMVP3] = glGetUniformLocation(m_iProgram, "shadow_mvp3"));

            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_CAMERAPOS_MODEL_SPACE] = glGetUniformLocation(m_iProgram, "camera_position_model_space"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_VIEWPORT] = glGetUniformLocation(m_iProgram, "viewport"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_DIFFUSETEXTURE] = glGetUniformLocation(m_iProgram, "diffuseTexture"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_SPECULARTEXTURE] = glGetUniformLocation(m_iProgram, "specularTexture"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_REFLECTIONTEXTURE] = glGetUniformLocation(m_iProgram, "reflectionTexture"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_REFLECTIONCUBETEXTURE] = glGetUniformLocation(m_iProgram, "reflectionCubeTexture"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_NORMALTEXTURE] = glGetUniformLocation(m_iProgram, "normalTexture"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_DIFFUSETEXTURE_SCALE] = glGetUniformLocation(m_iProgram, "diffuseTexture_Scale"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_SPECULARTEXTURE_SCALE] = glGetUniformLocation(m_iProgram, "specularTexture_Scale"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_REFLECTIONTEXTURE_SCALE] = glGetUniformLocation(m_iProgram, "reflectionTexture_Scale"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_NORMALTEXTURE_SCALE] = glGetUniformLocation(m_iProgram, "normalTexture_Scale"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_AMBIENTTEXTURE_SCALE] = glGetUniformLocation(m_iProgram, "ambientTexture_Scale"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_DIFFUSETEXTURE_OFFSET] = glGetUniformLocation(m_iProgram, "diffuseTexture_Offset"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_SPECULARTEXTURE_OFFSET] = glGetUniformLocation(m_iProgram, "specularTexture_Offset"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_REFLECTIONTEXTURE_OFFSET] = glGetUniformLocation(m_iProgram, "reflectionTexture_Offset"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_NORMALTEXTURE_OFFSET] = glGetUniformLocation(m_iProgram, "normalTexture_Offset"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_AMBIENTTEXTURE_OFFSET] = glGetUniformLocation(m_iProgram, "ambientTexture_Offset"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_LIGHTMAPTEXTURE] = glGetUniformLocation(m_iProgram, "lightmapTexture"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_SHADOWTEXTURE1] = glGetUniformLocation(m_iProgram, "shadowTexture1"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_SHADOWTEXTURE2] = glGetUniformLocation(m_iProgram, "shadowTexture2"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_SHADOWTEXTURE3] = glGetUniformLocation(m_iProgram, "shadowTexture3"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_GBUFFER_FRAME] = glGetUniformLocation(m_iProgram, "gbuffer_frame"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_GBUFFER_DEPTH] = glGetUniformLocation(m_iProgram, "gbuffer_depth"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_DEPTH_FRAME] = glGetUniformLocation(m_iProgram, "depthFrame"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_RENDER_FRAME] = glGetUniformLocation(m_iProgram, "renderFrame"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_ABSOLUTE_TIME] = glGetUniformLocation(m_iProgram, "time_absolute"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_SLICE_DEPTH_SCALE] = glGetUniformLocation(m_iProgram, "slice_depth_scale"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_VOLUMETRIC_ENVIRONMENT_FRAME] = glGetUniformLocation(m_iProgram, "volumetricEnvironmentFrame"));
            
            
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_FOG_NEAR] = glGetUniformLocation(m_iProgram, "fog_near"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_FOG_FAR] = glGetUniformLocation(m_iProgram, "fog_far"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_FOG_DENSITY] = glGetUniformLocation(m_iProgram, "fog_density"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_FOG_COLOR] = glGetUniformLocation(m_iProgram, "fog_color"));
            
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_FOG_SCALE] = glGetUniformLocation(m_iProgram, "fog_scale"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_DENSITY_PREMULTIPLIED_EXPONENTIAL] = glGetUniformLocation(m_iProgram, "fog_density_premultiplied_exponential"));
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_DENSITY_PREMULTIPLIED_SQUARED] = glGetUniformLocation(m_iProgram, "fog_density_premultiplied_squared"));
            
            
            GLDEBUG(m_uniforms[KRENGINE_UNIFORM_PARTICLE_ORIGIN] = glGetUniformLocation(m_iProgram, "particle_origin"));

            
        }
        
    } catch(...) {
        if(vertexShader) {
            GLDEBUG(glDeleteShader(vertexShader));
            vertexShader = 0;
        }
        if(fragShader) {
            GLDEBUG(glDeleteShader(fragShader));
            fragShader = 0;
        }
        if(m_iProgram) {
            GLDEBUG(glDeleteProgram(m_iProgram));
            m_iProgram = 0;
        }
    }
    
    // Release vertex and fragment shaders.
    if (vertexShader) {
        GLDEBUG(glDeleteShader(vertexShader));
	}
    if (fragShader) {
        GLDEBUG(glDeleteShader(fragShader));
	}
}

KRShader::~KRShader() {
    if(m_iProgram) {
        GLDEBUG(glDeleteProgram(m_iProgram));
    }
}

#if TARGET_OS_IPHONE

bool KRShader::bind(KRCamera &camera, const KRViewport &viewport, const KRMat4 &matModel, const std::vector<KRLight *> &lights, const KRNode::RenderPass &renderPass) const {
    if(m_iProgram == 0) {
        return false;
    }
    
    
    
    GLDEBUG(glUseProgram(m_iProgram));
    
    GLDEBUG(glUniform1f(m_uniforms[KRENGINE_UNIFORM_ABSOLUTE_TIME], getContext().getAbsoluteTime()));
    
    int light_directional_count = 0;
    int light_point_count = 0;
    int light_spot_count = 0;
    // TODO - Need to support multiple lights and more light types in forward rendering
    if(renderPass != KRNode::RENDER_PASS_DEFERRED_LIGHTS && renderPass != KRNode::RENDER_PASS_DEFERRED_GBUFFER && renderPass != KRNode::RENDER_PASS_DEFERRED_OPAQUE) {
        for(std::vector<KRLight *>::const_iterator light_itr=lights.begin(); light_itr != lights.end(); light_itr++) {
            KRLight *light = (*light_itr);
            KRDirectionalLight *directional_light = dynamic_cast<KRDirectionalLight *>(light);
            KRPointLight *point_light = dynamic_cast<KRPointLight *>(light);
            KRSpotLight *spot_light = dynamic_cast<KRSpotLight *>(light);
            if(directional_light) {
                if(light_directional_count == 0) {
                    int cShadowBuffers = directional_light->getShadowBufferCount();
                    if(m_uniforms[KRENGINE_UNIFORM_SHADOWTEXTURE1] != -1 && cShadowBuffers > 0) {
                        m_pContext->getTextureManager()->selectTexture(3, NULL);
                        GLDEBUG(glActiveTexture(GL_TEXTURE3));
                        GLDEBUG(glBindTexture(GL_TEXTURE_2D, directional_light->getShadowTextures()[0]));
                        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
                        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
                        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
                        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
                    }
                
                    if(m_uniforms[KRENGINE_UNIFORM_SHADOWTEXTURE2] != -1 && cShadowBuffers > 1 && camera.m_cShadowBuffers > 1) {
                        m_pContext->getTextureManager()->selectTexture(4, NULL);
                        GLDEBUG(glActiveTexture(GL_TEXTURE4));
                        GLDEBUG(glBindTexture(GL_TEXTURE_2D, directional_light->getShadowTextures()[1]));
                        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
                        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
                        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
                        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
                    }
                
                    if(m_uniforms[KRENGINE_UNIFORM_SHADOWTEXTURE3] != -1 && cShadowBuffers > 2 && camera.m_cShadowBuffers > 2) {
                        m_pContext->getTextureManager()->selectTexture(5, NULL);
                        GLDEBUG(glActiveTexture(GL_TEXTURE5));
                        GLDEBUG(glBindTexture(GL_TEXTURE_2D, directional_light->getShadowTextures()[2]));
                        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
                        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
                        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
                        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
                    }
                    
                    KRMat4 matBias;
                    matBias.translate(1.0, 1.0, 1.0);
                    matBias.scale(0.5);
                    for(int iShadow=0; iShadow < cShadowBuffers; iShadow++) {
                        (matModel * directional_light->getShadowViewports()[iShadow].getViewProjectionMatrix() * matBias).setUniform(m_uniforms[KRENGINE_UNIFORM_SHADOWMVP1 + iShadow]);
                    }
                    
                    if(m_uniforms[KRENGINE_UNIFORM_LIGHT_DIRECTION_MODEL_SPACE] != -1) {
                        KRMat4 inverseModelMatrix = matModel;
                        inverseModelMatrix.invert();
                        
                        // Bind the light direction vector
                        KRVector3 lightDirObject = KRMat4::Dot(inverseModelMatrix, directional_light->getWorldLightDirection());
                        lightDirObject.normalize();
                        lightDirObject.setUniform(m_uniforms[KRENGINE_UNIFORM_LIGHT_DIRECTION_MODEL_SPACE]);
                    }
                }
                
                light_directional_count++;
            }
            if(point_light) {
                light_point_count++;
            }
            if(spot_light) {
                light_spot_count++;
            }
        }
    }
    

    
    if(m_uniforms[KRENGINE_UNIFORM_CAMERAPOS_MODEL_SPACE] != -1) {
        KRMat4 inverseModelMatrix = matModel;
        inverseModelMatrix.invert();
        
        if(m_uniforms[KRENGINE_UNIFORM_CAMERAPOS_MODEL_SPACE] != -1) {
            // Transform location of camera to object space for calculation of specular halfVec
            KRVector3 cameraPosObject = KRMat4::Dot(inverseModelMatrix, viewport.getCameraPosition());
            cameraPosObject.setUniform(m_uniforms[KRENGINE_UNIFORM_CAMERAPOS_MODEL_SPACE]);
        }
    }
    
    if(m_uniforms[KRENGINE_UNIFORM_MVP] != -1 || m_uniforms[KRShader::KRENGINE_UNIFORM_INVMVP] != -1) {
        // Bind our modelmatrix variable to be a uniform called mvpmatrix in our shaderprogram
        KRMat4 mvpMatrix = matModel * viewport.getViewProjectionMatrix();
        mvpMatrix.setUniform(m_uniforms[KRENGINE_UNIFORM_MVP]);
        
        if(m_uniforms[KRShader::KRENGINE_UNIFORM_INVMVP] != -1) {
            KRMat4::Invert(mvpMatrix).setUniform(m_uniforms[KRShader::KRENGINE_UNIFORM_INVMVP]);
        }
    }
    
    if(m_uniforms[KRShader::KRENGINE_UNIFORM_VIEW_SPACE_MODEL_ORIGIN] != -1 || m_uniforms[KRENGINE_UNIFORM_MODEL_VIEW_INVERSE_TRANSPOSE] != -1 || m_uniforms[KRShader::KRENGINE_UNIFORM_MODEL_VIEW] != -1) {
        KRMat4 matModelView = matModel * viewport.getViewMatrix();
        matModelView.setUniform(m_uniforms[KRShader::KRENGINE_UNIFORM_MODEL_VIEW]);
        
        
        if(m_uniforms[KRShader::KRENGINE_UNIFORM_VIEW_SPACE_MODEL_ORIGIN] != -1) {
            KRVector3 view_space_model_origin = KRMat4::Dot(matModelView, KRVector3::Zero()); // Origin point of model space is the light source position.  No perspective, so no w divide required
            view_space_model_origin.setUniform(m_uniforms[KRShader::KRENGINE_UNIFORM_VIEW_SPACE_MODEL_ORIGIN]);
        }
        
        if(m_uniforms[KRENGINE_UNIFORM_MODEL_VIEW_INVERSE_TRANSPOSE] != -1) {
            KRMat4 matModelViewInverseTranspose = matModelView;
            matModelViewInverseTranspose.transpose();
            matModelViewInverseTranspose.invert();
            matModelViewInverseTranspose.setUniform(m_uniforms[KRENGINE_UNIFORM_MODEL_VIEW_INVERSE_TRANSPOSE]);
        }
    }
    
    if(m_uniforms[KRENGINE_UNIFORM_MODEL_INVERSE_TRANSPOSE] != -1) {
        KRMat4 matModelInverseTranspose = matModel;
        matModelInverseTranspose.transpose();
        matModelInverseTranspose.invert();
        matModelInverseTranspose.setUniform(m_uniforms[KRENGINE_UNIFORM_MODEL_INVERSE_TRANSPOSE]);
    }
    
    if(m_uniforms[KRShader::KRENGINE_UNIFORM_INVP] != -1) {
        viewport.getInverseProjectionMatrix().setUniform(m_uniforms[KRShader::KRENGINE_UNIFORM_INVP]);
    }
    
    if(m_uniforms[KRShader::KRENGINE_UNIFORM_INVMVP_NO_TRANSLATE] != -1) {
        KRMat4 matInvMVPNoTranslate = matModel * viewport.getViewMatrix();;
        // Remove the translation
        matInvMVPNoTranslate.getPointer()[3] = 0;
        matInvMVPNoTranslate.getPointer()[7] = 0;
        matInvMVPNoTranslate.getPointer()[11] = 0;
        matInvMVPNoTranslate.getPointer()[12] = 0;
        matInvMVPNoTranslate.getPointer()[13] = 0;
        matInvMVPNoTranslate.getPointer()[14] = 0;
        matInvMVPNoTranslate.getPointer()[15] = 1.0;
        matInvMVPNoTranslate = matInvMVPNoTranslate * viewport.getProjectionMatrix();
        matInvMVPNoTranslate.invert();
        matInvMVPNoTranslate.setUniform(m_uniforms[KRShader::KRENGINE_UNIFORM_INVMVP_NO_TRANSLATE]);
    }
    
    matModel.setUniform(m_uniforms[KRShader::KRENGINE_UNIFORM_MODEL_MATRIX]);
    if(m_uniforms[KRENGINE_UNIFORM_PROJECTION_MATRIX] != -1) {
        viewport.getProjectionMatrix().setUniform(m_uniforms[KRENGINE_UNIFORM_PROJECTION_MATRIX]);
    }
    
    if(m_uniforms[KRENGINE_UNIFORM_VIEWPORT] != -1) {
        GLDEBUG(glUniform4f(
                    m_uniforms[KRENGINE_UNIFORM_VIEWPORT],
                    (GLfloat)0.0,
                    (GLfloat)0.0,
                    (GLfloat)viewport.getSize().x,
                    (GLfloat)viewport.getSize().y
        ));
    }
    
    // Fog parameters
    GLDEBUG(glUniform1f(m_uniforms[KRENGINE_UNIFORM_FOG_NEAR], camera.fog_near));
    GLDEBUG(glUniform1f(m_uniforms[KRENGINE_UNIFORM_FOG_FAR], camera.fog_far));
    GLDEBUG(glUniform1f(m_uniforms[KRENGINE_UNIFORM_FOG_DENSITY], camera.fog_density));
    camera.fog_color.setUniform(m_uniforms[KRENGINE_UNIFORM_FOG_COLOR]);
    
    
    if(m_uniforms[KRENGINE_UNIFORM_FOG_SCALE] != -1) {
        GLDEBUG(glUniform1f(m_uniforms[KRENGINE_UNIFORM_FOG_SCALE], 1.0f / (camera.fog_far - camera.fog_near)));
    }
    if(m_uniforms[KRENGINE_UNIFORM_DENSITY_PREMULTIPLIED_EXPONENTIAL] != -1) {
        GLDEBUG(glUniform1f(m_uniforms[KRENGINE_UNIFORM_DENSITY_PREMULTIPLIED_EXPONENTIAL], -camera.fog_density * 1.442695f)); // -fog_density / log(2)
    }
    if(m_uniforms[KRENGINE_UNIFORM_DENSITY_PREMULTIPLIED_SQUARED] != -1) {
        GLDEBUG(glUniform1f(m_uniforms[KRENGINE_UNIFORM_DENSITY_PREMULTIPLIED_SQUARED],  -camera.fog_density * camera.fog_density * 1.442695)); // -fog_density * fog_density / log(2)
    }
    
    // Sets the diffuseTexture variable to the first texture unit
    GLDEBUG(glUniform1i(m_uniforms[KRENGINE_UNIFORM_DIFFUSETEXTURE], 0));
    
    // Sets the specularTexture variable to the second texture unit
    GLDEBUG(glUniform1i(m_uniforms[KRENGINE_UNIFORM_SPECULARTEXTURE], 1));
    
    // Sets the normalTexture variable to the third texture unit
    GLDEBUG(glUniform1i(m_uniforms[KRENGINE_UNIFORM_NORMALTEXTURE], 2));
    
    // Sets the shadowTexture variable to the fourth texture unit
    GLDEBUG(glUniform1i(m_uniforms[KRENGINE_UNIFORM_SHADOWTEXTURE1], 3));
    GLDEBUG(glUniform1i(m_uniforms[KRENGINE_UNIFORM_SHADOWTEXTURE2], 4));
    GLDEBUG(glUniform1i(m_uniforms[KRENGINE_UNIFORM_SHADOWTEXTURE3], 5));
    
    GLDEBUG(glUniform1i(m_uniforms[KRENGINE_UNIFORM_REFLECTIONCUBETEXTURE], 4));
    GLDEBUG(glUniform1i(m_uniforms[KRENGINE_UNIFORM_LIGHTMAPTEXTURE], 5));
    GLDEBUG(glUniform1i(m_uniforms[KRENGINE_UNIFORM_GBUFFER_FRAME], 6));
    GLDEBUG(glUniform1i(m_uniforms[KRENGINE_UNIFORM_GBUFFER_DEPTH], 7)); // Texture unit 7 is used for reading the depth buffer in gBuffer pass #2 and in post-processing pass
    GLDEBUG(glUniform1i(m_uniforms[KRENGINE_UNIFORM_REFLECTIONTEXTURE], 7)); // Texture unit 7 is used for the reflection map textures in gBuffer pass #3 and when using forward rendering
    
    GLDEBUG(glUniform1i(m_uniforms[KRENGINE_UNIFORM_DEPTH_FRAME], 0));
    GLDEBUG(glUniform1i(m_uniforms[KRENGINE_UNIFORM_RENDER_FRAME], 1));
    GLDEBUG(glUniform1i(m_uniforms[KRENGINE_UNIFORM_VOLUMETRIC_ENVIRONMENT_FRAME], 2));
    
#if defined(DEBUG)
    GLint logLength;
    
    GLint validate_status = GL_FALSE;
    GLDEBUG(glValidateProgram(m_iProgram));
    GLDEBUG(glGetProgramiv(m_iProgram, GL_VALIDATE_STATUS, &validate_status));
    if(validate_status != GL_TRUE) {
        fprintf(stderr, "KREngine - Failed to validate shader program: %s\n", m_szKey);
        GLDEBUG(glGetProgramiv(m_iProgram, GL_INFO_LOG_LENGTH, &logLength));
        if (logLength > 0)
        {
            GLchar *log = (GLchar *)malloc(logLength);
            assert(log != NULL);
            GLDEBUG(glGetProgramInfoLog(m_iProgram, logLength, &logLength, log));
            fprintf(stderr, "Program validate log:\n%s", log);
            free(log);
            
        }
        return false;
    }
#endif

    return true;
}

#endif

const char *KRShader::getKey() const {
    return m_szKey;
}
