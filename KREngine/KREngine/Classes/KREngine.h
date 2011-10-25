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
#import "KRMat4.h"
#import "KRVector3.h"
#import "KRModel.h"
#import "KRScene.h"
#import "KRTextureManager.h"
#import "KRMaterialManager.h"
#import "KRShaderManager.h"
#import "KRModelManager.h"
#import "KRCamera.h"

typedef enum KREngineParameterType {KRENGINE_PARAMETER_INT, KRENGINE_PARAMETER_FLOAT, KRENGINE_PARAMETER_BOOL} KREngineParameterType;

#define KRENGINE_MAX_SHADOW_BUFFERS 3
#define KRENGINE_SHADOW_MAP_WIDTH 2048
#define KRENGINE_SHADOW_MAP_HEIGHT 2048

@interface KREngine : NSObject
{
@private

    GLint backingWidth, backingHeight;
    
    GLuint compositeFramebuffer, compositeDepthTexture, compositeColorTexture;
    
    int m_cShadowBuffers;
    GLuint shadowFramebuffer[KRENGINE_MAX_SHADOW_BUFFERS], shadowDepthTexture[KRENGINE_MAX_SHADOW_BUFFERS];
    bool shadowValid[KRENGINE_MAX_SHADOW_BUFFERS];
    KRMat4 shadowmvpmatrix[KRENGINE_MAX_SHADOW_BUFFERS]; /* MVP Matrix for view from light source */
    
    // uniform index
    enum {
        KRENGINE_UNIFORM_MATERIAL_AMBIENT,
        KRENGINE_UNIFORM_MATERIAL_DIFFUSE,
        KRENGINE_UNIFORM_MATERIAL_SPECULAR,
        KRENGINE_UNIFORM_MVP,
        KRENGINE_UNIFORM_SHADOWMVP1,
        KRENGINE_UNIFORM_SHADOWMVP2,
        KRENGINE_UNIFORM_SHADOWMVP3,
        KRENGINE_UNIFORM_LIGHTDIRECTION,
        KRENGINE_UNIFORM_CAMERAPOS,
        KRENGINE_NUM_UNIFORMS
    };
    GLint m_shadowUniforms[KRENGINE_NUM_UNIFORMS];

    GLuint m_postShaderProgram;
    GLuint m_shadowShaderProgram;
    
    KRTextureManager *m_pTextureManager;
    KRMaterialManager *m_pMaterialManager;
    KRShaderManager *m_pShaderManager;
    KRModelManager *m_pModelManager;
    
    int m_iFrame;

    double sun_pitch, sun_yaw;
    
    KRCamera m_camera;
    NSString *debug_text;
    
}
- (id)initForWidth: (GLuint)width Height: (GLuint)height;

- (BOOL)loadVertexShader:(NSString *)vertexShaderName fragmentShader:(NSString *)fragmentShaderName forProgram:(GLuint *)programPointer withOptions:(NSString *)options;
- (BOOL)loadResource:(NSString *)path;

- (void)renderShadowBufferNumber: (int)iShadow ForScene: (KRScene *)pScene;
- (void)renderScene: (KRScene *)pScene WithViewMatrix: (KRMat4)viewMatrix LightDirection: (Vector3)lightDirection CameraPosition: (Vector3)cameraPosition;
- (KRModelManager *)getModelManager;
- (void)invalidateShadowBuffers;
- (void)allocateShadowBuffers;

- (void)invalidatePostShader;
- (void)bindPostShader;

// Parameter enumeration interface
-(int)getParameterCount;
-(NSString *)getParameterNameWithIndex: (int)i;
-(NSString *)getParameterLabelWithIndex: (int)i;
-(KREngineParameterType)getParameterTypeWithIndex: (int)i;
-(double)getParameterMinWithIndex: (int)i;
-(double)getParameterMaxWithIndex: (int)i;
-(double)getParameterValueWithIndex: (int)i;
-(void)setParameterValueWithIndex: (int)i Value: (double)v;
-(void)setParameterValueWithName: (NSString *)name Value: (double)v;

- (void)renderScene: (KRScene *)pScene WithViewMatrix: (KRMat4)viewMatrix;
- (void)renderScene: (KRScene *)pScene WithPosition: (Vector3)position Yaw: (GLfloat)yaw Pitch: (GLfloat)pitch Roll: (GLfloat)roll;
- (void)setNearZ: (double)dNearZ;
- (void)setFarZ: (double)dFarZ;
- (void)setAspect: (double)dAspect;
- (void)setDebugText: (NSString *)text;

@end

