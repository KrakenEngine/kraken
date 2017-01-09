//
//  KRRenderSettings.h
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-12-20.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KRRENDERSETTINGS_H
#define KRRENDERSETTINGS_H

#include "KREngine-common.h"

class KRRenderSettings {
public:
    KRRenderSettings();
    ~KRRenderSettings();
    
    // Overload assignment operator
    KRRenderSettings& operator=(const KRRenderSettings &s);
    
    const KRVector2 &getViewportSize();
    void setViewportSize(const KRVector2 &size);
    
    float getPerspectiveNearZ();
    float getPerspectiveFarZ();
    void setPerspectiveNear(float v);
    void setPerpsectiveFarZ(float v);
    
    float getLODBias();
    void setLODBias(float v);
    
    bool bEnablePerPixel;
    bool bEnableDiffuseMap;
    bool bEnableNormalMap;
    bool bEnableSpecMap;
    bool bEnableReflectionMap;
    bool bEnableReflection;
    bool bEnableLightMap;
    bool bDebugPSSM;
    bool bShowShadowBuffer;
    bool bShowOctree;
    bool bShowDeferred;
    bool bEnableAmbient;
    bool bEnableDiffuse;
    bool bEnableSpecular;
    bool bEnableDeferredLighting;
    KRVector3 light_intensity;
    KRVector3 ambient_intensity;
    float perspective_fov;
    
    int dof_quality;
    float dof_depth;
    float dof_falloff;
    bool bEnableFlash;
    float flash_intensity;
    float flash_depth;
    float flash_falloff;
    
    bool bEnableVignette;
    float vignette_radius;
    float vignette_falloff;
    
    KRVector2 m_viewportSize;
    
    int m_cShadowBuffers;
    
    std::string m_debug_text;
    
    bool volumetric_environment_enable;
    int volumetric_environment_downsample;
    float volumetric_environment_max_distance;
    float volumetric_environment_quality;
    float volumetric_environment_intensity;
    
    float fog_near;
    float fog_far;
    float fog_density;
    KRVector3 fog_color;
    int fog_type; // 0 = no fog, 1 = linear, 2 = exponential, 3 = exponential squared
    
    float dust_particle_intensity;
    bool dust_particle_enable;
    float perspective_nearz;
    float perspective_farz;
    
    enum debug_display_type{
        KRENGINE_DEBUG_DISPLAY_NONE = 0,
        KRENGINE_DEBUG_DISPLAY_TIME,
        KRENGINE_DEBUG_DISPLAY_MEMORY,
        KRENGINE_DEBUG_DISPLAY_TEXTURES,
        KRENGINE_DEBUG_DISPLAY_DRAW_CALLS,
        KRENGINE_DEBUG_DISPLAY_OCTREE,
        KRENGINE_DEBUG_DISPLAY_COLLIDERS,
        KRENGINE_DEBUG_DISPLAY_BONES,
        KRENGINE_DEBUG_DISPLAY_SIREN_REVERB_ZONES,
        KRENGINE_DEBUG_DISPLAY_SIREN_AMBIENT_ZONES,
        KRENGINE_DEBUG_DISPLAY_NUMBER
    } debug_display;
    
    bool getEnableRealtimeOcclusion();
    void setEnableRealtimeOcclusion(bool enable);
    
    bool siren_enable;
    bool siren_enable_reverb;
    bool siren_enable_hrtf;
    float siren_reverb_max_length;
    
    float max_anisotropy;

private:
    float m_lodBias;
    bool m_enable_realtime_occlusion;
};

#endif
