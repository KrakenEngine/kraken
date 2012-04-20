//
//  KREngine.h
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

// #import "KRTextureManager.h"
#import <map>
#import "KRMat4.h"
#import "KRVector3.h"
#import "KRModel.h"
#import "KRScene.h"
#import "KRContext.h"
#import "KRCamera.h"

#import "KREngine-common.h"

typedef enum KREngineParameterType {KRENGINE_PARAMETER_INT, KRENGINE_PARAMETER_FLOAT, KRENGINE_PARAMETER_BOOL} KREngineParameterType;

#define KRENGINE_MAX_SHADOW_BUFFERS 3
#define KRENGINE_SHADOW_MAP_WIDTH 2048
#define KRENGINE_SHADOW_MAP_HEIGHT 2048

@interface KREngine : NSObject
{
@private

    GLint backingWidth, backingHeight;
    
    GLuint compositeFramebuffer, compositeDepthTexture, compositeColorTexture;
    GLuint lightAccumulationBuffer, lightAccumulationTexture;
    
    int m_cShadowBuffers;
    GLuint shadowFramebuffer[KRENGINE_MAX_SHADOW_BUFFERS], shadowDepthTexture[KRENGINE_MAX_SHADOW_BUFFERS];
    bool shadowValid[KRENGINE_MAX_SHADOW_BUFFERS];
    KRMat4 shadowmvpmatrix[KRENGINE_MAX_SHADOW_BUFFERS]; /* MVP Matrix for view from light source */
    
    // uniform index
    enum {
        KRENGINE_UNIFORM_MATERIAL_AMBIENT,
        KRENGINE_UNIFORM_MATERIAL_DIFFUSE,
        KRENGINE_UNIFORM_MATERIAL_SPECULAR,
        KRENGINE_UNIFORM_LIGHT_COLOR,
        KRENGINE_UNIFORM_MVP,
        KRENGINE_UNIFORM_M2V,
        KRENGINE_UNIFORM_SHADOWMVP1,
        KRENGINE_UNIFORM_SHADOWMVP2,
        KRENGINE_UNIFORM_SHADOWMVP3,
        KRENGINE_UNIFORM_LIGHTDIRECTION,
        KRENGINE_UNIFORM_CAMERAPOS,
        KRENGINE_UNIFORM_VIEWPORT,
        KRENGINE_NUM_UNIFORMS
    };
    GLint m_shadowUniforms[KRENGINE_NUM_UNIFORMS];

    GLuint m_postShaderProgram;
    GLuint m_shadowShaderProgram;
    
    KRContext *m_pContext;
    
    int m_iFrame;

    double sun_pitch, sun_yaw;
    
    KRCamera m_camera;
    NSString *debug_text;
    
}

@property(nonatomic, readonly) KRContext *context;

- (id)initForWidth: (GLuint)width Height: (GLuint)height;

- (BOOL)loadVertexShader:(NSString *)vertexShaderName fragmentShader:(NSString *)fragmentShaderName forProgram:(GLuint *)programPointer withOptions:(NSString *)options;
- (BOOL)loadResource:(NSString *)path;

- (void)renderShadowBufferNumber: (int)iShadow ForScene: (KRScene *)pScene;
- (void)renderScene: (KRScene *)pScene WithViewMatrix: (KRMat4)viewMatrix LightDirection: (KRVector3)lightDirection CameraPosition: (KRVector3)cameraPosition;
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
- (void)renderScene: (KRScene *)pScene WithPosition: (KRVector3)position Yaw: (GLfloat)yaw Pitch: (GLfloat)pitch Roll: (GLfloat)roll;
- (void)setNearZ: (double)dNearZ;
- (void)setFarZ: (double)dFarZ;
- (void)setDebugText: (NSString *)text;

@end

