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

- (id)init
{
    _camera = NULL;
    _context = NULL;
    if ((self = [super init])) {
        _context = new KRContext();
        _camera = new KRCamera(*_context);
        _parameter_names = [@{
            @"camera_fov" : @0,
            @"shadow_quality" : @1,
            @"enable_per_pixel" : @2,
            @"enable_diffuse_map" : @3,
            @"enable_normal_map" : @4,
            @"enable_spec_map" : @5,
            @"enable_reflection_map" : @6,
            @"enable_light_map" : @7,
            @"ambient_temp" : @8,
            @"ambient_intensity" : @9,
            @"sun_temp": @10,
            @"sun_intensity": @11,
            @"dof_quality" : @12,
            @"dof_depth" : @13,
            @"dof_falloff" : @14,
            @"flash_enable" : @15,
            @"flash_intensity" : @16,
            @"flash_depth" : @17,
            @"flash_falloff" : @18,
            @"vignette_enable" : @19,
            @"vignette_radius" : @20,
            @"vignette_falloff" : @21,
            @"debug_shadowmap" : @22,
            @"debug_pssm" : @23,
            @"debug_enable_ambient" : @24,
            @"debug_enable_diffuse" : @25,
            @"debug_enable_specular" : @26,
            @"debug_enable_reflection" : @27,
            @"debug_super_shiny" : @28,
            @"debug_octree" : @29,
            @"debug_deferred" : @30,
            @"enable_deferred_lighting" : @31
        } copy];
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
    _camera->renderFrame(*pScene, viewMatrix);
}

- (BOOL)loadShaders
{
    NSFileManager* fileManager = [NSFileManager defaultManager];
    NSString *bundle_directory = [[NSBundle mainBundle] bundlePath];
    for (NSString* fileName in [fileManager contentsOfDirectoryAtPath: bundle_directory error:nil]) {
        if([fileName hasSuffix: @".vsh"] || [fileName hasSuffix: @".fsh"] || [fileName isEqualToString:@"font.pvr"]) {
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
    [_parameter_names release]; _parameter_names = nil;
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
    return [[self.parameter_names allKeysForObject:[NSNumber numberWithInt:i]] objectAtIndex:0];
}

-(NSString *)getParameterLabelWithIndex: (int)i
{
    NSString *parameter_labels[32] = {
        @"Camera FOV",
        @"Shadow Quality (0 - 2)",
        @"Enable per-pixel lighting",
        @"Enable diffuse map",
        @"Enable normal map",
        @"Enable specular map",
        @"Enable reflection map",
        @"Enable light map",
        @"Ambient Color Temp",
        @"Ambient Intensity",
        @"Sun Color Temp",
        @"Sun Intensity",
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
        @"Debug - Enable Reflections",
        @"Debug - Super Shiny",
        @"Debug - Octree Visualize",
        @"Debug - Deferred Lights Visualize",
        @"Enable Deferred Lighting"
    };
    return parameter_labels[i];
}
-(KREngineParameterType)getParameterTypeWithIndex: (int)i
{
    KREngineParameterType types[32] = {
        
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
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_BOOL
    };
    return types[i];
}
-(double)getParameterValueWithIndex: (int)i
{
    double values[32] = {
        _camera->perspective_fov,
        (double)_camera->m_cShadowBuffers,
        _camera->bEnablePerPixel ? 1.0f : 0.0f,
        _camera->bEnableDiffuseMap ? 1.0f : 0.0f,
        _camera->bEnableNormalMap ? 1.0f : 0.0f,
        _camera->bEnableSpecMap ? 1.0f : 0.0f,
        _camera->bEnableReflectionMap ? 1.0f : 0.0f,
        _camera->bEnableLightMap ? 1.0f : 0.0f,
        [self getAmbientTemperature],
        [self getAmbientIntensity],
        [self getSunTemperature],
        [self getSunIntensity],
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
        _camera->bEnableReflection ? 1.0f : 0.0f,
        _camera->bDebugSuperShiny ? 1.0f : 0.0f,
        _camera->bShowOctree ? 1.0f : 0.0f,
        _camera->bShowDeferred ? 1.0f : 0.0f,
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
            [self setAmbientTemperature:v];
            break;
        case 9:
            [self setAmbientIntensity:v];
            break;
        case 10:
            [self setSunTemperature:v];
            break;
        case 11:
            [self setSunIntensity:v];
            break;
        case 12:
            if(_camera->dof_quality != (int)v) {
                _camera->dof_quality = (int)v;
            }
            break;
        case 13:
            if(_camera->dof_depth != v) {
                _camera->dof_depth = v;
            }
            break;
        case 14:
            if(_camera->dof_falloff != v) {
                _camera->dof_falloff = v;
            }
            break;
        case 15:
            if(_camera->bEnableFlash != bNewBoolVal) {
                _camera->bEnableFlash = bNewBoolVal;
            }
            break;
        case 16:
            if(_camera->flash_intensity != v) {
                _camera->flash_intensity = v;
            }
            break;
        case 17:
            if(_camera->flash_depth != v) {
                _camera->flash_depth = v;
            }
            break;
        case 18:
            if(_camera->flash_falloff != v) {
                _camera->flash_falloff = v;
            }
            break;
        case 19:
            if(_camera->bEnableVignette != bNewBoolVal) {
                _camera->bEnableVignette = bNewBoolVal;
            }
            break;
        case 20:
            if(_camera->vignette_radius != v) {
                _camera->vignette_radius = v;
            }
            break;
        case 21:
            if(_camera->vignette_falloff != v) {
                _camera->vignette_falloff = v;
            }
            break;
        case 22:
            if(_camera->bShowShadowBuffer != bNewBoolVal) {
                _camera->bShowShadowBuffer = bNewBoolVal;
            }
            break;
        case 23:
            if(_camera->bDebugPSSM != bNewBoolVal) {
                _camera->bDebugPSSM = bNewBoolVal;
            }
            break;
        case 24:
            if(_camera->bEnableAmbient != bNewBoolVal) {
                _camera->bEnableAmbient = bNewBoolVal;
            }
            break;
        case 25:
            if(_camera->bEnableDiffuse != bNewBoolVal) {
                _camera->bEnableDiffuse = bNewBoolVal;
            }
            break;
        case 26:
            if(_camera->bEnableSpecular != bNewBoolVal) {
                _camera->bEnableSpecular = bNewBoolVal;
            }
            break;
        case 27:
            if(_camera->bEnableReflection != bNewBoolVal) {
                _camera->bEnableReflection = bNewBoolVal;
            }
            break;
        case 28:
            if(_camera->bDebugSuperShiny != bNewBoolVal) {
                _camera->bDebugSuperShiny = bNewBoolVal;
            }
            break;
        case 29:
            if(_camera->bShowOctree != bNewBoolVal) {
                _camera->bShowOctree = bNewBoolVal;
            }
            break;
        case 30:
            if(_camera->bShowDeferred != bNewBoolVal) {
                _camera->bShowDeferred = bNewBoolVal;
            }
            break;
        case 31:
            if(_camera->bEnableDeferredLighting != bNewBoolVal) {
                _camera->bEnableDeferredLighting = bNewBoolVal;
            }
            break;
    }
}

-(double)getParameterMinWithIndex: (int)i
{
    double minValues[32] = {
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f
    };

    return minValues[i];
}

-(double)getParameterMaxWithIndex: (int)i
{
    double maxValues[32] = {
        PI,   3.0f, 1.0f, 1.0,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 10.0f,
        1.0f, 10.0f, 2.0f, 1.0f, 1.0f, 1.0f, 5.0f, 1.0f,
        0.5f, 1.0f, 2.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f
    };
    
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

-(int)getParameterIndexWithName: (NSString *)name
{
    int cParameters = [self getParameterCount];
    for(int i=0; i < cParameters; i++) {
        if([[self getParameterNameWithIndex:i] caseInsensitiveCompare:name] == NSOrderedSame)
        {
            return i;
        }
    }
    return -1; // not found
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

// ---===--- Sun Temperature and intensity ---===--- 

-(void) setSunTemperature:(double)t
{
    double i = [self getSunIntensity];
    
    _camera->dSunR = (t < 0.5f ? t * 2.0f : 1.0f) * i;
    _camera->dSunG = (t < 0.5f ? t * 2.0f : (1.0f - t) * 2.0f) * i;
    _camera->dSunB = (t < 0.5f ? 1.0f : (1.0f - t) * 2.0f) * i;
}

-(void) setSunIntensity:(double)i
{
    double t = [self getSunTemperature];
    
    _camera->dSunR = (t < 0.5f ? t * 2.0f : 1.0f) * i;
    _camera->dSunG = (t < 0.5f ? t * 2.0f : (1.0f - t) * 2.0f) * i;
    _camera->dSunB = (t < 0.5f ? 1.0f : (1.0f - t) * 2.0f) * i;
}

-(double) getSunIntensity
{
    double i = _camera->dSunR;
    if(_camera->dSunG > i) i = _camera->dSunG;
    if(_camera->dSunB > i) i = _camera->dSunB;
    return i;
}

-(double) getSunTemperature
{
    double i = [self getSunIntensity];
    if(i == 0.0f) return 0.5f; // Avoid division by zero; assume black has a colour temperature of 0.5
    if(_camera->dSunB == i) {
        // Cold side, t < 0.5
        return _camera->dSunR / i * 0.5f;
    } else {
        // Warm side, t > 0.5
        return 1.0f - (_camera->dSunB / i) * 0.5f;
    }
}

// ---===--- Ambient Temperature and intensity ---===--- 

-(void) setAmbientTemperature:(double)t
{
    double i = [self getAmbientIntensity];
    
    _camera->dAmbientR = (t < 0.5f ? t * 2.0f : 1.0f) * i;
    _camera->dAmbientG = (t < 0.5f ? t * 2.0f : (1.0f - t) * 2.0f) * i;
    _camera->dAmbientB = (t < 0.5f ? 1.0f : (1.0f - t) * 2.0f) * i;
}

-(void) setAmbientIntensity:(double)i
{
    double t = [self getAmbientTemperature];
    
    _camera->dAmbientR = (t < 0.5f ? t * 2.0f : 1.0f) * i;
    _camera->dAmbientG = (t < 0.5f ? t * 2.0f : (1.0f - t) * 2.0f) * i;
    _camera->dAmbientB = (t < 0.5f ? 1.0f : (1.0f - t) * 2.0f) * i;
}

-(double) getAmbientIntensity
{
    double i = _camera->dAmbientR;
    if(_camera->dAmbientG > i) i = _camera->dAmbientG;
    if(_camera->dAmbientB > i) i = _camera->dAmbientB;
    return i;
}

-(double) getAmbientTemperature
{
    double i = [self getAmbientIntensity];
    if(i == 0.0f) return 0.5f; // Avoid division by zero; assume black has a colour temperature of 0.5
    if(_camera->dAmbientB == i) {
        // Cold side, t < 0.5
        return _camera->dAmbientR / i * 0.5f;
    } else {
        // Warm side, t > 0.5
        return 1.0f - (_camera->dAmbientB / i) * 0.5f;
    }
}


@end
