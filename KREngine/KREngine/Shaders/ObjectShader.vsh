//
//  Shader.vsh
//  KREngine
//
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

attribute highp vec3	vertex_position, vertex_normal, vertex_tangent;
attribute mediump vec2	vertex_uv;
uniform highp mat4      mvp_matrix; // mvp_matrix is the result of multiplying the model, view, and projection matrices 

#if ENABLE_PER_PIXEL == 1 || GBUFFER_PASS == 1
    #if HAS_DIFFUSE_MAP == 1 || HAS_NORMAL_MAP == 1 || HAS_SPEC_MAP == 1 || HAS_REFLECTION_MAP == 1
        varying highp vec2 texCoord;
    #endif
    #if HAS_NORMAL_MAP == 1
        #if HAS_NORMAL_MAP_SCALE == 1
            uniform highp vec2 normalTexture_Scale;
        #endif

        #if HAS_NORMAL_MAP_OFFSET == 1
            uniform highp vec2 normalTexture_Offset;
        #endif

        #if HAS_NORMAL_MAP_OFFSET == 1 || HAS_NORMAL_MAP_SCALE == 1
            varying highp vec2 normal_uv;
        #endif
    #else
        varying mediump vec3 normal;
    #endif
#else
    uniform mediump float material_shininess;
    #if HAS_DIFFUSE_MAP == 1
        varying highp vec2 texCoord;
    #endif
#endif

#if GBUFFER_PASS == 1
    #if HAS_NORMAL_MAP == 1
        uniform highp mat4 model_view_inverse_transpose_matrix;
        varying highp mat3 tangent_to_view_matrix;
    #endif
#else

    uniform highp vec3  light_direction; // Must be normalized before entering shader
    uniform highp vec3  cameraPosition;

    #if HAS_LIGHT_MAP == 1
        attribute mediump vec2  vertex_lightmap_uv;
        varying mediump vec2    lightmap_uv;
    #endif

    #if ENABLE_PER_PIXEL == 1
        varying mediump vec3    lightVec;
        varying mediump vec3    halfVec;

        #if HAS_SPEC_MAP_OFFSET == 1 || HAS_SPEC_MAP_SCALE == 1
            varying highp vec2 spec_uv;
        #endif

        #if HAS_SPEC_MAP_SCALE == 1
            uniform highp vec2    specularTexture_Scale;
        #endif

        #if HAS_SPEC_MAP_OFFSET == 1
            uniform highp vec2    specularTexture_Offset;
        #endif

        #if HAS_REFLECTION_MAP_OFFSET == 1 || HAS_REFLECTION_MAP_SCALE == 1
            varying highp vec2 reflection_uv;
        #endif

        #if HAS_REFLECTION_MAP_SCALE == 1
            uniform highp vec2    reflection_Scale;
        #endif

        #if HAS_REFLECTION_MAP_OFFSET == 1
            uniform highp vec2    reflection_Offset;
        #endif

        #if SHADOW_QUALITY >= 1
            uniform highp mat4  shadow_mvp1;
            varying highp vec4	shadowMapCoord1;
        #endif

        #if SHADOW_QUALITY >= 2
            uniform highp mat4  shadow_mvp2;
            varying highp vec4	shadowMapCoord2;
        #endif

        #if SHADOW_QUALITY >= 3
            uniform highp mat4  shadow_mvp3;
            varying highp vec4	shadowMapCoord3;
        #endif

    #else
        varying mediump float   lamberFactor;
        varying mediump float   specularFactor;
    #endif


    #if HAS_REFLECTION_CUBE_MAP == 1
        #if HAS_NORMAL_MAP == 1
            uniform highp mat4 model_inverse_transpose_matrix;
            varying mediump vec3 eyeVec;
            varying highp mat3 tangent_to_world_matrix;
        #else
            uniform highp mat4 model_matrix;
            varying mediump vec3 reflectionVec;
        #endif
    #endif

    #if HAS_DIFFUSE_MAP_SCALE == 1
        uniform highp vec2  diffuseTexture_Scale;
    #endif

    #if HAS_DIFFUSE_MAP_OFFSET == 1
        uniform highp vec2  diffuseTexture_Offset;
    #endif

    #if HAS_DIFFUSE_MAP_OFFSET == 1 || HAS_DIFFUSE_MAP_SCALE == 1
        varying highp vec2  diffuse_uv;
    #endif

#endif



void main()
{
    // Transform position
    gl_Position = mvp_matrix * vec4(vertex_position,1.0);
    
    #if HAS_DIFFUSE_MAP == 1 || (HAS_NORMAL_MAP == 1 && ENABLE_PER_PIXEL == 1) || (HAS_SPEC_MAP == 1 && ENABLE_PER_PIXEL == 1) || (HAS_REFLECTION_MAP == 1 && ENABLE_PER_PIXEL == 1)
        // Pass UV co-ordinates
        texCoord = vertex_uv.st;
    #endif
    

    

    // Scaled and translated normal map UV's
    #if (HAS_NORMAL_MAP_OFFSET == 1 || HAS_NORMAL_MAP_SCALE == 1) && ENABLE_PER_PIXEL == 1
        normal_uv = texCoord;
        
        #if HAS_NORMAL_MAP_OFFSET == 1
            normal_uv + normalTexture_Offset;
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
                diffuse_uv + diffuseTexture_Offset;
            #endif
                
            #if HAS_DIFFUSE_MAP_SCALE == 1
                diffuse_uv *= diffuseTexture_Scale;
            #endif
        #endif
    #endif
    
    
    #if GBUFFER_PASS == 1
        #if HAS_NORMAL_MAP == 1
            mediump vec3 a_bitangent = cross(vertex_normal, vertex_tangent);
            tangent_to_view_matrix[0] = vec3(model_view_inverse_transpose_matrix * vec4(vertex_tangent, 1.0));
            tangent_to_view_matrix[1] = vec3(model_view_inverse_transpose_matrix * vec4(a_bitangent, 1.0));
            tangent_to_view_matrix[2] = vec3(model_view_inverse_transpose_matrix * vec4(vertex_normal, 1.0));
        #else
            normal = vertex_normal;
        #endif
    #else

        #if HAS_REFLECTION_CUBE_MAP == 1
            #if HAS_NORMAL_MAP == 1
                eyeVec = normalize(cameraPosition - vertex_position);
            #else
                // Calculate reflection vector as I - 2.0 * dot(N, I) * N
                mediump vec3 eyeVec = normalize(cameraPosition - vertex_position);
                mediump vec3 incidenceVec = -eyeVec;
                reflectionVec = mat3(model_matrix) * (incidenceVec - 2.0 * dot(vertex_normal, incidenceVec) * vertex_normal);
            #endif
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
                    spec_uv + specularTexture_Offset;
                #endif
                    
                #if HAS_SPEC_MAP_SCALE == 1
                    spec_uv *= specularTexture_Scale;
                #endif
            #endif
    
            // Scaled and translated reflection map UV's
            #if HAS_REFLECTION_MAP_OFFSET == 1 || HAS_REFLECTION_MAP_SCALE == 1
                reflection_uv = texCoord;
                #if HAS_REFLECTION_MAP_OFFSET == 1
                    reflection_uv + reflectionTexture_Offset;
                #endif
                    
                #if HAS_REFLECTION_MAP_SCALE == 1
                    reflection_uv *= reflectionTexture_Scale;
                #endif
            #endif
    
    
            #if SHADOW_QUALITY >= 1
                shadowMapCoord1 = shadow_mvp1 * vec4(vertex_position,1.0);
            #endif
    
            #if SHADOW_QUALITY >= 2
                shadowMapCoord2 = shadow_mvp2 * vec4(vertex_position,1.0);
            #endif
                
            #if SHADOW_QUALITY >= 3
                shadowMapCoord3 = shadow_mvp3 * vec4(vertex_position,1.0);
            #endif

            // ----------- Directional Light (Sun) -----------
            #if HAS_NORMAL_MAP == 1
                // ----- Calculate per-pixel lighting in tangent space, for normal mapping ------
                mediump vec3 a_bitangent = cross(vertex_normal, vertex_tangent);
                #if HAS_REFLECTION_CUBE_MAP == 0
                    // The cube map reflections also require an eyeVec as a varying attribute when normal mapping, so only re-calculate here when needed
                    mediump vec3 eyeVec = normalize(cameraPosition - vertex_position);
                #else
                    tangent_to_world_matrix[0] = vec3(model_inverse_transpose_matrix * vec4(vertex_tangent, 1.0));
                    tangent_to_world_matrix[1] = vec3(model_inverse_transpose_matrix * vec4(a_bitangent, 1.0));
                    tangent_to_world_matrix[2] = vec3(model_inverse_transpose_matrix * vec4(vertex_normal, 1.0));
                #endif
    
                lightVec = normalize(vec3(dot(light_direction, vertex_tangent), dot(light_direction, a_bitangent), dot(light_direction, vertex_normal)));
                halfVec = normalize(vec3(dot(eyeVec, vertex_tangent), dot(eyeVec, a_bitangent), dot(eyeVec, vertex_normal)));
                halfVec = normalize(halfVec + lightVec); // Normalizing anyways, no need to divide by 2
            #else
                // ------ Calculate per-pixel lighting without normal mapping ------
                normal = vertex_normal;
                lightVec = light_direction;
                halfVec = normalize((normalize(cameraPosition - vertex_position) + lightVec)); // Normalizing anyways, no need to divide by 2
            #endif
        #else
    
            // ------ Calculate per-vertex lighting ------
            mediump vec3 halfVec = normalize((normalize(cameraPosition - vertex_position) + light_direction)); // Normalizing anyways, no need to divide by 2
            lamberFactor = max(0.0,dot(light_direction, vertex_normal));
            specularFactor = max(0.0,pow(dot(halfVec,vertex_normal), material_shininess));
        #endif
    #endif
}
