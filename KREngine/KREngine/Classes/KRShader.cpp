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

KRShader::KRShader(char *szKey, std::string options, const GLchar *szVertShaderSource, const GLchar *szFragShaderSource) {
    strcpy(m_szKey, szKey);
    m_iProgram = 0;
    GLuint vertexShader = 0, fragShader = 0;
    try {
        const GLchar *vertSource[2] = {options.c_str(), szVertShaderSource};
        const GLchar *fragSource[2] = {options.c_str(), szFragShaderSource};
        
        // Create shader program.
        m_iProgram = glCreateProgram();
        
        // Create and compile vertex shader.
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 2, vertSource, NULL);
        glCompileShader(vertexShader);
        
        // Create and compile vertex shader.
        fragShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragShader, 2, fragSource, NULL);
        glCompileShader(fragShader);
        
        // Attach vertex shader to program.
        glAttachShader(m_iProgram, vertexShader);
        
        // Attach fragment shader to program.
        glAttachShader(m_iProgram, fragShader);
        
        // Bind attribute locations.
        // This needs to be done prior to linking.
        glBindAttribLocation(m_iProgram, KRENGINE_ATTRIB_VERTEX, "position");
        glBindAttribLocation(m_iProgram, KRENGINE_ATTRIB_TEXUVA, "inputTextureCoordinate");
        glBindAttribLocation(m_iProgram, KRENGINE_ATTRIB_TEXUVB, "shadowuv");
        glBindAttribLocation(m_iProgram, KRENGINE_ATTRIB_VERTEX, "myVertex");
        glBindAttribLocation(m_iProgram, KRENGINE_ATTRIB_NORMAL, "myNormal");
        glBindAttribLocation(m_iProgram, KRENGINE_ATTRIB_TANGENT, "myTangent");
        glBindAttribLocation(m_iProgram, KRENGINE_ATTRIB_TEXUVA, "myUV");
        
        // Link program.
        glLinkProgram(m_iProgram);
        
        // Get uniform locations
        m_uniforms[KRENGINE_UNIFORM_MATERIAL_AMBIENT] = glGetUniformLocation(m_iProgram, "material_ambient");
        m_uniforms[KRENGINE_UNIFORM_MATERIAL_DIFFUSE] = glGetUniformLocation(m_iProgram, "material_diffuse");
        m_uniforms[KRENGINE_UNIFORM_MATERIAL_SPECULAR] = glGetUniformLocation(m_iProgram, "material_specular");
        m_uniforms[KRENGINE_UNIFORM_MATERIAL_ALPHA] = glGetUniformLocation(m_iProgram, "material_alpha");
        m_uniforms[KRENGINE_UNIFORM_MATERIAL_SHININESS] = glGetUniformLocation(m_iProgram, "material_shininess");
        
        m_uniforms[KRENGINE_UNIFORM_MVP] = glGetUniformLocation(m_iProgram, "myMVPMatrix");
        m_uniforms[KRENGINE_UNIFORM_M2V] = glGetUniformLocation(m_iProgram, "model_to_view");
        m_uniforms[KRENGINE_UNIFORM_SHADOWMVP1] = glGetUniformLocation(m_iProgram, "myShadowMVPMatrix1");
        m_uniforms[KRENGINE_UNIFORM_SHADOWMVP2] = glGetUniformLocation(m_iProgram, "myShadowMVPMatrix2");
        m_uniforms[KRENGINE_UNIFORM_SHADOWMVP3] = glGetUniformLocation(m_iProgram, "myShadowMVPMatrix3");
        m_uniforms[KRENGINE_UNIFORM_LIGHTDIRECTION] = glGetUniformLocation(m_iProgram, "lightDirection");
        m_uniforms[KRENGINE_UNIFORM_CAMERAPOS] = glGetUniformLocation(m_iProgram, "cameraPosition");
        
        m_uniforms[KRENGINE_UNIFORM_DIFFUSETEXTURE] = glGetUniformLocation(m_iProgram, "diffuseTexture");
        m_uniforms[KRENGINE_UNIFORM_SPECULARTEXTURE] = glGetUniformLocation(m_iProgram, "specularTexture");
        m_uniforms[KRENGINE_UNIFORM_NORMALTEXTURE] = glGetUniformLocation(m_iProgram, "normalTexture");
        
        m_uniforms[KRENGINE_UNIFORM_DIFFUSETEXTURE_SCALE] = glGetUniformLocation(m_iProgram, "diffuseTexture_Scale");
        m_uniforms[KRENGINE_UNIFORM_SPECULARTEXTURE_SCALE] = glGetUniformLocation(m_iProgram, "specularTexture_Scale");
        m_uniforms[KRENGINE_UNIFORM_NORMALTEXTURE_SCALE] = glGetUniformLocation(m_iProgram, "normalTexture_Scale");
        m_uniforms[KRENGINE_UNIFORM_AMBIENTTEXTURE_SCALE] = glGetUniformLocation(m_iProgram, "ambientTexture_Scale");
        m_uniforms[KRENGINE_UNIFORM_DIFFUSETEXTURE_OFFSET] = glGetUniformLocation(m_iProgram, "diffuseTexture_Offset");
        m_uniforms[KRENGINE_UNIFORM_SPECULARTEXTURE_OFFSET] = glGetUniformLocation(m_iProgram, "specularTexture_Offset");
        m_uniforms[KRENGINE_UNIFORM_NORMALTEXTURE_OFFSET] = glGetUniformLocation(m_iProgram, "normalTexture_Offset");
        m_uniforms[KRENGINE_UNIFORM_AMBIENTTEXTURE_OFFSET] = glGetUniformLocation(m_iProgram, "ambientTexture_Offset");
        
        m_uniforms[KRENGINE_UNIFORM_SHADOWTEXTURE1] = glGetUniformLocation(m_iProgram, "shadowTexture1");
        m_uniforms[KRENGINE_UNIFORM_SHADOWTEXTURE2] = glGetUniformLocation(m_iProgram, "shadowTexture2");
        m_uniforms[KRENGINE_UNIFORM_SHADOWTEXTURE3] = glGetUniformLocation(m_iProgram, "shadowTexture3");
        
        
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

void KRShader::bind(KRCamera *pCamera, KRMat4 &matModelToView, KRMat4 &mvpMatrix, KRVector3 &cameraPosition, KRVector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers) {
    glUseProgram(m_iProgram);
    

    // Bind our modelmatrix variable to be a uniform called mvpmatrix in our shaderprogram
    glUniformMatrix4fv(m_uniforms[KRENGINE_UNIFORM_MVP], 1, GL_FALSE, mvpMatrix.getPointer());
    glUniformMatrix4fv(m_uniforms[KRENGINE_UNIFORM_M2V], 1, GL_FALSE, matModelToView.getPointer());

    KRVector3 nLightDir = lightDirection;
    nLightDir.normalize();

    // Bind the light direction vector
    glUniform3f(
                m_uniforms[KRENGINE_UNIFORM_LIGHTDIRECTION],
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
    
#if defined(DEBUG)
    GLint logLength;
    
    glValidateProgram(m_iProgram);
    glGetProgramiv(m_iProgram, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(m_iProgram, logLength, &logLength, log);
        fprintf(stderr, "Program validate log:\n%s", log);
        free(log);
    }
#endif

}

GLuint KRShader::getProgram() {
    return m_iProgram;
}

char *KRShader::getKey() {
    return m_szKey;
}
