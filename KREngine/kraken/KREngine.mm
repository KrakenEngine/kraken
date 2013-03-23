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

#include "KREngine-common.h"

#include "KREngine.h"
#include "KRVector3.h"
#include "KRScene.h"
#include "KRSceneManager.h"
#include "KRNode.h"

using namespace std;

// Temporary wrapper function, until the KREngine class is refactored into the C++ codebase
void kraken::set_parameter(const std::string &parameter_name, float parameter_value)
{
    [[KREngine sharedInstance] setParameterValueWithName: [NSString stringWithUTF8String:parameter_name.c_str()] Value:parameter_value];
}


@interface KREngine() {
    KRRenderSettings _settings;
}
- (BOOL)loadShaders;
- (BOOL)loadResource:(NSString *)path;
@end

@implementation KREngine
@synthesize debug_text = _debug_text;

+ (KREngine *)sharedInstance
{
    static KREngine *sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[KREngine alloc] init];
    });
    return sharedInstance;
}

- (id)init
{

#if TARGET_OS_IPHONE
    BOOL isIpad = UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad;
    BOOL isRetina = [[UIScreen mainScreen] scale] >= 2.0;
    if(isIpad && isRetina) {
        KRContext::KRENGINE_MAX_VBO_HANDLES = 10000;
        KRContext::KRENGINE_MAX_VBO_MEM = 128000000 * 2;
        KRContext::KRENGINE_MAX_SHADER_HANDLES = 100;
        KRContext::KRENGINE_MAX_TEXTURE_HANDLES = 10000;
        KRContext::KRENGINE_MAX_TEXTURE_MEM = 64000000 * 2;
        KRContext::KRENGINE_TARGET_TEXTURE_MEM_MAX = 48000000 * 2;
        KRContext::KRENGINE_TARGET_TEXTURE_MEM_MIN = 32000000 * 2;
        KRContext::KRENGINE_MAX_TEXTURE_DIM = 2048;
        KRContext::KRENGINE_MIN_TEXTURE_DIM = 64;
        KRContext::KRENGINE_MAX_TEXTURE_THROUGHPUT = 32000000;
    } else {
        KRContext::KRENGINE_MAX_VBO_HANDLES = 10000;
        KRContext::KRENGINE_MAX_VBO_MEM = 128000000;
        KRContext::KRENGINE_MAX_SHADER_HANDLES = 100;
        KRContext::KRENGINE_MAX_TEXTURE_HANDLES = 10000;
        KRContext::KRENGINE_MAX_TEXTURE_MEM = 64000000;
        KRContext::KRENGINE_TARGET_TEXTURE_MEM_MAX = 48000000;
        KRContext::KRENGINE_TARGET_TEXTURE_MEM_MIN = 32000000;
        KRContext::KRENGINE_MAX_TEXTURE_DIM = 2048;
        KRContext::KRENGINE_MIN_TEXTURE_DIM = 64;
        KRContext::KRENGINE_MAX_TEXTURE_THROUGHPUT = 32000000;
    }
#else
    KRContext::KRENGINE_MAX_VBO_HANDLES = 10000;
    KRContext::KRENGINE_MAX_VBO_MEM = 256000000;
    KRContext::KRENGINE_MAX_SHADER_HANDLES = 100;
    KRContext::KRENGINE_MAX_TEXTURE_HANDLES = 10000;
    KRContext::KRENGINE_MAX_TEXTURE_MEM = 512000000;
    KRContext::KRENGINE_TARGET_TEXTURE_MEM_MAX = 384000000;
    KRContext::KRENGINE_TARGET_TEXTURE_MEM_MIN =  256000000;
    KRContext::KRENGINE_MAX_TEXTURE_DIM = 2048;
    KRContext::KRENGINE_MIN_TEXTURE_DIM = 64;
    KRContext::KRENGINE_MAX_TEXTURE_THROUGHPUT = 128000000;
#endif
    
    _context = NULL;
    if ((self = [super init])) {
        _context = new KRContext();
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
            @"enable_deferred_lighting" : @31,
            @"near_clip" : @32,
            @"far_clip" : @33,
            @"volumetric_environment_enable" : @34,
            @"volumetric_environment_downsample" : @35,
            @"volumetric_environment_max_distance" : @36,
            @"volumetric_environment_slices" : @37,
            @"volumetric_environment_intensity" : @38,
            @"fog_type": @39,
            @"fog_near": @40,
            @"fog_far": @41,
            @"fog_density": @42,
            @"fog_color_r": @43,
            @"fog_color_g": @44,
            @"fog_color_b": @45,
            @"dust_enable" : @46,
            @"dust_intensity" : @47,
            @"debug_display" : @48
                            
        } copy];
        [self loadShaders];
        
    }
    
    return self;
}

- (void)renderScene: (KRScene *)pScene WithDeltaTime: (float)deltaTime AndWidth: (int)width AndHeight: (int)height
{
    KRCamera *camera = pScene->find<KRCamera>();
    if(camera) {
        camera->settings = _settings;
    }
    pScene->renderFrame(deltaTime, width, height);
}

- (void)renderScene: (KRScene *)pScene WithDeltaTime: (float)deltaTime
{    
    GLint renderBufferWidth = 0, renderBufferHeight = 0;
    GLDEBUG(glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &renderBufferWidth));
    GLDEBUG(glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &renderBufferHeight));
    [self renderScene:pScene WithDeltaTime:deltaTime AndWidth:renderBufferWidth AndHeight:renderBufferHeight];
}

- (BOOL)loadShaders
{
#if TARGET_OS_IPHONE
    NSString *bundleName = @"kraken_standard_assets_ios";
#else
    NSString *bundleName = @"kraken_standard_assets_osx";
#endif

//    NSString *bundlePath = [[[NSBundle mainBundle] bundlePath] stringByAppendingPathComponent:bundleName];
    NSString *bundlePath = [[NSBundle mainBundle] pathForResource:bundleName ofType:@"bundle"];
    NSBundle *bundle = [NSBundle bundleWithPath:bundlePath];
    if(bundle == nil) {
        NSLog(@"ERROR - Standard asset bundle could not be found.");
    } else {
        NSEnumerator *bundleEnumerator = [[bundle pathsForResourcesOfType: nil inDirectory: nil] objectEnumerator];
        NSString * p = nil;
        while (p = [bundleEnumerator nextObject]) {
            NSString *file_name = [p lastPathComponent];
            if([file_name hasSuffix: @".vsh"] || [file_name hasSuffix: @".fsh"] || [file_name hasSuffix: @".krbundle"] ||[file_name hasPrefix:@"font."]) {
                NSLog(@"  %@\n", file_name);
                [self loadResource:p];
            }
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
    if(_context) {
        delete _context; _context = NULL;
    }
    [super dealloc];
}

-(int)getParameterCount
{
    return 49;
}

-(NSString *)getParameterNameWithIndex: (int)i
{
    return [[self.parameter_names allKeysForObject:[NSNumber numberWithInt:i]] objectAtIndex:0];
}

-(NSString *)getParameterLabelWithIndex: (int)i
{
    NSString *parameter_labels[49] = {
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
        @"Enable Deferred Lighting",
        @"Clip Plane - Near",
        @"Clip Plane - Far",
        @"Volumetric Env. - Enabled",
        @"Volumetric Env. - Resolution",
        @"Volumetric Env. - Maximum Distance",
        @"Volumetric Env. - Quality",
        @"Volumetric Env. - Intensity",
        @"Fog - Type",
        @"Fog - Near",
        @"Fog - Far",
        @"Fog - Density",
        @"Fog - Color R",
        @"Fog - Color G",
        @"Fog - Color B",
        @"Dust - Enable",
        @"Dust - Intensity",
        @"Debug - Display"
    };
    return parameter_labels[i];
}
-(KREngineParameterType)getParameterTypeWithIndex: (int)i
{
    KREngineParameterType types[49] = {
        
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
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_INT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_INT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_INT
    };
    return types[i];
}
-(float)getParameterValueWithIndex: (int)i
{
    float values[49] = {
        _settings.perspective_fov,
        (float)_settings.m_cShadowBuffers,
        _settings.bEnablePerPixel ? 1.0f : 0.0f,
        _settings.bEnableDiffuseMap ? 1.0f : 0.0f,
        _settings.bEnableNormalMap ? 1.0f : 0.0f,
        _settings.bEnableSpecMap ? 1.0f : 0.0f,
        _settings.bEnableReflectionMap ? 1.0f : 0.0f,
        _settings.bEnableLightMap ? 1.0f : 0.0f,
        [self getAmbientTemperature],
        [self getAmbientIntensity],
        [self getSunTemperature],
        [self getSunIntensity],
        _settings.dof_quality,
        _settings.dof_depth,
        _settings.dof_falloff,
        _settings.bEnableFlash ? 1.0f : 0.0f,
        _settings.flash_intensity,
        _settings.flash_depth,
        _settings.flash_falloff,
        _settings.bEnableVignette ? 1.0f : 0.0f,
        _settings.vignette_radius,
        _settings.vignette_falloff,
        _settings.bShowShadowBuffer ? 1.0f : 0.0f,
        _settings.bDebugPSSM ? 1.0f : 0.0f,
        _settings.bEnableAmbient ? 1.0f : 0.0f,
        _settings.bEnableDiffuse ? 1.0f : 0.0f,
        _settings.bEnableSpecular ? 1.0f : 0.0f,
        _settings.bEnableReflection ? 1.0f : 0.0f,
        _settings.bDebugSuperShiny ? 1.0f : 0.0f,
        _settings.bShowOctree ? 1.0f : 0.0f,
        _settings.bShowDeferred ? 1.0f : 0.0f,
        _settings.bEnableDeferredLighting ? 1.0f : 0.0f,
        _settings.getPerspectiveNearZ(),
        _settings.getPerspectiveFarZ(),
        _settings.volumetric_environment_enable,
        5 - _settings.volumetric_environment_downsample,
        _settings.volumetric_environment_max_distance,
        _settings.volumetric_environment_quality,
        _settings.volumetric_environment_intensity,
        _settings.fog_type,
        _settings.fog_near,
        _settings.fog_far,
        _settings.fog_density,
        _settings.fog_color.x,
        _settings.fog_color.y,
        _settings.fog_color.z,
        _settings.dust_particle_enable,
        _settings.dust_particle_intensity,
        _settings.debug_display
    };
    return values[i];
}
-(void)setParameterValueWithIndex: (int)i Value: (float)v
{
    bool bNewBoolVal = v > 0.5;
//    NSLog(@"Set Parameter: (%s, %f)", [[self getParameterNameWithIndex: i] UTF8String], v);
    switch(i) {
        case 0: // FOV
            _settings.perspective_fov = v;
            break;
        case 1: // Shadow Quality
            _settings.m_cShadowBuffers = (int)v;
            break;
        case 2:
            _settings.bEnablePerPixel = bNewBoolVal;
            break;
        case 3:
            _settings.bEnableDiffuseMap = bNewBoolVal;
            break;
        case 4:
            _settings.bEnableNormalMap = bNewBoolVal;
            break;
        case 5:
            _settings.bEnableSpecMap = bNewBoolVal;
            break;
        case 6:
            _settings.bEnableReflectionMap = bNewBoolVal;
            break;
        case 7:
            _settings.bEnableLightMap = bNewBoolVal;
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
            if(_settings.dof_quality != (int)v) {
                _settings.dof_quality = (int)v;
            }
            break;
        case 13:
            if(_settings.dof_depth != v) {
                _settings.dof_depth = v;
            }
            break;
        case 14:
            if(_settings.dof_falloff != v) {
                _settings.dof_falloff = v;
            }
            break;
        case 15:
            if(_settings.bEnableFlash != bNewBoolVal) {
                _settings.bEnableFlash = bNewBoolVal;
            }
            break;
        case 16:
            if(_settings.flash_intensity != v) {
                _settings.flash_intensity = v;
            }
            break;
        case 17:
            if(_settings.flash_depth != v) {
                _settings.flash_depth = v;
            }
            break;
        case 18:
            if(_settings.flash_falloff != v) {
                _settings.flash_falloff = v;
            }
            break;
        case 19:
            if(_settings.bEnableVignette != bNewBoolVal) {
                _settings.bEnableVignette = bNewBoolVal;
            }
            break;
        case 20:
            if(_settings.vignette_radius != v) {
                _settings.vignette_radius = v;
            }
            break;
        case 21:
            if(_settings.vignette_falloff != v) {
                _settings.vignette_falloff = v;
            }
            break;
        case 22:
            if(_settings.bShowShadowBuffer != bNewBoolVal) {
                _settings.bShowShadowBuffer = bNewBoolVal;
            }
            break;
        case 23:
            if(_settings.bDebugPSSM != bNewBoolVal) {
                _settings.bDebugPSSM = bNewBoolVal;
            }
            break;
        case 24:
            if(_settings.bEnableAmbient != bNewBoolVal) {
                _settings.bEnableAmbient = bNewBoolVal;
            }
            break;
        case 25:
            if(_settings.bEnableDiffuse != bNewBoolVal) {
                _settings.bEnableDiffuse = bNewBoolVal;
            }
            break;
        case 26:
            if(_settings.bEnableSpecular != bNewBoolVal) {
                _settings.bEnableSpecular = bNewBoolVal;
            }
            break;
        case 27:
            if(_settings.bEnableReflection != bNewBoolVal) {
                _settings.bEnableReflection = bNewBoolVal;
            }
            break;
        case 28:
            if(_settings.bDebugSuperShiny != bNewBoolVal) {
                _settings.bDebugSuperShiny = bNewBoolVal;
            }
            break;
        case 29:
            if(_settings.bShowOctree != bNewBoolVal) {
                _settings.bShowOctree = bNewBoolVal;
            }
            break;
        case 30:
            if(_settings.bShowDeferred != bNewBoolVal) {
                _settings.bShowDeferred = bNewBoolVal;
            }
            break;
        case 31:
            if(_settings.bEnableDeferredLighting != bNewBoolVal) {
                _settings.bEnableDeferredLighting = bNewBoolVal;
            }
            break;
        case 32:
            _settings.setPerspectiveNear(v);
            break;
        case 33:
            _settings.setPerpsectiveFarZ(v);
            break;
        case 34:
            _settings.volumetric_environment_enable = bNewBoolVal;
            break;
        case 35:
            _settings.volumetric_environment_downsample = 5 - (int)v;
            break;
        case 36:
            _settings.volumetric_environment_max_distance = v;
            break;
        case 37:
            _settings.volumetric_environment_quality = v;
            break;
        case 38:
            _settings.volumetric_environment_intensity = v;
            break;
        case 39:
            _settings.fog_type = v;
            break;
        case 40:
            _settings.fog_near = v;
            break;
        case 41:
            _settings.fog_far = v;
            break;
        case 42:
            _settings.fog_density = v;
            break;
        case 43:
            _settings.fog_color.x = v;
            break;
        case 44:
            _settings.fog_color.y = v;
            break;
        case 45:
            _settings.fog_color.z = v;
            break;
        case 46:
            _settings.dust_particle_enable = bNewBoolVal;
            break;
        case 47:
            _settings.dust_particle_intensity = v;
            break;
        case 48:
            _settings.debug_display = (KRRenderSettings::debug_display_type)v;
            break;
    }
}

-(float)getParameterMinWithIndex: (int)i
{
    float minValues[49] = {
        0.0f, 0.0f, 0.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.01f, 50.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f
    };

    return minValues[i];
}

-(float)getParameterMaxWithIndex: (int)i
{
    float maxValues[49] = {
             PI,    3.0f,     1.0f,    1.0,  1.0f, 1.0f,    1.0f, 1.0f, 1.0f, 10.0f,
           1.0f,   10.0f,    2.0f,     1.0f, 1.0f, 1.0f,    5.0f, 1.0f, 0.5f,  1.0f,
           2.0f,    2.0f,    1.0f,     1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f,  1.0f,
           1.0f,    1.0f,   10.0f, 1000.0f,  1.0f, 5.0f, 1000.0f, 1.0f, 5.0f,  3.0f,
        1000.0f, 1000.0f,    0.01f,    1.0f, 1.0f, 1.0f,    1.0f, 1.0f, (float)(KRRenderSettings::KRENGINE_DEBUG_DISPLAY_NUMBER - 1)
    };
    
    return maxValues[i];
}

-(void)setParameterValueWithName: (NSString *)name Value: (float)v
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

- (void)setNearZ: (float)dNearZ
{
    _settings.setPerspectiveNear(dNearZ);
}
- (void)setFarZ: (float)dFarZ
{
    _settings.setPerpsectiveFarZ(dFarZ);
}

- (void)setDebug_text:(NSString *)value
{
    [_debug_text release];
    _debug_text = value;
    [_debug_text retain];
    
    _settings.m_debug_text = value.UTF8String;
}

// ---===--- Sun Temperature and intensity ---===--- 

-(void) setSunTemperature:(float)t
{
    float i = [self getSunIntensity];
    _settings.light_intensity = KRVector3(
        (t < 0.5f ? t * 2.0f : 1.0f) * i,
        (t < 0.5f ? t * 2.0f : (1.0f - t) * 2.0f) * i,
        (t < 0.5f ? 1.0f : (1.0f - t) * 2.0f) * i
    );
}

-(void) setSunIntensity:(float)i
{
    float t = [self getSunTemperature];
    _settings.light_intensity = KRVector3(
        (t < 0.5f ? t * 2.0f : 1.0f) * i,
        (t < 0.5f ? t * 2.0f : (1.0f - t) * 2.0f) * i,
        (t < 0.5f ? 1.0f : (1.0f - t) * 2.0f) * i
    );
}

-(float) getSunIntensity
{
    float i = _settings.light_intensity[0];
    if(_settings.light_intensity[1] > i) i = _settings.light_intensity[1];
    if(_settings.light_intensity[2] > i) i = _settings.light_intensity[2];
    return i;
}

-(float) getSunTemperature
{
    float i = [self getSunIntensity];
    if(i == 0.0f) return 0.5f; // Avoid division by zero; assume black has a colour temperature of 0.5
    if(_settings.light_intensity[2] == i) {
        // Cold side, t < 0.5
        return _settings.light_intensity[0] / i * 0.5f;
    } else {
        // Warm side, t > 0.5
        return 1.0f - (_settings.light_intensity[2] / i) * 0.5f;
    }
}

// ---===--- Ambient Temperature and intensity ---===--- 

-(void) setAmbientTemperature:(float)t
{
    float i = [self getAmbientIntensity];
    _settings.ambient_intensity = KRVector3(
        (t < 0.5f ? t * 2.0f : 1.0f) * i,
        (t < 0.5f ? t * 2.0f : (1.0f - t) * 2.0f) * i,
        (t < 0.5f ? 1.0f : (1.0f - t) * 2.0f) * i
    );
}

-(void) setAmbientIntensity:(float)i
{
    float t = [self getAmbientTemperature];
    _settings.ambient_intensity = KRVector3(
        (t < 0.5f ? t * 2.0f : 1.0f) * i,
        (t < 0.5f ? t * 2.0f : (1.0f - t) * 2.0f) * i,
        (t < 0.5f ? 1.0f : (1.0f - t) * 2.0f) * i
    );
}

-(float) getAmbientIntensity
{
    float i = _settings.ambient_intensity[0];
    if(_settings.ambient_intensity[1] > i) i = _settings.ambient_intensity[1];
    if(_settings.ambient_intensity[2] > i) i = _settings.ambient_intensity[2];
    return i;
}

-(float) getAmbientTemperature
{
    float i = [self getAmbientIntensity];
    if(i == 0.0f) return 0.5f; // Avoid division by zero; assume black has a colour temperature of 0.5
    if(_settings.ambient_intensity[2] == i) {
        // Cold side, t < 0.5
        return _settings.ambient_intensity[0] / i * 0.5f;
    } else {
        // Warm side, t > 0.5
        return 1.0f - (_settings.ambient_intensity[2] / i) * 0.5f;
    }
}


@end
