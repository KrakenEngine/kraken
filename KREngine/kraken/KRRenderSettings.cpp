//
//  KRRenderSettings.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-12-20.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KRRenderSettings.h"

KRRenderSettings::KRRenderSettings()
{
    
    m_enable_realtime_occlusion = true;
    bShowShadowBuffer = false;
    bShowOctree = false;
    bShowDeferred = false;
    bEnablePerPixel = true;
    bEnableDiffuseMap = true;
    bEnableNormalMap = true;
    bEnableSpecMap = true;
    bEnableReflectionMap = true;
    bEnableReflection = true;
    bDebugPSSM = false;
    bEnableAmbient = true;
    bEnableDiffuse = true;
    bEnableSpecular = true;
    bEnableLightMap = true;
    bDebugSuperShiny = false;
    bEnableDeferredLighting = false;
    
    ambient_intensity = KRVector3::Zero();
    light_intensity = KRVector3::One();
    
    perspective_fov = 45.0 * D2R;
    perspective_nearz = 0.05f;
    perspective_farz = 1000.0f;
    
    dof_quality = 0;
    dof_depth = 0.05f;
    dof_falloff = 0.05f;
    
    bEnableFlash = false;
    flash_intensity = 1.0f;
    flash_depth = 0.7f;
    flash_falloff = 0.5f;
    
    
    bEnableVignette = false;
    vignette_radius = 0.4f;
    vignette_falloff = 1.0f;
    
    
    m_cShadowBuffers = 0;

    
    m_skyBoxName = "";
    
    
    
    volumetric_environment_enable = false;
    volumetric_environment_downsample = 2;
    volumetric_environment_max_distance = 100.0f;
    volumetric_environment_quality = (50 - 5.0) / 495.0f;
    volumetric_environment_intensity = 0.9f;
    
    
    fog_near = 50.0f;
    fog_far = 500.0f;
    fog_density = 0.0005f;
    fog_color = KRVector3(0.45, 0.45, 0.5);
    fog_type = 0;
    
    dust_particle_intensity = 0.25f;
    dust_particle_enable = false;
    
    m_lodBias = 0.0f;
    
    debug_display = KRENGINE_DEBUG_DISPLAY_NONE;
}

KRRenderSettings::~KRRenderSettings()
{
    
}

KRRenderSettings& KRRenderSettings::operator=(const KRRenderSettings &s)
{
    bEnablePerPixel = s.bEnablePerPixel;
    bEnableDiffuseMap = s.bEnableDiffuseMap;
    bEnableNormalMap = s.bEnableNormalMap;
    bEnableSpecMap = s.bEnableSpecMap;
    bEnableReflectionMap = s.bEnableReflectionMap;
    bEnableReflection=s.bEnableReflection;
    bEnableLightMap=s.bEnableLightMap;
    bDebugPSSM=s.bDebugPSSM;
    bDebugSuperShiny=s.bDebugSuperShiny;
    bShowShadowBuffer=s.bShowShadowBuffer;
    bShowOctree=s.bShowOctree;
    bShowDeferred=s.bShowDeferred;
    bEnableAmbient=s.bEnableAmbient;
    bEnableDiffuse=s.bEnableDiffuse;
    bEnableSpecular=s.bEnableSpecular;
    bEnableDeferredLighting=s.bEnableDeferredLighting;
    light_intensity=s.light_intensity;
    ambient_intensity=s.ambient_intensity;
    perspective_fov=s.perspective_fov;
    
    dof_quality=s.dof_quality;
    dof_depth=s.dof_depth;
    dof_falloff=s.dof_falloff;
    bEnableFlash=s.bEnableFlash;
    flash_intensity=s.flash_intensity;
    flash_depth=s.flash_depth;
    flash_falloff=s.flash_falloff;
    
    bEnableVignette=s.bEnableVignette;
    vignette_radius=s.vignette_radius;
    vignette_falloff=s.vignette_falloff;
    
    m_viewportSize=s.m_viewportSize;
    
    m_cShadowBuffers=s.m_cShadowBuffers;
    
    m_debug_text=s.m_debug_text;
    
    volumetric_environment_enable=s.volumetric_environment_enable;
    volumetric_environment_downsample=s.volumetric_environment_downsample;
    volumetric_environment_max_distance=s.volumetric_environment_max_distance;
    volumetric_environment_quality=s.volumetric_environment_quality;
    volumetric_environment_intensity=s.volumetric_environment_intensity;
    
    fog_near=s.fog_near;
    fog_far=s.fog_far;
    fog_density=s.fog_density;
    fog_color=s.fog_color;
    fog_type=s.fog_type;
    
    dust_particle_intensity=s.dust_particle_intensity;
    dust_particle_enable=s.dust_particle_enable;
    perspective_nearz=s.perspective_nearz;
    perspective_farz=s.perspective_farz;
    
    m_skyBoxName=s.m_skyBoxName;
    debug_display = s.debug_display;
    
    m_lodBias = s.m_lodBias;
    m_enable_realtime_occlusion = s.m_enable_realtime_occlusion;
    
    return *this;
}

const KRVector2 &KRRenderSettings::getViewportSize() {
    return m_viewportSize;
}

void KRRenderSettings::setViewportSize(const KRVector2 &size) {
    m_viewportSize = size;
}

void KRRenderSettings::setSkyBox(const std::string &skyBoxName) {
    m_skyBoxName = skyBoxName;
}

float KRRenderSettings::getPerspectiveNearZ()
{
    return perspective_nearz;
}
float KRRenderSettings::getPerspectiveFarZ()
{
    return perspective_farz;
}

void KRRenderSettings::setPerspectiveNear(float v)
{
    if(perspective_nearz != v) {
        perspective_nearz = v;
    }
}
void KRRenderSettings::setPerpsectiveFarZ(float v)
{
    if(perspective_farz != v) {
        perspective_farz = v;
    }
}


float KRRenderSettings::getLODBias()
{
    return m_lodBias;
}

void KRRenderSettings::setLODBias(float v)
{
    m_lodBias = v;
}

bool KRRenderSettings::getEnableRealtimeOcclusion()
{
    return m_enable_realtime_occlusion;
}
void KRRenderSettings::setEnableRealtimeOcclusion(bool enable)
{
    m_enable_realtime_occlusion = enable;
}