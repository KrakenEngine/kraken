//
//  KRShaderReflection.h
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

#include <map>
#include "hydra.h"

enum class ShaderValueType : uint8_t
{
  type_null = 0,
  type_int32,
  type_int64,
  type_float32,
  type_float64,
  type_vector2,
  type_vector3,
  type_vector4,
  type_matrix2,
  type_matrix2x3,
  type_matrix4,
  NUM_SHADER_VALUE_TYPES
};

enum class ShaderValue : uint8_t
{
  material_ambient = 0,
  material_diffuse,
  material_specular,
  material_reflection,
  material_alpha,
  material_shininess,
  light_position,
  light_direction_model_space,
  light_direction_view_space,
  light_color,
  light_decay_start,
  light_cutoff,
  light_intensity,
  flare_size,
  view_space_model_origin,
  mvp,
  invp,
  invmvp,
  invmvp_no_translate,
  model_view_inverse_transpose,
  model_inverse_transpose,
  model_view,
  model_matrix,
  projection_matrix,
  camerapos_model_space,
  viewport,
  diffusetexture,
  speculartexture,
  reflectioncubetexture,
  reflectiontexture,
  normaltexture,
  diffusetexture_scale,
  speculartexture_scale,
  reflectiontexture_scale,
  normaltexture_scale,
  ambienttexture_scale,
  diffusetexture_offset,
  speculartexture_offset,
  reflectiontexture_offset,
  normaltexture_offset,
  ambienttexture_offset,
  shadow_mvp1,
  shadow_mvp2,
  shadow_mvp3,
  shadowtexture1,
  shadowtexture2,
  shadowtexture3,
  lightmaptexture,
  gbuffer_frame,
  gbuffer_depth,
  depth_frame,
  volumetric_environment_frame,
  render_frame,
  absolute_time,
  fog_near,
  fog_far,
  fog_density,
  fog_color,
  fog_scale,
  density_premultiplied_exponential,
  density_premultiplied_squared,
  slice_depth_scale,
  particle_origin,
  bone_transforms,
  rim_color,
  rim_power,
  fade_color,
  NUM_PUSH_CONSTANTS
};

const char* SHADER_VALUE_NAMES[];

class KRReflectedObject
{
public:
  bool getShaderValue(ShaderValue value, ShaderValueType type, void* output) const;
private:
  virtual bool getShaderValue(ShaderValue value, int32_t* output) const;
  virtual bool getShaderValue(ShaderValue value, int64_t* output) const;
  virtual bool getShaderValue(ShaderValue value, float* output) const;
  virtual bool getShaderValue(ShaderValue value, double* output) const;
  virtual bool getShaderValue(ShaderValue value, hydra::Vector2* output) const;
  virtual bool getShaderValue(ShaderValue value, hydra::Vector3* output) const;
  virtual bool getShaderValue(ShaderValue value, hydra::Vector4* output) const;
  virtual bool getShaderValue(ShaderValue value, hydra::Matrix2* output) const;
  virtual bool getShaderValue(ShaderValue value, hydra::Matrix2x3* output) const;
  virtual bool getShaderValue(ShaderValue value, hydra::Matrix4* output) const;

};
