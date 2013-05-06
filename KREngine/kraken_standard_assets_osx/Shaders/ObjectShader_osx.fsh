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

// #extension GL_EXT_shadow_samplers : require

#if FOG_TYPE > 0
    // FOG_TYPE 1 - Linear
    // FOG_TYPE 2 - Exponential
    // FOG_TYPE 3 - Exponential squared
    uniform lowp vec3 fog_color;
    uniform mediump float fog_near;
    #if FOG_TYPE == 1
        uniform mediump float fog_far;
        uniform mediump float fog_scale;
    #endif

    #if FOG_TYPE > 1
        uniform mediump float fog_density;
    #endif

    #if FOG_TYPE == 2
        uniform mediump float fog_density_premultiplied_exponential;
    #endif
    #if FOG_TYPE == 3
        uniform mediump float fog_density_premultiplied_squared;
    #endif
#endif


#if ENABLE_PER_PIXEL == 1 || GBUFFER_PASS == 1
    uniform mediump float material_shininess;
    #if HAS_NORMAL_MAP == 1
        uniform sampler2D normalTexture;
    #else
        varying mediump vec3 normal;
    #endif

    #if HAS_DIFFUSE_MAP == 1 || HAS_NORMAL_MAP == 1 || HAS_SPEC_MAP == 1 || HAS_REFLECTION_MAP == 1
        varying highp vec2    texCoord;
    #endif
    #if HAS_NORMAL_MAP_OFFSET == 1 || HAS_NORMAL_MAP_SCALE == 1
        varying highp vec2  normal_uv;
    #else
        #define normal_uv texCoord
    #endif
#else
    #if HAS_DIFFUSE_MAP == 1
        varying highp vec2    texCoord;
    #endif
#endif


#if GBUFFER_PASS == 3
    uniform sampler2D gbuffer_frame;
    uniform sampler2D gbuffer_depth;
#endif

#if GBUFFER_PASS == 1
    #if HAS_NORMAL_MAP == 1
        varying highp mat3 tangent_to_view_matrix;
    #else
        uniform highp mat4 model_view_inverse_transpose_matrix;
    #endif

    #if HAS_DIFFUSE_MAP == 1 && ALPHA_TEST == 1
        uniform sampler2D 		diffuseTexture;
        #if HAS_DIFFUSE_MAP_OFFSET == 1 || HAS_DIFFUSE_MAP_SCALE == 1
            varying highp vec2  diffuse_uv;
        #else
            #define diffuse_uv texCoord
        #endif
    #endif
#else
    uniform lowp vec3   material_ambient, material_diffuse, material_specular;
    uniform lowp float  material_alpha;


    #if HAS_DIFFUSE_MAP == 1
        uniform sampler2D 		diffuseTexture;
    #endif

    #if HAS_SPEC_MAP == 1
        uniform sampler2D 		specularTexture;
    #endif

    #if HAS_REFLECTION_MAP == 1
        uniform sampler2D 		reflectionTexture;
    #endif

    #if HAS_REFLECTION_CUBE_MAP == 1
        uniform lowp vec3       material_reflection;
        uniform samplerCube     reflectionCubeTexture;
        #if HAS_NORMAL_MAP == 1
            varying highp mat3 tangent_to_world_matrix;
            varying mediump vec3 eyeVec;
            uniform highp mat4 model_matrix;
        #else
            varying mediump vec3 reflectionVec;
        #endif
    #endif


    #if SHADOW_QUALITY >= 1
        #ifdef GL_EXT_shadow_samplers
            uniform sampler2DShadow   shadowTexture1;
        #else
            uniform sampler2D   shadowTexture1;
        #endif
        varying highp vec4  shadowMapCoord1;
    #endif

    #if HAS_LIGHT_MAP == 1
        uniform sampler2D     lightmapTexture;
        varying mediump vec2  lightmap_uv;
    #endif

    #if SHADOW_QUALITY >= 2
        uniform sampler2D   shadowTexture2;
        varying highp vec4	shadowMapCoord2;
    #endif

    #if SHADOW_QUALITY >= 3
        uniform sampler2D   shadowTexture3;
        varying highp vec4  shadowMapCoord3;
    #endif

    #if ENABLE_PER_PIXEL == 1
        varying mediump vec3    lightVec;
        varying mediump vec3    halfVec;
    #else
        varying mediump float   lamberFactor;
        varying mediump float   specularFactor;
    #endif

    #if (HAS_SPEC_MAP_OFFSET == 1|| HAS_SPEC_MAP_SCALE == 1) && ENABLE_PER_PIXEL == 1
        varying mediump vec2 spec_uv;
    #else
        #define spec_uv texCoord
    #endif

    #if (HAS_REFLECTION_MAP_OFFSET == 1|| HAS_REFLECTION_MAP_SCALE == 1) && ENABLE_PER_PIXEL == 1
        varying mediump vec2 reflection_uv;
    #else
        #define reflection_uv texCoord
    #endif

    #if HAS_DIFFUSE_MAP_OFFSET == 1 || HAS_DIFFUSE_MAP_SCALE == 1
        varying highp vec2  diffuse_uv;
    #else
        #define diffuse_uv texCoord
    #endif

#endif

#if GBUFFER_PASS == 1 || GBUFFER_PASS == 3
    uniform mediump vec4 viewport;
#endif

void main()
{
    #if ALPHA_TEST == 1 && HAS_DIFFUSE_MAP == 1
        mediump vec4 diffuseMaterial = texture2D(diffuseTexture, diffuse_uv);
        if(diffuseMaterial.a < 0.5) discard;
    #endif
    
    #if GBUFFER_PASS == 1 && ALPHA_TEST == 1
        if(texture2D(diffuseTexture, diffuse_uv).a < 0.5) discard;
    #endif
    
    #if GBUFFER_PASS == 2 || GBUFFER_PASS == 3
        mediump vec2 gbuffer_uv = vec2(gl_FragCoord.xy / viewport.zw); // FINDME, TODO - Dependent Texture Read adding latency, due to calculation of texture UV within fragment -- move to vertex shader?
    #endif
    
    #if GBUFFER_PASS == 3
        lowp vec4 gbuffer_sample = texture2D(gbuffer_frame, gbuffer_uv);
        mediump vec3 gbuffer_lamber_factor = gbuffer_sample.rgb * 5.0;
        lowp float gbuffer_specular_factor = gbuffer_sample.a;
    #endif
    
    #if GBUFFER_PASS == 1
        #if HAS_NORMAL_MAP == 1
            // lookup normal from normal map, move from [0,1] to  [-1, 1] range, normalize
            mediump vec3 normal = normalize(2.0 * texture2D(normalTexture,normal_uv).rgb - 1.0);
            mediump vec3 view_space_normal = tangent_to_view_matrix * normal;
        #else
            mediump vec3 view_space_normal = vec3(model_view_inverse_transpose_matrix * vec4(normal, 1.0));
        #endif
        gl_FragColor = vec4(view_space_normal * 0.5 + 0.5, material_shininess / 100.0);
    #else
        #if HAS_DIFFUSE_MAP == 1
            #if ALPHA_TEST == 1
                diffuseMaterial.a = 1.0;
            #else
                mediump vec4 diffuseMaterial = texture2D(diffuseTexture, diffuse_uv);
            #endif
            
        #else
            mediump vec4 diffuseMaterial = vec4(1.0);
        #endif
    
        #if ENABLE_PER_PIXEL == 1
            #if HAS_NORMAL_MAP == 1    
                // lookup normal from normal map, move from [0,1] to  [-1, 1] range, normalize
                mediump vec3 normal = normalize(2.0 * texture2D(normalTexture,normal_uv).rgb - 1.0);
            #endif
    
            #if GBUFFER_PASS == 3
                mediump vec3 lamberFactor = gbuffer_lamber_factor;
            #else
                mediump float lamberFactor = max(0.0,dot(lightVec, normal));
            #endif
            mediump float specularFactor = 0.0;
            if(material_shininess > 0.0) {
                #if GBUFFER_PASS == 3
                    specularFactor = gbuffer_specular_factor;   
                #else
                    specularFactor = max(0.0,pow(dot(halfVec,normal), material_shininess));
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
                            shadowMapDepth =  texture2D(shadowTexture1, shadowMapPos).z;
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
                        shadowMapDepth =  texture2D(shadowTexture1, shadowMapPos).z;
                        vertexShadowDepth = (shadowMapCoord1 / shadowMapCoord1.w).z;
                    } else if(shadowMapCoord2.s >= -1.0 && shadowMapCoord2.s <= 1.0 && shadowMapCoord2.t >= -1.0 && shadowMapCoord2.t <= 1.0 && shadowMapCoord2.z >= 0.0 && shadowMapCoord2.z <= 1.0) {
                        #if DEBUG_PSSM == 1
                            diffuseMaterial = diffuseMaterial * vec4(0.75, 0.50, 0.75, 1.0) + vec4(0.0, 0.5 * diffuseMaterial.a, 0.0, 0.0);
                        #endif
                        highp vec2 shadowMapPos = (shadowMapCoord2 / shadowMapCoord2.w).st;
                        shadowMapDepth =  texture2D(shadowTexture2, shadowMapPos).z;
                        vertexShadowDepth = (shadowMapCoord2 / shadowMapCoord2.w).z;
                    }
                    #if SHADOW_QUALITY >= 3
                        else if(shadowMapCoord3.s >= -1.0 && shadowMapCoord3.s <= 1.0 && shadowMapCoord3.t >= -1.0 && shadowMapCoord3.t <= 1.0 && shadowMapCoord3.z >= 0.0 && shadowMapCoord3.z <= 1.0) {
                            #if DEBUG_PSSM == 1
                                diffuseMaterial = diffuseMaterial * vec4(0.50, 0.75, 0.75, 1.0) + vec4(0.5 * diffuseMaterial.a, 0.0, 0.0, 0.0);
                            #endif
                            highp vec2 shadowMapPos = (shadowMapCoord3 / shadowMapCoord3.w).st;
                            shadowMapDepth =  texture2D(shadowTexture3, shadowMapPos).z;
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
            gl_FragColor = vec4(vec3(diffuseMaterial) * material_ambient, 0.0);
        #else
            gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
        #endif
            
        #if ENABLE_DIFFUSE == 1
            // -------------------- Add diffuse light --------------------
            gl_FragColor += diffuseMaterial * vec4(material_diffuse * lamberFactor, 1.0);
        #endif

        // -------------------- Apply material_alpha --------------------
    
        #if ALPHA_BLEND == 1
            gl_FragColor.a = diffuseMaterial.a;
            gl_FragColor *= material_alpha;
        #endif
    
        // -------------------- Add specular light --------------------
        // Additive, not masked against diffuse alpha
        #if ENABLE_SPECULAR == 1
            #if HAS_SPEC_MAP == 1 && ENABLE_PER_PIXEL == 1
                gl_FragColor.rgb += material_specular * vec3(texture2D(specularTexture, spec_uv)) * specularFactor;
            #else
                gl_FragColor.rgb += material_specular * specularFactor;
            #endif    
        #endif

        // -------------------- Multiply light map --------------------
        #if HAS_LIGHT_MAP == 1
            mediump vec3 lightMapColor = vec3(texture2D(lightmapTexture, lightmap_uv));
            //gl_FragColor = vec4(gl_FragColor.r * lightMapColor.r, gl_FragColor.g * lightMapColor.g, gl_FragColor.b * lightMapColor.b, gl_FragColor.a);
            gl_FragColor.rgb *= lightMapColor;
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
                gl_FragColor += vec4(material_reflection, 0.0) * texture2D(reflectionTexture, reflection_uv) * textureCube(reflectionCubeTexture, reflectionVec);
            #else
                gl_FragColor += vec4(material_reflection, 0.0) * textureCube(reflectionCubeTexture, reflectionVec);
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
                gl_FragColor.rgb = mix(fog_color.rgb * gl_FragColor.a, gl_FragColor.rgb, fog_alpha);
            #else
                gl_FragColor.rgb = mix(fog_color.rgb, gl_FragColor.rgb, fog_alpha);
            #endif
        #endif
    

    #endif
}
