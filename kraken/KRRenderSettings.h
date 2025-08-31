//
//  KRRenderSettings.h
//  Kraken Engine
//
//  Copyright 2024 Kearwood Gilbert. All rights reserved.
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

#pragma once

#include "KREngine-common.h"

#include "KRShaderReflection.h"

class KRRenderSettings
  : public KRReflectedObject
{
public:
  KRRenderSettings();
  ~KRRenderSettings();

  // Overload assignment operator
  KRRenderSettings& operator=(const KRRenderSettings& s);

  const hydra::Vector2& getViewportSize();
  void setViewportSize(const hydra::Vector2& size);

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
  hydra::Vector3 light_intensity;
  hydra::Vector3 ambient_intensity;
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

  hydra::Vector2 m_viewportSize;

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
  hydra::Vector3 fog_color;
  int fog_type; // 0 = no fog, 1 = linear, 2 = exponential, 3 = exponential squared

  float dust_particle_intensity;
  bool dust_particle_enable;
  float perspective_nearz;
  float perspective_farz;

  enum debug_display_type
  {
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

  bool getShaderValue(ShaderValue value, float* output) const final;
  bool getShaderValue(ShaderValue value, hydra::Vector3* output) const final;
};
