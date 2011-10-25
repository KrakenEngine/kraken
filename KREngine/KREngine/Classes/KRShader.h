//
//  KRShader.h
//  KREngine
//
//  Created by Kearwood Gilbert on 11-08-11.
//  Copyright 2011 Kearwood Software. All rights reserved.
//

#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import <stdint.h>
#import <vector>
#import <string>

using std::vector;

#ifndef KRSHADER_H
#define KRSHADER_H

#import "KRShader.h"
#import "KRMat4.h"
#import "KRCamera.h"

class KRShader {
public:
    KRShader(std::string options, const GLchar *szVertShaderSource, const GLchar *szFragShaderSource);
    ~KRShader();
    GLuint getProgram();
    
    void bind(KRCamera *pCamera, KRMat4 &mvpMatrix, Vector3 &cameraPosition, Vector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers);
    
    enum {
        KRENGINE_ATTRIB_VERTEX,
        KRENGINE_ATTRIB_NORMAL,
        KRENGINE_ATTRIB_TANGENT,
        KRENGINE_ATTRIB_TEXUV,
        KRENGINE_NUM_ATTRIBUTES
    };
    
    enum {
        KRENGINE_UNIFORM_MATERIAL_AMBIENT,
        KRENGINE_UNIFORM_MATERIAL_DIFFUSE,
        KRENGINE_UNIFORM_MATERIAL_SPECULAR,
        KRENGINE_UNIFORM_MATERIAL_ALPHA,
        KRENGINE_UNIFORM_MATERIAL_SHININESS,
        KRENGINE_UNIFORM_MVP,
        KRENGINE_UNIFORM_LIGHTDIRECTION,
        KRENGINE_UNIFORM_CAMERAPOS,
        KRENGINE_UNIFORM_DIFFUSETEXTURE,
        KRENGINE_UNIFORM_SPECULARTEXTURE,
        KRENGINE_UNIFORM_NORMALTEXTURE,
        KRENGINE_UNIFORM_SHADOWMVP1,
        KRENGINE_UNIFORM_SHADOWMVP2,
        KRENGINE_UNIFORM_SHADOWMVP3,
        KRENGINE_UNIFORM_SHADOWTEXTURE1,
        KRENGINE_UNIFORM_SHADOWTEXTURE2,
        KRENGINE_UNIFORM_SHADOWTEXTURE3,
        
        KRENGINE_NUM_UNIFORMS
    };
    GLint m_uniforms[KRENGINE_NUM_UNIFORMS];
    
private:
    GLuint m_iProgram;
};

#endif
