//
//  KREngine.h
//  gldemo
//
//  Created by Kearwood Gilbert on 10-09-16.
//  Copyright (c) 2010 Kearwood Software. All rights reserved.
//

#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

// #import "KRTextureManager.h"
#import <map>
#import <string>
#import "KRMat4.h"
#import "KRModel.h"
#import "KRTextureManager.h"
#import "KRMaterialManager.h"

using std::map;

@interface KREngine : NSObject
{
@private

    GLint backingWidth, backingHeight;
    
    GLuint compositeFramebuffer, compositeDepthTexture, compositeColorTexture;
    GLuint shadowFramebuffer, shadowDepthTexture;
    
    // uniform index
    enum {
        KRENGINE_UNIFORM_MATERIAL_AMBIENT,
        KRENGINE_UNIFORM_MATERIAL_DIFFUSE,
        KRENGINE_UNIFORM_MATERIAL_SPECULAR,
        KRENGINE_UNIFORM_MVP,
        KRENGINE_UNIFORM_MODEL,
        KRENGINE_UNIFORM_MODELIT, // Inverse Transform
        KRENGINE_NUM_UNIFORMS
    };
    GLint m_uniforms[KRENGINE_NUM_UNIFORMS];
    
    // attribute index
    enum {
        KRENGINE_ATTRIB_VERTEX,
        KRENGINE_ATTRIB_NORMAL,
        KRENGINE_ATTRIB_TANGENT,
        KRENGINE_ATTRIB_TEXUV,
        KRENGINE_NUM_ATTRIBUTES
    };

    
    GLuint m_objectShaderProgram;
    GLuint m_postShaderProgram;
    GLuint m_shadowShaderProgram;
    
    std::map<std::string, KRModel *> m_models;
    KRTextureManager *m_pTextureManager;
    KRMaterialManager *m_pMaterialManager;
}
- (id)initForWidth: (GLuint)width Height: (GLuint)height;
- (void)renderWithModelMatrix: (KRMat4)modelMatrix;
- (BOOL)loadVertexShader:(NSString *)vertexShaderName fragmentShader:(NSString *)fragmentShaderName forProgram:(GLuint *)programPointer;
- (BOOL)loadResource:(NSString *)path;

@end

