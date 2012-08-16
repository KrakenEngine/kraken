//
//  KREngine.mm
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

#import "KREngine.h"
#import "KRVector3.h"
#import "KRScene.h"
#import "KRSceneManager.h"
#import "KRNode.h"

#import <string>
#import <sstream> 

using namespace std;



@interface KREngine (PrivateMethods)
- (BOOL)loadShaders;
- (BOOL)loadResource:(NSString *)path;
@end

@implementation KREngine
@synthesize debug_text = _debug_text;
double const PI = 3.141592653589793f;

- (id)initForWidth: (GLuint)width Height: (GLuint)height
{
    _camera = NULL;
    _context = NULL;
    if ((self = [super init])) {
        _context = new KRContext();
        _camera = new KRCamera(*_context, width, height);
        [self loadShaders];
        
    }
    
    return self;
}

- (void)renderScene: (KRScene *)pScene WithPosition: (KRVector3)position Yaw: (GLfloat)yaw Pitch: (GLfloat)pitch Roll: (GLfloat)roll
{
    KRMat4 viewMatrix;
    viewMatrix.translate(-position.x, -position.y, -position.z);
    viewMatrix.rotate(yaw, Y_AXIS);
    viewMatrix.rotate(pitch, X_AXIS);
    viewMatrix.rotate(roll, Z_AXIS);
    
    [self renderScene: pScene WithViewMatrix: viewMatrix];
}

- (void)renderScene: (KRScene *)pScene WithViewMatrix: (KRMat4)viewMatrix
{
    viewMatrix.rotate(-90 * 0.0174532925199, Z_AXIS);
    _camera->renderFrame(*_context, *pScene, viewMatrix);

}

- (BOOL)loadShaders
{
    NSFileManager* fileManager = [NSFileManager defaultManager];
    NSString *bundle_directory = [[NSBundle mainBundle] bundlePath];
    for (NSString* fileName in [fileManager contentsOfDirectoryAtPath: bundle_directory error:nil]) {
        if([fileName hasSuffix: @".vsh"] || [fileName hasSuffix: @".fsh"]) {
            NSString* path = [NSString stringWithFormat:@"%@/%@", bundle_directory, fileName];
            _context->loadResource([path UTF8String]);
        }
    }

    return TRUE;
}

- (BOOL)loadResource:(NSString *)path
{
    _context->loadResource([path UTF8String]);
    
    return TRUE;
}

- (void)dealloc
{
    if(_camera) {
        delete _camera; _camera = NULL;
    }
    if(_context) {
        delete _context; _context = NULL;
    }
    [super dealloc];
}

-(int)getParameterCount
{
    return 31;
}

-(NSString *)getParameterNameWithIndex: (int)i
{
    NSString *parameter_names[31] = {
        @"camera_fov",
        @"shadow_quality",
        @"enable_per_pixel",
        @"enable_diffuse_map",
        @"enable_normal_map",
        @"enable_spec_map",
        @"enable_reflection_map",
        @"enable_light_map",
        @"ambient_r",
        @"ambient_g",
        @"ambient_b",
        @"sun_r",
        @"sun_g",
        @"sun_b",
        @"dof_quality",
        @"dof_depth",
        @"dof_falloff",
        @"flash_enable",
        @"flash_intensity",
        @"flash_depth",
        @"flash_falloff",
        @"vignette_enable",
        @"vignette_radius",
        @"vignette_falloff",
        @"debug_shadowmap",
        @"debug_pssm",
        @"debug_enable_ambient",
        @"debug_enable_diffuse",
        @"debug_enable_specular",
        @"debug_super_shiny",
        @"enable_deferred_lighting"
    };
    return parameter_names[i];
}
-(NSString *)getParameterLabelWithIndex: (int)i
{
    NSString *parameter_labels[31] = {
        @"Camera FOV",
        @"Shadow Quality (0 - 2)",
        @"Enable per-pixel lighting",
        @"Enable diffuse map",
        @"Enable normal map",
        @"Enable specular map",
        @"Enable reflection map",
        @"Enable light map",
        @"Ambient light red intensity",
        @"Ambient light green intensity",
        @"Ambient light blue intensity",
        @"Sun red intensity",
        @"Sun green intensity",
        @"Sun blue intensity",
        @"DOF Quality",
        @"DOF Depth",
        @"DOF Falloff",
        @"Enable Night/Flash Effect",
        @"Flash Intensity",
        @"Flash Depth",
        @"Flash Falloff",
        @"Enable Vignette",
        @"Vignette Radius",
        @"Vignette Falloff",
        @"Debug - View Shadow Volume",
        @"Debug - PSSM",
        @"Debug - Enable Ambient",
        @"Debug - Enable Diffuse",
        @"Debug - Enable Specular",
        @"Debug - Super Shiny",
        @"Enable Deferred Lighting"
    };
    return parameter_labels[i];
}
-(KREngineParameterType)getParameterTypeWithIndex: (int)i
{
    KREngineParameterType types[31] = {
        
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_INT,
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_INT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_BOOL
    };
    return types[i];
}
-(double)getParameterValueWithIndex: (int)i
{
    double values[31] = {
        _camera->perspective_fov,
        (double)_camera->m_cShadowBuffers,
        _camera->bEnablePerPixel ? 1.0f : 0.0f,
        _camera->bEnableDiffuseMap ? 1.0f : 0.0f,
        _camera->bEnableNormalMap ? 1.0f : 0.0f,
        _camera->bEnableSpecMap ? 1.0f : 0.0f,
        _camera->bEnableReflectionMap ? 1.0f : 0.0f,
        _camera->bEnableLightMap ? 1.0f : 0.0f,
        _camera->dAmbientR,
        _camera->dAmbientG,
        _camera->dAmbientB,
        _camera->dSunR,
        _camera->dSunG,
        _camera->dSunB,
        _camera->dof_quality,
        _camera->dof_depth,
        _camera->dof_falloff,
        _camera->bEnableFlash ? 1.0f : 0.0f,
        _camera->flash_intensity,
        _camera->flash_depth,
        _camera->flash_falloff,
        _camera->bEnableVignette ? 1.0f : 0.0f,
        _camera->vignette_radius,
        _camera->vignette_falloff,
        _camera->bShowShadowBuffer ? 1.0f : 0.0f,
        _camera->bDebugPSSM ? 1.0f : 0.0f,
        _camera->bEnableAmbient ? 1.0f : 0.0f,
        _camera->bEnableDiffuse ? 1.0f : 0.0f,
        _camera->bEnableSpecular ? 1.0f : 0.0f,
        _camera->bDebugSuperShiny ? 1.0f : 0.0f,
        _camera->bEnableDeferredLighting ? 1.0f : 0.0f
    };
    return values[i];
}
-(void)setParameterValueWithIndex: (int)i Value: (double)v
{
    bool bNewBoolVal = v > 0.5;
    NSLog(@"Set Parameter: (%s, %f)", [[self getParameterNameWithIndex: i] UTF8String], v);
    switch(i) {
        case 0: // FOV
            _camera->perspective_fov = v;
            break;
        case 1: // Shadow Quality
            _camera->m_cShadowBuffers = (int)v;
            break;
        case 2:
            _camera->bEnablePerPixel = bNewBoolVal;
            break;
        case 3:
            _camera->bEnableDiffuseMap = bNewBoolVal;
            break;
        case 4:
            _camera->bEnableNormalMap = bNewBoolVal;
            break;
        case 5:
            _camera->bEnableSpecMap = bNewBoolVal;
            break;
        case 6:
            _camera->bEnableReflectionMap = bNewBoolVal;
            break;
        case 7:
            _camera->bEnableLightMap = bNewBoolVal;
            break;
        case 8:
            _camera->dAmbientR = v;
            break;
        case 9:
            _camera->dAmbientG = v;
            break;
        case 10:
            _camera->dAmbientB = v;
            break;
        case 11:
            _camera->dSunR = v;
            break;
        case 12:
            _camera->dSunG = v;
            break;
        case 13:
            _camera->dSunB = v;
            break;
        case 14:
            if(_camera->dof_quality != (int)v) {
                _camera->dof_quality = (int)v;
                _camera->invalidatePostShader();
            }
            break;
        case 15:
            if(_camera->dof_depth != v) {
                _camera->dof_depth = v;
                _camera->invalidatePostShader();
            }
            break;
        case 16:
            if(_camera->dof_falloff != v) {
                _camera->dof_falloff = v;
                _camera->invalidatePostShader();
            }
            break;
        case 17:
            if(_camera->bEnableFlash != bNewBoolVal) {
                _camera->bEnableFlash = bNewBoolVal;
                _camera->invalidatePostShader();
            }
            break;
        case 18:
            if(_camera->flash_intensity != v) {
                _camera->flash_intensity = v;
                _camera->invalidatePostShader();
            }
            break;
        case 19:
            if(_camera->flash_depth != v) {
                _camera->flash_depth = v;
                _camera->invalidatePostShader();
            }
            break;
        case 20:
            if(_camera->flash_falloff != v) {
                _camera->flash_falloff = v;
                _camera->invalidatePostShader();
            }
            break;
        case 21:
            if(_camera->bEnableVignette != bNewBoolVal) {
                _camera->bEnableVignette = bNewBoolVal;
                _camera->invalidatePostShader();
            }
            break;
        case 22:
            if(_camera->vignette_radius != v) {
                _camera->vignette_radius = v;
                _camera->invalidatePostShader();
            }
            break;
        case 23:
            if(_camera->vignette_falloff != v) {
                _camera->vignette_falloff = v;
                _camera->invalidatePostShader();
            }
            break;
        case 24:
            if(_camera->bShowShadowBuffer != bNewBoolVal) {
                _camera->bShowShadowBuffer = bNewBoolVal;
            }
            break;
        case 25:
            if(_camera->bDebugPSSM != bNewBoolVal) {
                _camera->bDebugPSSM = bNewBoolVal;
            }
            break;
        case 26:
            if(_camera->bEnableAmbient != bNewBoolVal) {
                _camera->bEnableAmbient = bNewBoolVal;
            }
            break;
        case 27:
            if(_camera->bEnableDiffuse != bNewBoolVal) {
                _camera->bEnableDiffuse = bNewBoolVal;
            }
            break;
        case 28:
            if(_camera->bEnableSpecular != bNewBoolVal) {
                _camera->bEnableSpecular = bNewBoolVal;
            }
            break;
        case 29:
            if(_camera->bDebugSuperShiny != bNewBoolVal) {
                _camera->bDebugSuperShiny = bNewBoolVal;
            }
            break;
        case 30:
            if(_camera->bEnableDeferredLighting != bNewBoolVal) {
                _camera->bEnableDeferredLighting = bNewBoolVal;
            }
    }
}

-(double)getParameterMinWithIndex: (int)i
{
    double minValues[31] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    return minValues[i];
}

-(double)getParameterMaxWithIndex: (int)i
{
    double maxValues[31] = {PI, 3.0f, 1.0f, 1.0, 1.0f, 1.0f, 1.0f, 1.0f, 3.0f, 3.0f, 3.0f, 3.0f, 3.0f, 3.0f, 2.0f, 1.0f, 1.0f, 1.0f, 5.0f, 1.0f, 0.5f, 1.0f, 2.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    return maxValues[i];
}

-(void)setParameterValueWithName: (NSString *)name Value: (double)v
{
    int cParameters = [self getParameterCount];
    for(int i=0; i < cParameters; i++) {
        if([[self getParameterNameWithIndex: i] caseInsensitiveCompare:name] == NSOrderedSame) {
            [self setParameterValueWithIndex:i Value:v];
        }
    }
}

- (void)setNearZ: (double)dNearZ
{
    if(_camera->perspective_nearz != dNearZ) {
        _camera->perspective_nearz = dNearZ;
        _camera->invalidateShadowBuffers();
    }
}
- (void)setFarZ: (double)dFarZ
{
    if(_camera->perspective_farz != dFarZ) {
        _camera->perspective_farz = dFarZ;
        _camera->invalidateShadowBuffers();
    }
}

- (void)setDebug_text:(NSString *)value
{
    [_debug_text release];
    _debug_text = value;
    [_debug_text retain];
    
    _camera->m_debug_text = value.UTF8String;
}

@end
