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


#if ENABLE_PER_PIXEL == 1 || GBUFFER_PASS == 1
    uniform mediump float material_shininess;
    #if HAS_NORMAL_MAP == 1
        uniform sampler2D normalTexture;
    #else
        varying mediump vec3 normal;
    #endif

    #if HAS_DIFFUSE_MAP == 1 || HAS_NORMAL_MAP == 1 || HAS_SPEC_MAP == 1
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
        uniform highp mat4 model_to_view_matrix;
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

    #if SHADOW_QUALITY >= 1
        uniform sampler2D   shadowTexture1;
        varying highp vec4  shadowMapCoord1;
    #endif

    #if HAS_LIGHT_MAP == 1
        uniform sampler2D     shadowTexture1;
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
    #if GBUFFER_PASS == 2 || GBUFFER_PASS == 3
        mediump vec2 gbuffer_uv = vec2(gl_FragCoord.xy / viewport.zw);
    #endif
    
    #if GBUFFER_PASS == 2
        lowp vec4 gbuffer_sample = texture2D(gbuffer_frame, gbuffer_uv);
        mediump vec3 gbuffer_normal = normalize(2.0 * gbuffer_sample.rgb - 1.0);
        mediump float gbuffer_specular_exponent = gbuffer_sample.a;
    #endif
    
    #if GBUFFER_PASS == 3
        lowp vec4 gbuffer_sample = texture2D(gbuffer_frame, gbuffer_uv);
        lowp vec3 gbuffer_lamber_factor = gbuffer_sample.rgb;
        lowp float gbuffer_specular_factor = gbuffer_sample.a;
    #endif
    
    #if GBUFFER_PASS == 1
        #if HAS_NORMAL_MAP == 1
            // lookup normal from normal map, move from [0,1] to  [-1, 1] range, normalize
            mediump vec3 normal = normalize(2.0 * texture2D(normalTexture,normal_uv).rgb - 1.0);
            mediump vec3 view_space_normal = tangent_to_view_matrix * normal;
        #else
            mediump vec3 view_space_normal = vec3(model_to_view_matrix * vec4(normal, 1.0));
        #endif
        gl_FragColor = vec4(view_space_normal * 0.5 + 0.5, material_shininess / 100.0);
    #else
        #if HAS_DIFFUSE_MAP == 1
            mediump vec4 diffuseMaterial = vec4(vec3(texture2D(diffuseTexture, diffuse_uv)), material_alpha);
        #else
            mediump vec4 diffuseMaterial = vec4(vec3(1.0), material_alpha);
        #endif
    
        #if ENABLE_PER_PIXEL == 1
            #if HAS_NORMAL_MAP == 1    
                // lookup normal from normal map, move from [0,1] to  [-1, 1] range, normalize
                mediump vec3 normal = normalize(2.0 * texture2D(normalTexture,normal_uv).rgb - 1.0);
            #endif
    
            #if GBUFFER_PASS == 3
                mediump float lamberFactor = gbuffer_lamber_factor.r;
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

            #if SHADOW_QUALITY == 1
                highp float shadowMapDepth = 1.0;
                highp float vertexShadowDepth = 1.0;
                highp vec2 shadowMapPos = ((shadowMapCoord1 / shadowMapCoord1.w + 1.0) / 2.0).st;
                
                if(shadowMapCoord1.x >= -1.0 && shadowMapCoord1.x <= 1.0 && shadowMapCoord1.y >= -1.0 && shadowMapCoord1.y <= 1.0 && shadowMapCoord1.z >= 0.0 && shadowMapCoord1.z <= 1.0) {
                #if DEBUG_PSSM == 1
                        diffuseMaterial = diffuseMaterial * vec4(0.75, 0.75, 0.5, 1.0) + vec4(0.0, 0.0, 0.5, 0.0);
                #endif
                    shadowMapDepth =  texture2D(shadowTexture1, shadowMapPos).z;
                    vertexShadowDepth = ((shadowMapCoord1 / shadowMapCoord1.w + 1.0) / 2.0).z;
                }
            #endif

            #if SHADOW_QUALITY >= 2

                highp float shadowMapDepth = 1.0;
                highp float vertexShadowDepth = 1.0;
                
                if(shadowMapCoord1.x >= -1.0 && shadowMapCoord1.x <= 1.0 && shadowMapCoord1.y >= -1.0 && shadowMapCoord1.y <= 1.0 && shadowMapCoord1.z >= 0.0 && shadowMapCoord1.z <= 1.0) {
                    #if DEBUG_PSSM == 1
                        diffuseMaterial = diffuseMaterial * vec4(0.75, 0.75, 0.5, 1.0) + vec4(0.0, 0.0, 0.5, 0.0);
                    #endif
                    highp vec2 shadowMapPos = ((shadowMapCoord1 / shadowMapCoord1.w + 1.0) / 2.0).st;
                    shadowMapDepth =  texture2D(shadowTexture1, shadowMapPos).z;
                    vertexShadowDepth = ((shadowMapCoord1 / shadowMapCoord1.w + 1.0) / 2.0).z;
                } else if(shadowMapCoord2.s >= -1.0 && shadowMapCoord2.s <= 1.0 && shadowMapCoord2.t >= -1.0 && shadowMapCoord2.t <= 1.0 && shadowMapCoord2.z >= 0.0 && shadowMapCoord2.z <= 1.0) {
                    #if DEBUG_PSSM == 1
                        diffuseMaterial = diffuseMaterial * vec4(0.75, 0.50, 0.75, 1.0) + vec4(0.0, 0.5, 0.0, 0.0);
                    #endif
                    highp vec2 shadowMapPos = ((shadowMapCoord2 / shadowMapCoord2.w + 1.0) / 2.0).st;
                    shadowMapDepth =  texture2D(shadowTexture2, shadowMapPos).z;
                    vertexShadowDepth = ((shadowMapCoord2 / shadowMapCoord2.w + 1.0) / 2.0).z;
                }
                #if SHADOW_QUALITY >= 3
                    else if(shadowMapCoord3.s >= -1.0 && shadowMapCoord3.s <= 1.0 && shadowMapCoord3.t >= -1.0 && shadowMapCoord3.t <= 1.0 && shadowMapCoord3.z >= 0.0 && shadowMapCoord3.z <= 1.0) {
                        #if DEBUG_PSSM == 1
                            diffuseMaterial = diffuseMaterial * vec4(0.50, 0.75, 0.75, 1.0) + vec4(0.5, 0.0, 0.0, 0.0);
                        #endif
                        highp vec2 shadowMapPos = ((shadowMapCoord3 / shadowMapCoord3.w + 1.0) / 2.0).st;
                        shadowMapDepth =  texture2D(shadowTexture3, shadowMapPos).z;
                        vertexShadowDepth = ((shadowMapCoord3 / shadowMapCoord3.w + 1.0) / 2.0).z;
                    }

                #endif
            #endif
                    
            #if SHADOW_QUALITY >= 1
                if(vertexShadowDepth >= shadowMapDepth && shadowMapDepth < 1.0) {
                    lamberFactor = 0.0;
                    specularFactor = 0.0;
                }
            #endif
        #endif
            
        #if ENABLE_AMBIENT
            // -------------------- Add ambient light and alpha component --------------------
            gl_FragColor = vec4(vec3(diffuseMaterial) * material_ambient, material_alpha);
        #else
            gl_FragColor = vec4(0.0, 0.0, 0.0, material_alpha);
        #endif
            
        #if ENABLE_DIFFUSE
            // -------------------- Add diffuse light --------------------
            gl_FragColor += vec4(vec3(diffuseMaterial) * material_diffuse * lamberFactor, 0.0);
        #endif
            
        #if ENABLE_SPECULAR
            
            // -------------------- Add specular light --------------------
            #if HAS_SPEC_MAP == 1 && ENABLE_PER_PIXEL == 1
                gl_FragColor += vec4(material_specular * vec3(texture2D(specularTexture, spec_uv)) * specularFactor, 0.0);
            #else
                gl_FragColor += vec4(material_specular * specularFactor, 0.0);
            #endif
            
        #endif


            // -------------------- Multiply light map -------------------- 
            
        #if HAS_LIGHT_MAP
            mediump vec3 lightMapColor = vec3(texture2D(shadowTexture1, lightmap_uv));
            gl_FragColor = vec4(gl_FragColor.r * lightMapColor.r, gl_FragColor.g * lightMapColor.g, gl_FragColor.b * lightMapColor.b, 1.0);
        #endif
    
        // gl_FragColor = vec4(vec3(specularFactor), 1.0);
    
    #endif
}
