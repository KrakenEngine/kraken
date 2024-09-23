//
//  object.vert
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

#version 450
#extension GL_GOOGLE_include_directive : enable

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;

#if HAS_NORMAL_MAP == 1
layout(location = 2) in vec3 vertex_tangent;
#endif
layout(location = 3) in lowp vec3 vertex_uv;

#if BONE_COUNT > 0
    layout(location = 4) in lowp vec3 vertex_uv;
    layout(location = 5) in highp vec4 bone_weights;
    layout(location = 6) in highp vec4 bone_indexes;
#else
    #define vertex_position_skinned vertex_position
    #define vertex_normal_skinned vertex_normal
    #define vertex_tangent_skinned vertex_tangent
#endif

#if GBUFFER_PASS == 1
    #if HAS_NORMAL_MAP == 1
        highp mat3 tangent_to_view_matrix;
    #endif
#else
    #if HAS_LIGHT_MAP == 1
        layout(loction = 7) in mediump vec2  vertex_lightmap_uv;
    #endif //HAS_LIGHT_MAP
#endif // GBUFFER_PASS


layout( push_constant ) uniform constants
{
  highp mat4 mvp_matrix; // mvp_matrix is the result of multiplying the model, view, and projection matrices
#if BONE_COUNT > 0
  highp mat4 bone_transforms[BONE_COUNT];
#endif
#if ENABLE_PER_PIXEL == 1 || GBUFFER_PASS == 1
  #if HAS_NORMAL_MAP == 1
    #if HAS_NORMAL_MAP_SCALE == 1
      highp vec2 normalTexture_Scale;
    #endif
    #if HAS_NORMAL_MAP_OFFSET == 1
      highp vec2 normalTexture_Offset;
    #endif
  #endif
#else
  mediump float material_shininess;
#endif
#if GBUFFER_PASS == 1
    #if HAS_NORMAL_MAP == 1
        highp mat4 model_view_inverse_transpose_matrix;
    #endif
#else
  highp vec3 light_direction_model_space; // Must be normalized before entering shader
  highp vec3 camera_position_model_space;
  #if ENABLE_PER_PIXEL == 1
      #if HAS_SPEC_MAP_SCALE == 1
          highp vec2 specularTexture_Scale;
      #endif

      #if HAS_SPEC_MAP_OFFSET == 1
          highp vec2 specularTexture_Offset;
      #endif
      
      #if HAS_REFLECTION_MAP_SCALE == 1
          highp vec2 reflectionTexture_Scale;
      #endif

      #if HAS_REFLECTION_MAP_OFFSET == 1
          highp vec2 reflectionTexture_Offset;
      #endif
      
      #if SHADOW_QUALITY >= 1
          highp mat4 shadow_mvp1;
      #endif

      #if SHADOW_QUALITY >= 2
          highp mat4 shadow_mvp2;
      #endif

      #if SHADOW_QUALITY >= 3
          highp mat4 shadow_mvp3;
      #endif
  #endif // ENABLE_PER_PIXEL
  
  #if HAS_REFLECTION_CUBE_MAP == 1
      #if HAS_NORMAL_MAP == 1
          #define NEED_EYEVEC
          highp mat4 model_inverse_transpose_matrix;
      #else
          highp mat4 model_matrix;
      #endif
  #endif
  
  #if HAS_DIFFUSE_MAP_SCALE == 1
      highp vec2  diffuseTexture_Scale;
  #endif

  #if HAS_DIFFUSE_MAP_OFFSET == 1
      highp vec2  diffuseTexture_Offset;
  #endif
#endif


#if ENABLE_RIM_COLOR == 1
    lowp vec3 rim_color;
    mediump float rim_power;
#endif

#if FOG_TYPE > 0
    // FOG_TYPE 1 - Linear
    // FOG_TYPE 2 - Exponential
    // FOG_TYPE 3 - Exponential squared
    lowp vec3 fog_color;
    mediump float fog_near;
    #if FOG_TYPE == 1
        mediump float fog_far;
        mediump float fog_scale;
    #endif

    #if FOG_TYPE > 1
        mediump float fog_density;
    #endif

    #if FOG_TYPE == 2
        mediump float fog_density_premultiplied_exponential;
    #endif
    #if FOG_TYPE == 3
        mediump float fog_density_premultiplied_squared;
    #endif
#endif


#if ENABLE_PER_PIXEL == 1 || GBUFFER_PASS == 1
    mediump float material_shininess;
    #if HAS_NORMAL_MAP == 1
        sampler2D normalTexture;
    #endif
#endif


#if GBUFFER_PASS == 3
    sampler2D gbuffer_frame;
    sampler2D gbuffer_depth;
#endif

#if GBUFFER_PASS == 1
    #if HAS_NORMAL_MAP == 1

    #else
        highp mat4 model_view_inverse_transpose_matrix;
    #endif

    #if HAS_DIFFUSE_MAP == 1 && ALPHA_TEST == 1
        sampler2D     diffuseTexture;
    #endif
#else
    lowp vec3 material_ambient;
    lowp vec3 material_diffuse;
    lowp vec3 material_specular;
    lowp float material_alpha;


    #if HAS_DIFFUSE_MAP == 1
        sampler2D     diffuseTexture;
    #endif

    #if HAS_SPEC_MAP == 1
        sampler2D     specularTexture;
    #endif

    #if HAS_REFLECTION_MAP == 1
        sampler2D     reflectionTexture;
    #endif

    #if ENABLE_RIM_COLOR == 1
        #define NEED_EYEVEC
    #endif

    #if HAS_REFLECTION_CUBE_MAP == 1
        lowp vec3       material_reflection;
        samplerCube     reflectionCubeTexture;
        #if HAS_NORMAL_MAP == 1
            highp mat4 model_matrix;
        #endif
    #endif

    #if SHADOW_QUALITY >= 1
        #ifdef GL_EXT_shadow_samplers
            sampler2DShadow   shadowTexture1;
        #else
            sampler2D   shadowTexture1;
        #endif
    #endif

    #if HAS_LIGHT_MAP == 1
        sampler2D     lightmapTexture;
    #endif

    #if SHADOW_QUALITY >= 2
        ampler2D   shadowTexture2;
    #endif

    #if SHADOW_QUALITY >= 3
        sampler2D   shadowTexture3;
    #endif

#endif

#if GBUFFER_PASS == 1 || GBUFFER_PASS == 3
    mediump vec4 viewport;
#endif
} PushConstants;

#if ENABLE_PER_PIXEL == 1 || GBUFFER_PASS == 1
    #if HAS_DIFFUSE_MAP == 1 || HAS_NORMAL_MAP == 1 || HAS_SPEC_MAP == 1 || HAS_REFLECTION_MAP == 1
        layout(location=0) out highp vec2 texCoord;
    #endif
    #if HAS_NORMAL_MAP == 1
        #if HAS_NORMAL_MAP_OFFSET == 1 || HAS_NORMAL_MAP_SCALE == 1
        layout(location=1) out highp vec2 normal_uv;
        #endif
    #else
      layout(location=2) out mediump vec3 normal;
    #endif
#else
    #if HAS_DIFFUSE_MAP == 1
      layout(location=3) out highp vec2 texCoord;
    #endif
#endif

#if GBUFFER_PASS == 1
    #if HAS_NORMAL_MAP == 1
      layout(location=4) out highp mat3 tangent_to_view_matrix;
    #endif
#else
    #if HAS_LIGHT_MAP == 1
      layout(location=5) out mediump vec2    lightmap_uv;
    #endif

    #if ENABLE_PER_PIXEL == 1
        layout(location=6) out mediump vec3    lightVec;
        layout(location=7) out out mediump vec3    halfVec;

        #if HAS_SPEC_MAP_OFFSET == 1 || HAS_SPEC_MAP_SCALE == 1
          layout(location = 8) out highp vec2 spec_uv;
        #endif

        #if HAS_REFLECTION_MAP_OFFSET == 1 || HAS_REFLECTION_MAP_SCALE == 1
          layout(location = 9) out highp vec2 reflection_uv;
        #endif

        #if SHADOW_QUALITY >= 1
          layout(location = 10) out highp vec4	shadowMapCoord1;
        #endif

        #if SHADOW_QUALITY >= 2
          layout(location = 11) out highp vec4	shadowMapCoord2;
        #endif

        #if SHADOW_QUALITY >= 3
          layout(location = 12) out highp vec4	shadowMapCoord3;
        #endif

    #else
      layout(location = 13) out mediump float   lamberFactor;
      layout(location = 14) out mediump float   specularFactor;
    #endif

    #if ENABLE_RIM_COLOR == 1
        #define NEED_EYEVEC
    #endif

    #if HAS_REFLECTION_CUBE_MAP == 1
        #if HAS_NORMAL_MAP == 1
            #define NEED_EYEVEC
          layout(location = 15) out highp mat3 tangent_to_world_matrix;
        #else
          layout(location = 16) out mediump vec3 reflectionVec;
        #endif
    #endif

    #ifdef NEED_EYEVEC
      layout(location = 17) out mediump vec3 eyeVec;
    #endif

    #if HAS_DIFFUSE_MAP_OFFSET == 1 || HAS_DIFFUSE_MAP_SCALE == 1
      layout(location = 18) out highp vec2  diffuse_uv;
    #endif

#endif


void main()
{
#if BONE_COUNT > 0
    mediump vec4 scaled_bone_indexes = bone_indexes;
    mediump vec4 scaled_bone_weights = bone_weights;
    
    //scaled_bone_indexes = vec4(0.0, 0.0, 0.0, 0.0);
    //scaled_bone_weights = vec4(1.0, 0.0, 0.0, 0.0);
    
    highp mat4 skin_matrix =
        bone_transforms[ int(scaled_bone_indexes.x) ] * scaled_bone_weights.x +
        bone_transforms[ int(scaled_bone_indexes.y) ] * scaled_bone_weights.y +
        bone_transforms[ int(scaled_bone_indexes.z) ] * scaled_bone_weights.z +
        bone_transforms[ int(scaled_bone_indexes.w) ] * scaled_bone_weights.w;
    //skin_matrix = bone_transforms[0];
    highp vec3 vertex_position_skinned = (skin_matrix * vec4(vertex_position, 1)).xyz;

    highp vec3 vertex_normal_skinned = normalize(mat3(skin_matrix) * vertex_normal);
    #if HAS_NORMAL_MAP == 1
        highp vec3 vertex_tangent_skinned = normalize(mat3(skin_matrix) * vertex_tangent);
    #endif
    
#endif
    
    // Transform position
    gl_Position = PushConstants.mvp_matrix * vec4(vertex_position_skinned,1.0);


    
    #if HAS_DIFFUSE_MAP == 1 || (HAS_NORMAL_MAP == 1 && ENABLE_PER_PIXEL == 1) || (HAS_SPEC_MAP == 1 && ENABLE_PER_PIXEL == 1) || (HAS_REFLECTION_MAP == 1 && ENABLE_PER_PIXEL == 1)
        // Pass UV co-ordinates
        texCoord = vertex_uv.st;
    #endif
    

    

    // Scaled and translated normal map UV's
    #if (HAS_NORMAL_MAP_OFFSET == 1 || HAS_NORMAL_MAP_SCALE == 1) && ENABLE_PER_PIXEL == 1
        normal_uv = texCoord;
        
        #if HAS_NORMAL_MAP_OFFSET == 1
            normal_uv += normalTexture_Offset;
        #endif
            
        #if HAS_NORMAL_MAP_SCALE == 1
            normal_uv *= normalTexture_Scale;
        #endif
        
    #endif

    #if GBUFFER_PASS != 1 || ALPHA_TEST == 1
        // Scaled and translated diffuse map UV's
        #if HAS_DIFFUSE_MAP_OFFSET == 1 || HAS_DIFFUSE_MAP_SCALE == 1
            diffuse_uv = texCoord;
                
            #if HAS_DIFFUSE_MAP_OFFSET == 1
                diffuse_uv += PushConstants.diffuseTexture_Offset;
            #endif
                
            #if HAS_DIFFUSE_MAP_SCALE == 1
                diffuse_uv *= PushConstants.diffuseTexture_Scale;
            #endif
        #endif
    #endif
    
    
    #if GBUFFER_PASS == 1
        #if HAS_NORMAL_MAP == 1
            mediump vec3 a_bitangent = cross(vertex_normal_skinned, vertex_tangent_skinned);
            tangent_to_view_matrix[0] = vec3(model_view_inverse_transpose_matrix * vec4(vertex_tangent_skinned, 1.0));
            tangent_to_view_matrix[1] = vec3(model_view_inverse_transpose_matrix * vec4(a_bitangent, 1.0));
            tangent_to_view_matrix[2] = vec3(model_view_inverse_transpose_matrix * vec4(vertex_normal_skinned, 1.0));
        #else
            normal = vertex_normal_skinned;
        #endif
    #else

        #if HAS_REFLECTION_CUBE_MAP == 1
            #if HAS_NORMAL_MAP == 1
    
            #else
                // Calculate reflection vector as I - 2.0 * dot(N, I) * N
                mediump vec3 eyeVec = normalize(PushConstants.camera_position_model_space - vertex_position_skinned);
                mediump vec3 incidenceVec = -eyeVec;
                reflectionVec = mat3(PushConstants.model_matrix) * (incidenceVec - 2.0 * dot(vertex_normal_skinned, incidenceVec) * vertex_normal_skinned);
            #endif
        #endif
    
        #ifdef NEED_EYEVEC
            eyeVec = normalize(PushConstants.camera_position_model_space - vertex_position_skinned);
        #endif
    
        #if HAS_LIGHT_MAP == 1
            // Pass shadow UV co-ordinates
            lightmap_uv = vertex_lightmap_uv.st;
        #endif
    


        #if ENABLE_PER_PIXEL == 1
            // Scaled and translated specular map UV's
            #if HAS_SPEC_MAP_OFFSET == 1 || HAS_SPEC_MAP_SCALE == 1
                spec_uv = texCoord;
                #if HAS_SPEC_MAP_OFFSET == 1
                    spec_uv += PushConstants.specularTexture_Offset;
                #endif
                    
                #if HAS_SPEC_MAP_SCALE == 1
                    spec_uv *= PushConstants.specularTexture_Scale;
                #endif
            #endif
    
            // Scaled and translated reflection map UV's
            #if HAS_REFLECTION_MAP_OFFSET == 1 || HAS_REFLECTION_MAP_SCALE == 1
                reflection_uv = texCoord;
                #if HAS_REFLECTION_MAP_OFFSET == 1
                    reflection_uv += PushConstants.reflectionTexture_Offset;
                #endif
                    
                #if HAS_REFLECTION_MAP_SCALE == 1
                    reflection_uv *= PushConstants.reflectionTexture_Scale;
                #endif
            #endif
    
    
            #if SHADOW_QUALITY >= 1
                shadowMapCoord1 = PushConstants.shadow_mvp1 * vec4(vertex_position_skinned,1.0);
            #endif
    
            #if SHADOW_QUALITY >= 2
                shadowMapCoord2 = PushConstants.shadow_mvp2 * vec4(vertex_position_skinned,1.0);
            #endif
                
            #if SHADOW_QUALITY >= 3
                shadowMapCoord3 = PushConstants.shadow_mvp3 * vec4(vertex_position_skinned,1.0);
            #endif

            // ----------- Directional Light (Sun) -----------
            #if HAS_NORMAL_MAP == 1
                // ----- Calculate per-pixel lighting in tangent space, for normal mapping ------
                mediump vec3 a_bitangent = cross(vertex_normal_skinned, vertex_tangent_skinned);
                #if HAS_REFLECTION_CUBE_MAP == 0
                    // The cube map reflections also require an eyeVec as a varying attribute when normal mapping, so only re-calculate here when needed
                    mediump vec3 eyeVec = normalize(PushConstants.camera_position_model_space - vertex_position_skinned);
                #else
                    tangent_to_world_matrix[0] = vec3(PushConstants.model_inverse_transpose_matrix * vec4(vertex_tangent_skinned, 1.0));
                    tangent_to_world_matrix[1] = vec3(PushConstants.model_inverse_transpose_matrix * vec4(a_bitangent, 1.0));
                    tangent_to_world_matrix[2] = vec3(PushConstants.model_inverse_transpose_matrix * vec4(vertex_normal_skinned, 1.0));
                #endif
    
                lightVec = normalize(vec3(dot(PushConstants.light_direction_model_space, vertex_tangent_skinned), dot(PushConstants.light_direction_model_space, a_bitangent), dot(PushConstants.light_direction_model_space, vertex_normal_skinned)));
                halfVec = normalize(vec3(dot(eyeVec, vertex_tangent_skinned), dot(eyeVec, a_bitangent), dot(eyeVec, vertex_normal_skinned)));
                halfVec = normalize(halfVec + lightVec); // Normalizing anyways, no need to divide by 2
            #else
                // ------ Calculate per-pixel lighting without normal mapping ------
                normal = vertex_normal_skinned;
                lightVec = PushConstants.light_direction_model_space;
                halfVec = normalize((normalize(PushConstants.camera_position_model_space - vertex_position_skinned) + lightVec)); // Normalizing anyways, no need to divide by 2
            #endif
        #else
    
            // ------ Calculate per-vertex lighting ------
            mediump vec3 halfVec = normalize((normalize(PushConstants.camera_position_model_space - vertex_position_skinned) + PushConstants.light_direction_model_space)); // Normalizing anyways, no need to divide by 2
            lamberFactor = max(0.0,dot(PushConstants.light_direction_model_space, vertex_normal_skinned));
            specularFactor = max(0.0,pow(dot(halfVec,vertex_normal_skinned), PushConstants.material_shininess));
        #endif
    #endif
}
