//
//  KRRenderSettings.cpp
//  Kraken Engine
//
//  Copyright 2022 Kearwood Gilbert. All rights reserved.
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

#include "KRRenderSettings.h"

KRRenderSettings::KRRenderSettings()
{
    siren_enable = true;
    siren_enable_reverb = true;
    siren_enable_hrtf = true;
    siren_reverb_max_length = 2.0f;
    
    m_enable_realtime_occlusion = false;
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
    bEnableDeferredLighting = false;
    max_anisotropy = 4.0f;
    
    ambient_intensity = Vector3::Zero();
    light_intensity = Vector3::One();
    
    perspective_fov = 45.0f * D2R;
    perspective_nearz = 0.3f;     // was 0.05f
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
    
    volumetric_environment_enable = false;
    volumetric_environment_downsample = 2;
    volumetric_environment_max_distance = 100.0f;
    volumetric_environment_quality = (50.0f - 5.0f) / 495.0f;
    volumetric_environment_intensity = 0.9f;
    
    
    fog_near = 50.0f;
    fog_far = 500.0f;
    fog_density = 0.0005f;
    fog_color = Vector3::Create(0.45f, 0.45f, 0.5f);
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
    siren_enable = s.siren_enable;
    siren_enable_reverb = s.siren_enable_reverb;
    siren_enable_hrtf = s.siren_enable_hrtf;
    siren_reverb_max_length = s.siren_reverb_max_length;
    
    bEnablePerPixel = s.bEnablePerPixel;
    bEnableDiffuseMap = s.bEnableDiffuseMap;
    bEnableNormalMap = s.bEnableNormalMap;
    bEnableSpecMap = s.bEnableSpecMap;
    bEnableReflectionMap = s.bEnableReflectionMap;
    bEnableReflection=s.bEnableReflection;
    bEnableLightMap=s.bEnableLightMap;
    bDebugPSSM=s.bDebugPSSM;
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
    debug_display = s.debug_display;
    
    m_lodBias = s.m_lodBias;
    m_enable_realtime_occlusion = s.m_enable_realtime_occlusion;
    
    max_anisotropy = s.max_anisotropy;
    
    return *this;
}

const Vector2 &KRRenderSettings::getViewportSize() {
    return m_viewportSize;
}

void KRRenderSettings::setViewportSize(const Vector2 &size) {
    m_viewportSize = size;
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