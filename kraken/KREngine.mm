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
#include "KRScene.h"
#include "KRSceneManager.h"
#include "KRNode.h"

using namespace std;

// Temporary wrapper function, until the KREngine class is refactored into the C++ codebase
void kraken::set_parameter(const std::string &parameter_name, float parameter_value)
{
    [[KREngine sharedInstance] setParameterValueWithName: [NSString stringWithUTF8String:parameter_name.c_str()] Value:parameter_value];
}

void kraken::set_debug_text(const std::string &print_text)
{
    [KREngine sharedInstance].settings->m_debug_text = print_text;
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
    mach_port_t host_port = mach_host_self();
    mach_msg_type_number_t host_size = sizeof(vm_statistics_data_t) / sizeof(integer_t);
    vm_size_t pagesize = 0;
    vm_statistics_data_t vm_stat;
    int total_ram = 256 * 1024 * 1024;
    if(host_page_size(host_port, &pagesize) != KERN_SUCCESS) {
        fprintf(stderr, "ERROR: Could not get VM page size.\n");
    } else if(host_statistics(host_port, HOST_VM_INFO, (host_info_t)&vm_stat, &host_size) != KERN_SUCCESS) {
        fprintf(stderr, "ERROR: Could not get VM stats.\n");
    } else {
        total_ram = (vm_stat.wire_count + vm_stat.active_count + vm_stat.inactive_count + vm_stat.free_count) * pagesize;
    }
    
    
    KRContext::KRENGINE_MAX_TEXTURE_DIM = 2048;
    KRContext::KRENGINE_MIN_TEXTURE_DIM = 64;
    KRContext::KRENGINE_PRESTREAM_DISTANCE = 1000.0f;
    

    KRContext::KRENGINE_GPU_MEM_MAX = total_ram * 1 / 8;
    KRContext::KRENGINE_GPU_MEM_TARGET = KRContext::KRENGINE_GPU_MEM_MAX * 3 / 4;


    
    /*
    BOOL isIpad = UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad;
    BOOL isRetina = [[UIScreen mainScreen] scale] >= 2.0;
    if(isIpad && isRetina) {
        KRContext::KRENGINE_GPU_MEM_MAX = 64000000 * 2;
        KRContext::KRENGINE_GPU_MEM_TARGET = 48000000 * 2;
        KRContext::KRENGINE_MAX_TEXTURE_DIM = 2048;
        KRContext::KRENGINE_MIN_TEXTURE_DIM = 64;
    } else {
        KRContext::KRENGINE_GPU_MEM_MAX = 64000000;
        KRContext::KRENGINE_GPU_MEM_TARGET = 48000000;
        KRContext::KRENGINE_MAX_TEXTURE_DIM = 2048;
        KRContext::KRENGINE_MIN_TEXTURE_DIM = 64;
    }
     */
#else
    KRContext::KRENGINE_GPU_MEM_MAX = 256000000;
    KRContext::KRENGINE_GPU_MEM_TARGET = 192000000;
    KRContext::KRENGINE_MAX_TEXTURE_DIM = 8192;
    KRContext::KRENGINE_MIN_TEXTURE_DIM = 64;
    KRContext::KRENGINE_PRESTREAM_DISTANCE = 1000.0f;
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
            @"debug_deferred" : @28,
            @"enable_deferred_lighting" : @29,
            @"near_clip" : @30,
            @"far_clip" : @31,
            @"volumetric_environment_enable" : @32,
            @"volumetric_environment_downsample" : @33,
            @"volumetric_environment_max_distance" : @34,
            @"volumetric_environment_slices" : @35,
            @"volumetric_environment_intensity" : @36,
            @"fog_type": @37,
            @"fog_near": @38,
            @"fog_far": @39,
            @"fog_density": @40,
            @"fog_color_r": @41,
            @"fog_color_g": @42,
            @"fog_color_b": @43,
            @"dust_enable" : @44,
            @"dust_intensity" : @45,
            @"lod_bias" : @46,
            @"enable_realtime_occlusion" : @47,
            @"debug_display" : @48,
            @"siren_enable" : @49,
            @"siren_enable_reverb" : @50,
            @"siren_enable_hrtf" : @51,
            @"siren_reverb_max_length" : @52,
            @"max_anisotropy" : @53
                            
        } copy];
        [self loadShaders];
        
    }
    
    return self;
}

- (void)renderScene: (KRScene *)pScene WithDeltaTime: (float)deltaTime AndWidth: (int)width AndHeight: (int)height AndDefaultFBO: (GLint)defaultFBO
{
    KRCamera *camera = pScene->find<KRCamera>("default_camera");
    if(camera) {
        camera->settings = _settings;
    }
    pScene->renderFrame(defaultFBO, deltaTime, width, height);
}

- (BOOL)loadShaders
{
#if TARGET_OS_IPHONE
    NSString *bundleName = @"kraken_standard_assets_ios";
#else
    NSString *bundleName = @"kraken_standard_assets_osx";
#endif

    //NSBundle *frameworkBundle = [NSBundle bundleWithIdentifier: @"com.krakenengine.standardassets"];
    NSBundle *frameworkBundle = [NSBundle bundleForClass: [self class]];
    //NSString *bundlePath = [[frameworkBundle bundlePath] stringByAppendingPathComponent:bundleName];
    NSString *bundlePath = [frameworkBundle pathForResource:bundleName ofType:@"bundle"];
    NSBundle *bundle = [NSBundle bundleWithPath:bundlePath];
    
    if(bundle == nil) {
        KRContext::Log(KRContext::LOG_LEVEL_ERROR, "%s", "ERROR - Standard asset bundle could not be found.");
    } else {
        NSEnumerator *bundleEnumerator = [[bundle pathsForResourcesOfType: nil inDirectory: nil] objectEnumerator];
        NSString * p = nil;
        while (p = [bundleEnumerator nextObject]) {
            NSString *file_name = [p lastPathComponent];
            if([file_name hasSuffix: @".vsh"] || [file_name hasSuffix: @".fsh"] || [file_name hasSuffix: @".krbundle"] ||[file_name hasPrefix:@"font."]) {
                KRContext::Log(KRContext::LOG_LEVEL_INFORMATION, "%s", [file_name UTF8String]);
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
    _parameter_names = nil;
    if(_context) {
        delete _context; _context = NULL;
    }
    [super dealloc];
}

-(int)getParameterCount
{
    return 53;
}


-(KRRenderSettings *)settings
{
    return &_settings;
}

-(NSString *)getParameterNameWithIndex: (int)i
{
    return [[self.parameter_names allKeysForObject:[NSNumber numberWithInt:i]] objectAtIndex:0];
}

-(NSString *)getParameterLabelWithIndex: (int)i
{
    NSString *parameter_labels[54] = {
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
        @"LOD Bias",
        @"Realtime Occlusion Tests",
        @"Debug - Display",
        @"Siren - Enable Audio",
        @"Siren - Enable Reverb",
        @"Siren - Enable HRTF",
        @"Siren - Max Reverb Len",
        @"Anisotropic Filtering"
    };
    return parameter_labels[i];
}
-(KREngineParameterType)getParameterTypeWithIndex: (int)i
{
    KREngineParameterType types[54] = {
        
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
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_INT,
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_FLOAT
    };
    return types[i];
}
-(float)getParameterValueWithIndex: (int)i
{
    float values[54] = {
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
        static_cast<float>(_settings.dof_quality),
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
        _settings.bShowDeferred ? 1.0f : 0.0f,
        _settings.bEnableDeferredLighting ? 1.0f : 0.0f,
        _settings.getPerspectiveNearZ(),
        _settings.getPerspectiveFarZ(),
        static_cast<float>(_settings.volumetric_environment_enable),
        static_cast<float>(5 - _settings.volumetric_environment_downsample),
        _settings.volumetric_environment_max_distance,
        _settings.volumetric_environment_quality,
        _settings.volumetric_environment_intensity,
        static_cast<float>(_settings.fog_type),
        _settings.fog_near,
        _settings.fog_far,
        _settings.fog_density,
        _settings.fog_color.x,
        _settings.fog_color.y,
        _settings.fog_color.z,
        static_cast<float>(_settings.dust_particle_enable),
        _settings.dust_particle_intensity,
        _settings.getLODBias(),
        static_cast<float>(_settings.getEnableRealtimeOcclusion()),
        _settings.debug_display,
        static_cast<float>(_settings.siren_enable),
        static_cast<float>(_settings.siren_enable_reverb),
        static_cast<float>(_settings.siren_enable_hrtf),
        _settings.siren_reverb_max_length,
        _settings.max_anisotropy
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
            if(_settings.bShowDeferred != bNewBoolVal) {
                _settings.bShowDeferred = bNewBoolVal;
            }
            break;
        case 29:
            if(_settings.bEnableDeferredLighting != bNewBoolVal) {
                _settings.bEnableDeferredLighting = bNewBoolVal;
            }
            break;
        case 30:
            _settings.setPerspectiveNear(v);
            break;
        case 31:
            _settings.setPerpsectiveFarZ(v);
            break;
        case 32:
            _settings.volumetric_environment_enable = bNewBoolVal;
            break;
        case 33:
            _settings.volumetric_environment_downsample = 5 - (int)v;
            break;
        case 34:
            _settings.volumetric_environment_max_distance = v;
            break;
        case 35:
            _settings.volumetric_environment_quality = v;
            break;
        case 36:
            _settings.volumetric_environment_intensity = v;
            break;
        case 37:
            _settings.fog_type = v;
            break;
        case 38:
            _settings.fog_near = v;
            break;
        case 39:
            _settings.fog_far = v;
            break;
        case 40:
            _settings.fog_density = v;
            break;
        case 41:
            _settings.fog_color.x = v;
            break;
        case 42:
            _settings.fog_color.y = v;
            break;
        case 43:
            _settings.fog_color.z = v;
            break;
        case 44:
            _settings.dust_particle_enable = bNewBoolVal;
            break;
        case 45:
            _settings.dust_particle_intensity = v;
            break;
        case 46:
            _settings.setLODBias(v);
            break;
        case 47:
            _settings.setEnableRealtimeOcclusion(bNewBoolVal);
            break;
        case 48:
            _settings.debug_display = (KRRenderSettings::debug_display_type)v;
            break;
        case 49:
            _settings.siren_enable = bNewBoolVal;
            break;
        case 50:
            _settings.siren_enable_reverb = bNewBoolVal;
            break;
        case 51:
            _settings.siren_enable_hrtf = bNewBoolVal;
            break;
        case 52:
            _settings.siren_reverb_max_length = v;
            break;
        case 53:
            _settings.max_anisotropy = v;
            break;
    }
}

-(float)getParameterMinWithIndex: (int)i
{
    float minValues[54] = {
        0.0f, 0.0f, 0.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.01f, 50.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f, -10.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,  0.05f, 1.0f
    };

    return minValues[i];
}

-(float)getParameterMaxWithIndex: (int)i
{
    float maxValues[54] = {
             PI,    3.0f,     1.0f,    1.0,  1.0f, 1.0f,    1.0f, 1.0f, 1.0f, 2.0f,
           1.0f,   5.0f,    2.0f,     1.0f, 1.0f, 1.0f,    5.0f, 1.0f, 0.5f,  1.0f,
           2.0f,    2.0f,    1.0f,     1.0f, 1.0f, 1.0f,    1.0f, 1.0f,
           1.0f,    1.0f,   10.0f, 1000.0f,  1.0f, 5.0f, 1000.0f, 1.0f, 5.0f,  3.0f,
        1000.0f, 1000.0f,    0.01f,    1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 10.0f, 1.0f, (float)(KRRenderSettings::KRENGINE_DEBUG_DISPLAY_NUMBER - 1),
        1.0f, 1.0f, 1.0f, 10.0f, 8.0f
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
    _debug_text = value;
    
    _settings.m_debug_text = value.UTF8String;
}

// ---===--- Sun Temperature and intensity ---===--- 

-(void) setSunTemperature:(float)t
{
    float i = [self getSunIntensity];
    _settings.light_intensity.init(
        (t < 0.5f ? t * 2.0f : 1.0f) * i,
        (t < 0.5f ? t * 2.0f : (1.0f - t) * 2.0f) * i,
        (t < 0.5f ? 1.0f : (1.0f - t) * 2.0f) * i
    );
}

-(void) setSunIntensity:(float)i
{
    float t = [self getSunTemperature];
    _settings.light_intensity.init(
        (t < 0.5f ? t * 2.0f : 1.0f) * i,
        (t < 0.5f ? t * 2.0f : (1.0f - t) * 2.0f) * i,
        (t < 0.5f ? 1.0f : (1.0f - t) * 2.0f) * i
    );
#ifdef TEST4REL    
    printf("Sun Intensity = %f \n", i);
#endif
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
    _settings.ambient_intensity.init(
        (t < 0.5f ? t * 2.0f : 1.0f) * i,
        (t < 0.5f ? t * 2.0f : (1.0f - t) * 2.0f) * i,
        (t < 0.5f ? 1.0f : (1.0f - t) * 2.0f) * i
    );
}

-(void) setAmbientIntensity:(float)i
{
    float t = [self getAmbientTemperature];
    _settings.ambient_intensity.init(
        (t < 0.5f ? t * 2.0f : 1.0f) * i,
        (t < 0.5f ? t * 2.0f : (1.0f - t) * 2.0f) * i,
        (t < 0.5f ? 1.0f : (1.0f - t) * 2.0f) * i
    );
#ifdef TEST4REL
    printf("ambient Intensity = %f \n", i);
#endif
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
