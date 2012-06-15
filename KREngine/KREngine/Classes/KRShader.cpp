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

KRShader::KRShader(char *szKey, std::string options, std::string vertShaderSource, const std::string fragShaderSource) {
    strcpy(m_szKey, szKey);
    m_iProgram = 0;
    GLuint vertexShader = 0, fragShader = 0;
    try {
        const GLchar *vertSource[2] = {options.c_str(), vertShaderSource.c_str()};
        const GLchar *fragSource[2] = {options.c_str(), fragShaderSource.c_str()};
        
        // Create shader program.
        m_iProgram = glCreateProgram();
        
        // Create and compile vertex shader.
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 2, vertSource, NULL);
        glCompileShader(vertexShader);
        
        // Report any compile issues to stderr
        GLint logLength;
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
            GLchar *log = (GLchar *)malloc(logLength);
            glGetShaderInfoLog(vertexShader, logLength, &logLength, log);
            fprintf(stderr, "KREngine - Failed to compile vertex shader: %s\nShader compile log:\n%s", szKey, log);
            free(log);
        }

        
        // Create and compile vertex shader.
        fragShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragShader, 2, fragSource, NULL);
        glCompileShader(fragShader);
        
        // Report any compile issues to stderr
        glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
            GLchar *log = (GLchar *)malloc(logLength);
            glGetShaderInfoLog(fragShader, logLength, &logLength, log);
            fprintf(stderr, "KREngine - Failed to compile fragment shader: %s\nShader compile log:\n%s", szKey, log);
            free(log);
        }
        
        // Attach vertex shader to program.
        glAttachShader(m_iProgram, vertexShader);
        
        // Attach fragment shader to program.
        glAttachShader(m_iProgram, fragShader);
        
        // Bind attribute locations.
        // This needs to be done prior to linking.
        glBindAttribLocation(m_iProgram, KRENGINE_ATTRIB_VERTEX, "vertex_position");
        glBindAttribLocation(m_iProgram, KRENGINE_ATTRIB_NORMAL, "vertex_normal");
        glBindAttribLocation(m_iProgram, KRENGINE_ATTRIB_TANGENT, "vertex_tangent");
        glBindAttribLocation(m_iProgram, KRENGINE_ATTRIB_TEXUVA, "vertex_uv");
        glBindAttribLocation(m_iProgram, KRENGINE_ATTRIB_TEXUVB, "vertex_lightmap_uv");

        
        // Link program.
        glLinkProgram(m_iProgram);
        
        // Report any linking issues to stderr
        glGetProgramiv(m_iProgram, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0)
        {
            GLchar *log = (GLchar *)malloc(logLength);
            glGetProgramInfoLog(m_iProgram, logLength, &logLength, log);
            fprintf(stderr, "KREngine - Failed to link shader program: %s\nProgram link log:\n%s", szKey, log);
            free(log);
        }
        
        // Get uniform locations
        m_uniforms[KRENGINE_UNIFORM_MATERIAL_AMBIENT] = glGetUniformLocation(m_iProgram, "material_ambient");
        m_uniforms[KRENGINE_UNIFORM_MATERIAL_DIFFUSE] = glGetUniformLocation(m_iProgram, "material_diffuse");
        m_uniforms[KRENGINE_UNIFORM_MATERIAL_SPECULAR] = glGetUniformLocation(m_iProgram, "material_specular");
        m_uniforms[KRENGINE_UNIFORM_MATERIAL_REFLECTION] = glGetUniformLocation(m_iProgram, "material_reflection");
        m_uniforms[KRENGINE_UNIFORM_MATERIAL_REFLECTIVITY] = glGetUniformLocation(m_iProgram, "material_reflectivity");
        
        m_uniforms[KRENGINE_UNIFORM_LIGHT_POSITION] = glGetUniformLocation(m_iProgram, "light_position");
        m_uniforms[KRENGINE_UNIFORM_LIGHT_POSITION_VIEW_SPACE] = glGetUniformLocation(m_iProgram, "view_space_light_position");
        m_uniforms[KRENGINE_UNIFORM_LIGHT_COLOR] = glGetUniformLocation(m_iProgram, "light_color");
        m_uniforms[KRENGINE_UNIFORM_LIGHT_INTENSITY] = glGetUniformLocation(m_iProgram, "light_intensity");
        m_uniforms[KRENGINE_UNIFORM_LIGHT_DECAY_START] = glGetUniformLocation(m_iProgram, "light_decay_start");
        m_uniforms[KRENGINE_UNIFORM_LIGHT_CUTOFF] = glGetUniformLocation(m_iProgram, "light_cutoff");
        m_uniforms[KRENGINE_UNIFORM_FLARE_SIZE] = glGetUniformLocation(m_iProgram, "flare_size");
        
        
        

        m_uniforms[KRENGINE_UNIFORM_MATERIAL_ALPHA] = glGetUniformLocation(m_iProgram, "material_alpha");
        m_uniforms[KRENGINE_UNIFORM_MATERIAL_SHININESS] = glGetUniformLocation(m_iProgram, "material_shininess");
        
        m_uniforms[KRENGINE_UNIFORM_MVP] = glGetUniformLocation(m_iProgram, "mvp_matrix");
        m_uniforms[KRENGINE_UNIFORM_INVP] = glGetUniformLocation(m_iProgram, "inv_projection_matrix");
        
        m_uniforms[KRENGINE_UNIFORM_MN2V] = glGetUniformLocation(m_iProgram, "model_normal_to_view_matrix");
        m_uniforms[KRENGINE_UNIFORM_M2V] = glGetUniformLocation(m_iProgram, "model_to_view_matrix");
        m_uniforms[KRENGINE_UNIFORM_V2M] = glGetUniformLocation(m_iProgram, "view_to_model_matrix");
        m_uniforms[KRENGINE_UNIFORM_SHADOWMVP1] = glGetUniformLocation(m_iProgram, "shadow_mvp1");
        m_uniforms[KRENGINE_UNIFORM_SHADOWMVP2] = glGetUniformLocation(m_iProgram, "shadow_mvp2");
        m_uniforms[KRENGINE_UNIFORM_SHADOWMVP3] = glGetUniformLocation(m_iProgram, "shadow_mvp3");
        m_uniforms[KRENGINE_UNIFORM_LIGHT_DIRECTION] = glGetUniformLocation(m_iProgram, "light_direction");
        m_uniforms[KRENGINE_UNIFORM_LIGHT_DIRECTION_VIEW_SPACE] = glGetUniformLocation(m_iProgram, "light_direction_view_space");
        m_uniforms[KRENGINE_UNIFORM_CAMERAPOS] = glGetUniformLocation(m_iProgram, "cameraPosition");
        m_uniforms[KRENGINE_UNIFORM_VIEWPORT] = glGetUniformLocation(m_iProgram, "viewport");
        
        m_uniforms[KRENGINE_UNIFORM_DIFFUSETEXTURE] = glGetUniformLocation(m_iProgram, "diffuseTexture");
        m_uniforms[KRENGINE_UNIFORM_SPECULARTEXTURE] = glGetUniformLocation(m_iProgram, "specularTexture");
        m_uniforms[KRENGINE_UNIFORM_REFLECTIONTEXTURE] = glGetUniformLocation(m_iProgram, "reflectionTexture");
        m_uniforms[KRENGINE_UNIFORM_NORMALTEXTURE] = glGetUniformLocation(m_iProgram, "normalTexture");
        
        m_uniforms[KRENGINE_UNIFORM_DIFFUSETEXTURE_SCALE] = glGetUniformLocation(m_iProgram, "diffuseTexture_Scale");
        m_uniforms[KRENGINE_UNIFORM_SPECULARTEXTURE_SCALE] = glGetUniformLocation(m_iProgram, "specularTexture_Scale");
        m_uniforms[KRENGINE_UNIFORM_REFLECTIONTEXTURE_SCALE] = glGetUniformLocation(m_iProgram, "reflectionTexture_Scale");
        m_uniforms[KRENGINE_UNIFORM_NORMALTEXTURE_SCALE] = glGetUniformLocation(m_iProgram, "normalTexture_Scale");
        m_uniforms[KRENGINE_UNIFORM_AMBIENTTEXTURE_SCALE] = glGetUniformLocation(m_iProgram, "ambientTexture_Scale");
        m_uniforms[KRENGINE_UNIFORM_DIFFUSETEXTURE_OFFSET] = glGetUniformLocation(m_iProgram, "diffuseTexture_Offset");
        m_uniforms[KRENGINE_UNIFORM_SPECULARTEXTURE_OFFSET] = glGetUniformLocation(m_iProgram, "specularTexture_Offset");
        m_uniforms[KRENGINE_UNIFORM_REFLECTIONTEXTURE_OFFSET] = glGetUniformLocation(m_iProgram, "reflectionTexture_Offset");
        m_uniforms[KRENGINE_UNIFORM_NORMALTEXTURE_OFFSET] = glGetUniformLocation(m_iProgram, "normalTexture_Offset");
        m_uniforms[KRENGINE_UNIFORM_AMBIENTTEXTURE_OFFSET] = glGetUniformLocation(m_iProgram, "ambientTexture_Offset");
        
        m_uniforms[KRENGINE_UNIFORM_SHADOWTEXTURE1] = glGetUniformLocation(m_iProgram, "shadowTexture1");
        m_uniforms[KRENGINE_UNIFORM_SHADOWTEXTURE2] = glGetUniformLocation(m_iProgram, "shadowTexture2");
        m_uniforms[KRENGINE_UNIFORM_SHADOWTEXTURE3] = glGetUniformLocation(m_iProgram, "shadowTexture3");
        
        
        m_uniforms[KRENGINE_UNIFORM_GBUFFER_FRAME] = glGetUniformLocation(m_iProgram, "gbuffer_frame");
        m_uniforms[KRENGINE_UNIFORM_GBUFFER_DEPTH] = glGetUniformLocation(m_iProgram, "gbuffer_depth");
        
    } catch(...) {
        if(vertexShader) {
            glDeleteShader(vertexShader);
            vertexShader = 0;
        }
        if(fragShader) {
            glDeleteShader(fragShader);
            fragShader = 0;
        }
        if(m_iProgram) {
            glDeleteProgram(m_iProgram);
            m_iProgram = 0;
        }
    }
    
    // Release vertex and fragment shaders.
    if (vertexShader) {
        glDeleteShader(vertexShader);
	}
    if (fragShader) {
        glDeleteShader(fragShader);		
	}
}

KRShader::~KRShader() {
    if(m_iProgram) {
        glDeleteProgram(m_iProgram);
    }
}

#if TARGET_OS_IPHONE

void KRShader::bind(KRCamera *pCamera, KRMat4 &matModelToView, KRMat4 &mvpMatrix, KRVector3 &cameraPosition, KRVector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers, KRNode::RenderPass renderPass) {
    glUseProgram(m_iProgram);

    // Bind our modelmatrix variable to be a uniform called mvpmatrix in our shaderprogram
    glUniformMatrix4fv(m_uniforms[KRENGINE_UNIFORM_MVP], 1, GL_FALSE, mvpMatrix.getPointer());
    glUniformMatrix4fv(m_uniforms[KRENGINE_UNIFORM_MN2V], 1, GL_FALSE, matModelToView.getPointer());


    KRVector3 nLightDir = lightDirection;
    nLightDir.normalize();

    // Bind the light direction vector
    glUniform3f(
                m_uniforms[KRENGINE_UNIFORM_LIGHT_DIRECTION],
                (GLfloat)nLightDir.x,
                (GLfloat)nLightDir.y,
                (GLfloat)nLightDir.z
    );

    // Bind the camera position, in model space    
    glUniform3f(
                m_uniforms[KRENGINE_UNIFORM_CAMERAPOS],
                (GLfloat)cameraPosition.x,
                (GLfloat)cameraPosition.y,
                (GLfloat)cameraPosition.z
    );
    
    glUniform4f(
                m_uniforms[KRENGINE_UNIFORM_VIEWPORT],
                (GLfloat)0.0,
                (GLfloat)0.0,
                (GLfloat)pCamera->getViewportSize().x,
                (GLfloat)pCamera->getViewportSize().y
    );
    
    // Bind the shadowmap space matrices
    for(int iShadow=0; iShadow < cShadowBuffers; iShadow++) {
        glUniformMatrix4fv(m_uniforms[KRENGINE_UNIFORM_SHADOWMVP1 + iShadow], 1, GL_FALSE, pShadowMatrices[iShadow].getPointer());
    }
    
    // Sets the diffuseTexture variable to the first texture unit
    glUniform1i(m_uniforms[KRENGINE_UNIFORM_DIFFUSETEXTURE], 0);
    
    // Sets the specularTexture variable to the second texture unit
    glUniform1i(m_uniforms[KRENGINE_UNIFORM_SPECULARTEXTURE], 1);
    
    // Sets the normalTexture variable to the third texture unit
    glUniform1i(m_uniforms[KRENGINE_UNIFORM_NORMALTEXTURE], 2);
    
    // Sets the shadowTexture variable to the fourth texture unit
    glUniform1i(m_uniforms[KRENGINE_UNIFORM_SHADOWTEXTURE1], 3);
    glUniform1i(m_uniforms[KRENGINE_UNIFORM_SHADOWTEXTURE2], 4);
    glUniform1i(m_uniforms[KRENGINE_UNIFORM_SHADOWTEXTURE3], 5);
    
    glUniform1i(m_uniforms[KRENGINE_UNIFORM_GBUFFER_FRAME], 6);
    
    glUniform1i(m_uniforms[KRENGINE_UNIFORM_GBUFFER_DEPTH], 7); // Texture unit 7 is used for reading the depth buffer in gBuffer pass #2 and in post-processing pass
    glUniform1i(m_uniforms[KRENGINE_UNIFORM_REFLECTIONTEXTURE], 7); // Texture unit 7 is used for the reflection map textures in gBuffer pass #3 and when using forward rendering
    
#if defined(DEBUG)
    GLint logLength;
    
    glValidateProgram(m_iProgram);
    glGetProgramiv(m_iProgram, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(m_iProgram, logLength, &logLength, log);
        fprintf(stderr, "KREngine - Failed to validate shader program: %s\n Program validate log:\n%s", m_szKey, log);
        free(log);
    }
#endif

}

#endif

GLuint KRShader::getProgram() {
    return m_iProgram;
}

char *KRShader::getKey() {
    return m_szKey;
}
