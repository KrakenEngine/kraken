//
//  KRPipeline.cpp
//  Kraken Engine
//
//  Copyright 2025 Kearwood Gilbert. All rights reserved.
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

#include "KRShaderReflection.h"

using namespace hydra;

const char* SHADER_VALUE_NAMES[] = {
    "material_ambient", // PushConstant::material_ambient
    "material_diffuse", // PushConstant::material_diffuse
    "material_specular", // PushConstant::material_specular
    "material_reflection", // PushConstant::material_reflection
    "material_alpha", // PushConstant::material_alpha
    "material_shininess", // PushConstant::material_shininess
    "light_position", // PushConstant::light_position
    "light_direction_model_space", // PushConstant::light_direction_model_space
    "light_direction_view_space", // PushConstant::light_direction_view_space
    "light_color", //    PushConstant::light_color
    "light_decay_start", //    PushConstant::light_decay_start
    "light_cutoff", //    PushConstant::light_cutoff
    "light_intensity", //    PushConstant::light_intensity
    "flare_size", //    PushConstant::flare_size
    "dust_particle_size", //  PushConstant::dust_particle_size
    "dust_particle_color", //   PushConstant::dust_particle_color
    "view_space_model_origin", //    PushConstant::view_space_model_origin
    "mvp_matrix", //    PushConstant::mvp
    "inv_projection_matrix", //    PushConstant::invp
    "inv_mvp_matrix", //    PushConstant::invmvp
    "inv_mvp_matrix_no_translate", //    PushConstant::invmvp_no_translate
    "model_view_inverse_transpose_matrix", //    PushConstant::model_view_inverse_transpose
    "model_inverse_transpose_matrix", //    PushConstant::model_inverse_transpose
    "model_view_matrix", //    PushConstant::model_view
    "model_matrix", //    PushConstant::model_matrix
    "projection_matrix", //    PushConstant::projection_matrix
    "camera_position_model_space", //    PushConstant::camerapos_model_space
    "viewport", //    PushConstant::viewport
    "diffuseTexture", //    PushConstant::diffusetexture
    "specularTexture", //    PushConstant::speculartexture
    "reflectionCubeTexture", //    PushConstant::reflectioncubetexture
    "reflectionTexture", //    PushConstant::reflectiontexture
    "normalTexture", //    PushConstant::normaltexture
    "diffuseTexture_Scale", //    PushConstant::diffusetexture_scale
    "specularTexture_Scale", //    PushConstant::speculartexture_scale
    "reflectionTexture_Scale", //    PushConstant::reflectiontexture_scale
    "normalTexture_Scale", //    PushConstant::normaltexture_scale
    "normalTexture_Scale", //    PushConstant::ambienttexture_scale
    "diffuseTexture_Offset", //    PushConstant::diffusetexture_offset
    "specularTexture_Offset", //    PushConstant::speculartexture_offset
    "reflectionTexture_Offset", //    PushConstant::reflectiontexture_offset
    "normalTexture_Offset", //    PushConstant::normaltexture_offset
    "ambientTexture_Offset", //    PushConstant::ambienttexture_offset
    "shadow_mvp1", //    PushConstant::shadow_mvp1
    "shadow_mvp2", //    PushConstant::shadow_mvp2
    "shadow_mvp3", //    PushConstant::shadow_mvp3
    "shadowTexture1", //    PushConstant::shadowtexture1
    "shadowTexture2", //    PushConstant::shadowtexture2
    "shadowTexture3", //    PushConstant::shadowtexture3
    "lightmapTexture", //    PushConstant::lightmaptexture
    "gbuffer_frame", //    PushConstant::gbuffer_frame
    "gbuffer_depth", //    PushConstant::gbuffer_depth
    "depthFrame", //    PushConstant::depth_frame
    "volumetricEnvironmentFrame", //    PushConstant::volumetric_environment_frame
    "renderFrame", //    PushConstant::render_frame
    "time_absolute", //    PushConstant::absolute_time
    "fog_near", //    PushConstant::fog_near
    "fog_far", //    PushConstant::fog_far
    "fog_density", //    PushConstant::fog_density
    "fog_color", //    PushConstant::fog_color
    "fog_scale", //    PushConstant::fog_scale
    "fog_density_premultiplied_exponential", //    PushConstant::density_premultiplied_exponential
    "fog_density_premultiplied_squared", //    PushConstant::density_premultiplied_squared
    "slice_depth_scale", //    PushConstant::slice_depth_scale
    "particle_origin", //    PushConstant::particle_origin
    "bone_transforms", //    PushConstant::bone_transforms
    "rim_color", // PushConstant::rim_color
    "rim_power", // PushConstant::rim_power
    "fade_color", // PushConstant::fade_color
};

bool KRReflectedObject::getShaderValue(ShaderValue value, ShaderValueType type, void* output) const
{
  switch (type) {
  case ShaderValueType::type_int32:
    return getShaderValue(value, static_cast<int32_t*>(output));
  case ShaderValueType::type_int64:
    return getShaderValue(value, static_cast<int64_t*>(output));
  case ShaderValueType::type_float32:
    return getShaderValue(value, static_cast<float*>(output));
  case ShaderValueType::type_float64:
    return getShaderValue(value, static_cast<double*>(output));
  case ShaderValueType::type_vector2:
    return getShaderValue(value, static_cast<Vector2*>(output));
  case ShaderValueType::type_vector3:
    return getShaderValue(value, static_cast<Vector3*>(output));
  case ShaderValueType::type_vector4:
    return getShaderValue(value, static_cast<Vector4*>(output));
  case ShaderValueType::type_matrix2:
    return getShaderValue(value, static_cast<Matrix2*>(output));
  case ShaderValueType::type_matrix2x3:
    return getShaderValue(value, static_cast<Matrix2x3*>(output));
  case ShaderValueType::type_matrix4:
    return getShaderValue(value, static_cast<Matrix4*>(output));
  default:
    return false;
  }
}

bool KRReflectedObject::getShaderValue(ShaderValue value, int32_t* output) const
{
  return false;
}

bool KRReflectedObject::getShaderValue(ShaderValue value, int64_t* output) const
{
  return false;
}

bool KRReflectedObject::getShaderValue(ShaderValue value, float* output) const
{
  return false;
}

bool KRReflectedObject::getShaderValue(ShaderValue value, double* output) const
{
  return false;
}

bool KRReflectedObject::getShaderValue(ShaderValue value, hydra::Vector2* output) const
{
  return false;
}

bool KRReflectedObject::getShaderValue(ShaderValue value, hydra::Vector3* output) const
{
  return false;
}

bool KRReflectedObject::getShaderValue(ShaderValue value, hydra::Vector4* output) const
{
  return false;
}

bool KRReflectedObject::getShaderValue(ShaderValue value, hydra::Matrix2* output) const
{
  return false;
}

bool KRReflectedObject::getShaderValue(ShaderValue value, hydra::Matrix2x3* output) const
{
  return false;
}

bool KRReflectedObject::getShaderValue(ShaderValue value, hydra::Matrix4* output) const
{
  return false;
}
