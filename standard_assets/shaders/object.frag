//
//  object.frag
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

#version 450
#extension GL_GOOGLE_include_directive : enable

// TODO - HACK! Need to dynamically set these defines...
#define ENABLE_DIFFUSE 1
#define ENABLE_PER_PIXEL 1

//#extension GL_EXT_shadow_samplers : require

layout(location = 0) out vec4 colorOut;

/*
#if ENABLE_PER_PIXEL == 1 || GBUFFER_PASS == 1
    #if HAS_NORMAL_MAP == 1
        
    #else
        layout(location = 0) in mediump vec3 normal;
    #endif

    #if HAS_DIFFUSE_MAP == 1 || HAS_NORMAL_MAP == 1 || HAS_SPEC_MAP == 1 || HAS_REFLECTION_MAP == 1
        layout(location = 1) in highp vec2    texCoord;
    #endif
    #if HAS_NORMAL_MAP_OFFSET == 1 || HAS_NORMAL_MAP_SCALE == 1
        layout(location = 2) in highp vec2  normal_uv;
    #else
        #define normal_uv texCoord
    #endif
#else
    #if HAS_DIFFUSE_MAP == 1
        layout(location = 3) in highp vec2    texCoord;
    #endif
#endif

#if GBUFFER_PASS == 1
    #if HAS_NORMAL_MAP == 1
        layout(location = 4) in highp mat3 tangent_to_view_matrix;
    #endif

    #if HAS_DIFFUSE_MAP == 1 && ALPHA_TEST == 1
        #if HAS_DIFFUSE_MAP_OFFSET == 1 || HAS_DIFFUSE_MAP_SCALE == 1
            layout(location = 5) in highp vec2  diffuse_uv;
        #else
            #define diffuse_uv texCoord
        #endif
    #endif
#else

    #if ENABLE_RIM_COLOR == 1
        #define NEED_EYEVEC
    #endif

    #if HAS_REFLECTION_CUBE_MAP == 1
        #if HAS_NORMAL_MAP == 1
            layout(location = 6) in highp mat3 tangent_to_world_matrix;
            #define NEED_EYEVEC

        #else
            layout(location = 7) in mediump vec3 reflectionVec;
        #endif
    #endif

    #ifdef NEED_EYEVEC
        layout(location = 8) in mediump vec3 eyeVec;
    #endif


    #if SHADOW_QUALITY >= 1
        layout(location = 9) in highp vec4  shadowMapCoord1;
    #endif

    #if HAS_LIGHT_MAP == 1
        layout(location = 10) in mediump vec2  lightmap_uv;
    #endif

    #if SHADOW_QUALITY >= 2
        layout(location = 11) in highp vec4  shadowMapCoord2;
    #endif

    #if SHADOW_QUALITY >= 3
        layout(location = 12) in highp vec4  shadowMapCoord3;
    #endif

    #if ENABLE_PER_PIXEL == 1
        layout(location = 13) in mediump vec3    lightVec;
        layout(location = 14) in mediump vec3    halfVec;
    #else
        layout(location = 15) in mediump float   lamberFactor;
        layout(location = 16) in mediump float   specularFactor;
    #endif

    #if (HAS_SPEC_MAP_OFFSET == 1|| HAS_SPEC_MAP_SCALE == 1) && ENABLE_PER_PIXEL == 1
        layout(location = 17) in mediump vec2 spec_uv;
    #else
        #define spec_uv texCoord
    #endif

    #if (HAS_REFLECTION_MAP_OFFSET == 1|| HAS_REFLECTION_MAP_SCALE == 1) && ENABLE_PER_PIXEL == 1
        layout(location = 18) in mediump vec2 reflection_uv;
    #else
        #define reflection_uv texCoord
    #endif

    #if HAS_DIFFUSE_MAP_OFFSET == 1 || HAS_DIFFUSE_MAP_SCALE == 1
        layout(location = 19) in highp vec2  diffuse_uv;
    #else
        #define diffuse_uv texCoord
    #endif

#endif
 */

#if ENABLE_PER_PIXEL == 1 || GBUFFER_PASS == 1
    #if HAS_DIFFUSE_MAP == 1 || HAS_NORMAL_MAP == 1 || HAS_SPEC_MAP == 1 || HAS_REFLECTION_MAP == 1
        layout(location=0) in highp vec2 texCoord;
    #endif
    #if HAS_NORMAL_MAP == 1
        #if HAS_NORMAL_MAP_OFFSET == 1 || HAS_NORMAL_MAP_SCALE == 1
        layout(location=1) in highp vec2 normal_uv;
        #endif
    #else
      layout(location=2) in mediump vec3 normal;
    #endif
#else
    #if HAS_DIFFUSE_MAP == 1
      layout(location=3) in highp vec2 texCoord;
    #endif
#endif

#if GBUFFER_PASS == 1
    #if HAS_NORMAL_MAP == 1
      layout(location=4) in highp mat3 tangent_to_view_matrix;
    #endif
#else
    #if HAS_LIGHT_MAP == 1
      layout(location=5) in mediump vec2    lightmap_uv;
    #endif

    #if ENABLE_PER_PIXEL == 1
        layout(location=6) in mediump vec3    lightVec;
        layout(location=7) in mediump vec3    halfVec;

        #if HAS_SPEC_MAP_OFFSET == 1 || HAS_SPEC_MAP_SCALE == 1
          layout(location = 8) in highp vec2 spec_uv;
        #endif

        #if HAS_REFLECTION_MAP_OFFSET == 1 || HAS_REFLECTION_MAP_SCALE == 1
          layout(location = 9) in highp vec2 reflection_uv;
        #endif

        #if SHADOW_QUALITY >= 1
          layout(location = 10) in highp vec4  shadowMapCoord1;
        #endif

        #if SHADOW_QUALITY >= 2
          layout(location = 11) in highp vec4  shadowMapCoord2;
        #endif

        #if SHADOW_QUALITY >= 3
          layout(location = 12) in highp vec4  shadowMapCoord3;
        #endif

    #else
      layout(location = 13) in mediump float   lamberFactor;
      layout(location = 14) in mediump float   specularFactor;
    #endif

    #if ENABLE_RIM_COLOR == 1
        #define NEED_EYEVEC
    #endif

    #if HAS_REFLECTION_CUBE_MAP == 1
        #if HAS_NORMAL_MAP == 1
            #define NEED_EYEVEC
          layout(location = 15) in highp mat3 tangent_to_world_matrix;
        #else
          layout(location = 16) in mediump vec3 reflectionVec;
        #endif
    #endif

    #ifdef NEED_EYEVEC
      layout(location = 17) in mediump vec3 eyeVec;
    #endif

    #if HAS_DIFFUSE_MAP_OFFSET == 1 || HAS_DIFFUSE_MAP_SCALE == 1
      layout(location = 18) in highp vec2  diffuse_uv;
    #endif

#endif

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

void main()
{
    #if ALPHA_TEST == 1 && HAS_DIFFUSE_MAP == 1
        mediump vec4 diffuseMaterial = texture(diffuseTexture, diffuse_uv);
        if(diffuseMaterial.a < 0.5) discard;
    #endif
    
    #if GBUFFER_PASS == 1 && ALPHA_TEST == 1
        if(texture(diffuseTexture, diffuse_uv).a < 0.5) discard;
    #endif
    
    #if GBUFFER_PASS == 2 || GBUFFER_PASS == 3
        mediump vec2 gbuffer_uv = vec2(gl_FragCoord.xy / viewport.zw); // FINDME, TODO - Dependent Texture Read adding latency, due to calculation of texture UV within fragment -- move to vertex shader?
    #endif
    
    #if GBUFFER_PASS == 3
        lowp vec4 gbuffer_sample = texture(gbuffer_frame, gbuffer_uv);
        mediump vec3 gbuffer_lamber_factor = gbuffer_sample.rgb * 5.0;
        lowp float gbuffer_specular_factor = gbuffer_sample.a;
    #endif
    
    #if GBUFFER_PASS == 1
        #if HAS_NORMAL_MAP == 1
            // lookup normal from normal map, move from [0,1] to  [-1, 1] range, normalize
            mediump vec3 normal = normalize(2.0 * texture(normalTexture,normal_uv).rgb - 1.0);
            mediump vec3 view_space_normal = tangent_to_view_matrix * normal;
        #else
            mediump vec3 view_space_normal = vec3(model_view_inverse_transpose_matrix * vec4(normal, 1.0));
        #endif
        colorOut = vec4(view_space_normal * 0.5 + 0.5, PushConstants.material_shininess / 100.0);
    #else
        #if HAS_DIFFUSE_MAP == 1
            #if ALPHA_TEST == 1
                diffuseMaterial.a = 1.0;
            #else
                mediump vec4 diffuseMaterial = texture(diffuseTexture, diffuse_uv);
            #endif
            
        #else
            mediump vec4 diffuseMaterial = vec4(1.0);
        #endif
    
        #if ENABLE_PER_PIXEL == 1
            #if HAS_NORMAL_MAP == 1    
                // lookup normal from normal map, move from [0,1] to  [-1, 1] range, normalize
                mediump vec3 normal = normalize(2.0 * texture(normalTexture,normal_uv).rgb - 1.0);
            #endif
    
            #if GBUFFER_PASS == 3
                mediump vec3 lamberFactor = gbuffer_lamber_factor;
            #else
                mediump float lamberFactor = max(0.0,dot(lightVec, normal));
            #endif
            mediump float specularFactor = 0.0;
            if(PushConstants.material_shininess > 0.0) {
                #if GBUFFER_PASS == 3
                    specularFactor = gbuffer_specular_factor;
                #else
                    mediump float halfVecDot = dot(halfVec,normal);
                    if(halfVecDot > 0.0) {
                        specularFactor = max(0.0,pow(halfVecDot, PushConstants.material_shininess));
                    }
                #endif
            }

            #ifdef GL_EXT_shadow_samplers
                #if SHADOW_QUALITY == 1
                    lowp float shadow = shadow2DProjEXT(shadowTexture1, shadowMapCoord1);
                    lamberFactor *= shadow;
                    specularFactor *= shadow;
                #endif
            #else
    
                #if SHADOW_QUALITY == 1

                        highp float shadowMapDepth = 1.0;
                        highp float vertexShadowDepth = 1.0;
                        highp vec2 shadowMapPos = (shadowMapCoord1 / shadowMapCoord1.w).st;
                        
                        if(shadowMapCoord1.x >= -1.0 && shadowMapCoord1.x <= 1.0 && shadowMapCoord1.y >= -1.0 && shadowMapCoord1.y <= 1.0 && shadowMapCoord1.z >= 0.0 && shadowMapCoord1.z <= 1.0) {
                        #if DEBUG_PSSM == 1
                                diffuseMaterial = diffuseMaterial * vec4(0.75, 0.75, 0.5, 1.0) + vec4(0.0, 0.0, 0.5, 0.0);
                        #endif
                            shadowMapDepth =  texture(shadowTexture1, shadowMapPos).z;
                            vertexShadowDepth = (shadowMapCoord1 / shadowMapCoord1.w).z;
                        }
                #endif

                #if SHADOW_QUALITY >= 2

                    highp float shadowMapDepth = 1.0;
                    highp float vertexShadowDepth = 1.0;
                    
                    if(shadowMapCoord1.x >= -1.0 && shadowMapCoord1.x <= 1.0 && shadowMapCoord1.y >= -1.0 && shadowMapCoord1.y <= 1.0 && shadowMapCoord1.z >= 0.0 && shadowMapCoord1.z <= 1.0) {
                        #if DEBUG_PSSM == 1
                            diffuseMaterial = diffuseMaterial * vec4(0.75, 0.75, 0.5, 1.0) + vec4(0.0, 0.0, 0.5 * diffuseMaterial.a, 0.0);
                        #endif
                        highp vec2 shadowMapPos = (shadowMapCoord1 / shadowMapCoord1.w).st;
                        shadowMapDepth =  texture(shadowTexture1, shadowMapPos).z;
                        vertexShadowDepth = (shadowMapCoord1 / shadowMapCoord1.w).z;
                    } else if(shadowMapCoord2.s >= -1.0 && shadowMapCoord2.s <= 1.0 && shadowMapCoord2.t >= -1.0 && shadowMapCoord2.t <= 1.0 && shadowMapCoord2.z >= 0.0 && shadowMapCoord2.z <= 1.0) {
                        #if DEBUG_PSSM == 1
                            diffuseMaterial = diffuseMaterial * vec4(0.75, 0.50, 0.75, 1.0) + vec4(0.0, 0.5 * diffuseMaterial.a, 0.0, 0.0);
                        #endif
                        highp vec2 shadowMapPos = (shadowMapCoord2 / shadowMapCoord2.w).st;
                        shadowMapDepth =  texture(shadowTexture2, shadowMapPos).z;
                        vertexShadowDepth = (shadowMapCoord2 / shadowMapCoord2.w).z;
                    }
                    #if SHADOW_QUALITY >= 3
                        else if(shadowMapCoord3.s >= -1.0 && shadowMapCoord3.s <= 1.0 && shadowMapCoord3.t >= -1.0 && shadowMapCoord3.t <= 1.0 && shadowMapCoord3.z >= 0.0 && shadowMapCoord3.z <= 1.0) {
                            #if DEBUG_PSSM == 1
                                diffuseMaterial = diffuseMaterial * vec4(0.50, 0.75, 0.75, 1.0) + vec4(0.5 * diffuseMaterial.a, 0.0, 0.0, 0.0);
                            #endif
                            highp vec2 shadowMapPos = (shadowMapCoord3 / shadowMapCoord3.w).st;
                            shadowMapDepth =  texture(shadowTexture3, shadowMapPos).z;
                            vertexShadowDepth = (shadowMapCoord3 / shadowMapCoord3.w).z;
                        }

                    #endif
                #endif
                        
                #if SHADOW_QUALITY >= 1
                    if(vertexShadowDepth >= shadowMapDepth && shadowMapDepth < 1.0) {
                        #if GBUFFER_PASS == 3
                            lamberFactor = vec3(0.0);
                        #else
                            lamberFactor = 0.0;
                        #endif
                        specularFactor = 0.0;
                    }
                #endif
            #endif
        #endif
            
        #if ENABLE_AMBIENT == 1
            // -------------------- Add ambient light and alpha component --------------------
            colorOut = vec4(vec3(diffuseMaterial) * material_ambient, 0.0);
        #else
            colorOut = vec4(0.0, 0.0, 0.0, 0.0);
        #endif
            
        #if ENABLE_DIFFUSE == 1
            // -------------------- Add diffuse light --------------------
            colorOut += diffuseMaterial * vec4(PushConstants.material_diffuse * lamberFactor, 1.0);
        #endif

        // -------------------- Apply material_alpha --------------------
    
        #if ALPHA_BLEND == 1
            colorOut.a = diffuseMaterial.a;
            colorOut *= material_alpha;
        #endif
    
        // -------------------- Add specular light --------------------
        // Additive, not masked against diffuse alpha
        #if ENABLE_SPECULAR == 1
            #if HAS_SPEC_MAP == 1 && ENABLE_PER_PIXEL == 1
                colorOut.rgb += material_specular * vec3(texture(specularTexture, spec_uv)) * specularFactor;
            #else
                colorOut.rgb += material_specular * specularFactor;
            #endif    
        #endif

        // -------------------- Multiply light map --------------------
        #if HAS_LIGHT_MAP == 1
            mediump vec3 lightMapColor = vec3(texture(lightmapTexture, lightmap_uv));
            //colorOut = vec4(colorOut.r * lightMapColor.r, colorOut.g * lightMapColor.g, colorOut.b * lightMapColor.b, colorOut.a);
            colorOut.rgb *= lightMapColor;
        #endif
    
    
        // -------------------- Add reflected light --------------------
        #if HAS_REFLECTION_CUBE_MAP == 1
            // Reflected light is additive and not modulated by the light map
            #if HAS_NORMAL_MAP == 1
                // Calculate reflection vector as I - 2.0 * dot(N, I) * N
                mediump vec3 incidenceVec = -normalize(eyeVec);
                highp vec3 world_space_normal = tangent_to_world_matrix * normal;
                mediump vec3 reflectionVec = mat3(model_matrix) * (incidenceVec - 2.0 * dot(world_space_normal, incidenceVec) * world_space_normal);
            #endif
            #if HAS_REFLECTION_MAP == 1
                colorOut += vec4(material_reflection, 0.0) * texture(reflectionTexture, reflection_uv) * vec4(texture(reflectionCubeTexture, reflectionVec).rgb, 1.0);
            #else
                colorOut += vec4(material_reflection, 0.0) * vec4(texture(reflectionCubeTexture, reflectionVec).rgb, 1.0);
            #endif
        #endif
    
        // -------------------- Apply Fog --------------------
        #if FOG_TYPE == 1 || FOG_TYPE == 2 || FOG_TYPE == 3
            
            #if FOG_TYPE == 1
                // Linear fog
                lowp float fog_alpha = clamp((fog_far - gl_FragCoord.z / gl_FragCoord.w) * fog_scale, 0.0, 1.0);
            #endif
                
            #if FOG_TYPE == 2
                // Exponential fog
                mediump float fog_z = gl_FragCoord.z / gl_FragCoord.w - fog_near;
                lowp float fog_alpha = clamp(exp2(fog_density_premultiplied_exponential * fog_z), 0.0, 1.0);
            #endif
                
            #if FOG_TYPE == 3
                // Exponential squared fog
                mediump float fog_z = max(gl_FragCoord.z / gl_FragCoord.w - fog_near, 0.0);
                lowp float fog_alpha = clamp(exp2(fog_density_premultiplied_squared * fog_z * fog_z), 0.0, 1.0);
            #endif
    
            #if ALPHA_BLEND == 1
                colorOut.rgb = mix(fog_color.rgb * colorOut.a, colorOut.rgb, fog_alpha);
            #else
                colorOut.rgb = mix(fog_color.rgb, colorOut.rgb, fog_alpha);
            #endif
        #endif
    

    #endif
    
    #if ENABLE_RIM_COLOR == 1
        lowp float rim = 1.0 - clamp(dot(normalize(eyeVec), normal), 0.0, 1.0);
        
        colorOut += vec4(rim_color, 1.0) * pow(rim, rim_power);
    #endif
    
    #if BONE_COUNT > 0
        colorOut.b = 1.0;
    #endif

    colorOut.a = 1.0; // HACK?
}
