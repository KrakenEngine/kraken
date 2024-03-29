//
//  ObjectShader_osx.vsh
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



in highp vec3	vertex_position, vertex_normal;
#if HAS_NORMAL_MAP == 1
    in highp vec3    vertex_tangent;
#endif
in mediump vec2	vertex_uv;
uniform highp mat4      mvp_matrix; // mvp_matrix is the result of multiplying the model, view, and projection matrices

#if BONE_COUNT > 0
    in highp vec4 bone_weights;
    in highp vec4 bone_indexes;
    uniform highp mat4 bone_transforms[BONE_COUNT];
#else
    #define vertex_position_skinned vertex_position
    #define vertex_normal_skinned vertex_normal
    #define vertex_tangent_skinned vertex_tangent
#endif



#if ENABLE_PER_PIXEL == 1 || GBUFFER_PASS == 1
    #if HAS_DIFFUSE_MAP == 1 || HAS_NORMAL_MAP == 1 || HAS_SPEC_MAP == 1 || HAS_REFLECTION_MAP == 1
        out highp vec2 texCoord;
    #endif
    #if HAS_NORMAL_MAP == 1
        #if HAS_NORMAL_MAP_SCALE == 1
            uniform highp vec2 normalTexture_Scale;
        #endif

        #if HAS_NORMAL_MAP_OFFSET == 1
            uniform highp vec2 normalTexture_Offset;
        #endif

        #if HAS_NORMAL_MAP_OFFSET == 1 || HAS_NORMAL_MAP_SCALE == 1
            out highp vec2 normal_uv;
        #endif
    #else
        out mediump vec3 normal;
    #endif
#else
    uniform mediump float material_shininess;
    #if HAS_DIFFUSE_MAP == 1
        out highp vec2 texCoord;
    #endif
#endif

#if GBUFFER_PASS == 1
    #if HAS_NORMAL_MAP == 1
        uniform highp mat4 model_view_inverse_transpose_matrix;
        out highp mat3 tangent_to_view_matrix;
    #endif
#else

    uniform highp vec3  light_direction_model_space; // Must be normalized before entering shader
    uniform highp vec3  camera_position_model_space;

    #if HAS_LIGHT_MAP == 1
        in mediump vec2  vertex_lightmap_uv;
        out mediump vec2    lightmap_uv;
    #endif

    #if ENABLE_PER_PIXEL == 1
        out mediump vec3    lightVec;
        out mediump vec3    halfVec;

        #if HAS_SPEC_MAP_OFFSET == 1 || HAS_SPEC_MAP_SCALE == 1
            out highp vec2 spec_uv;
        #endif

        #if HAS_SPEC_MAP_SCALE == 1
            uniform highp vec2    specularTexture_Scale;
        #endif

        #if HAS_SPEC_MAP_OFFSET == 1
            uniform highp vec2    specularTexture_Offset;
        #endif

        #if HAS_REFLECTION_MAP_OFFSET == 1 || HAS_REFLECTION_MAP_SCALE == 1
            out highp vec2 reflection_uv;
        #endif

        #if HAS_REFLECTION_MAP_SCALE == 1
            uniform highp vec2    reflectionTexture_Scale;
        #endif

        #if HAS_REFLECTION_MAP_OFFSET == 1
            uniform highp vec2    reflectionTexture_Offset;
        #endif

        #if SHADOW_QUALITY >= 1
            uniform highp mat4  shadow_mvp1;
            out highp vec4	shadowMapCoord1;
        #endif

        #if SHADOW_QUALITY >= 2
            uniform highp mat4  shadow_mvp2;
            out highp vec4	shadowMapCoord2;
        #endif

        #if SHADOW_QUALITY >= 3
            uniform highp mat4  shadow_mvp3;
            out highp vec4	shadowMapCoord3;
        #endif

    #else
        out mediump float   lamberFactor;
        out mediump float   specularFactor;
    #endif

    #if ENABLE_RIM_COLOR == 1
        #define NEED_EYEVEC
    #endif

    #if HAS_REFLECTION_CUBE_MAP == 1
        #if HAS_NORMAL_MAP == 1
            #define NEED_EYEVEC
            uniform highp mat4 model_inverse_transpose_matrix;
            out highp mat3 tangent_to_world_matrix;
        #else
            uniform highp mat4 model_matrix;
            out mediump vec3 reflectionVec;
        #endif
    #endif

    #ifdef NEED_EYEVEC
        out mediump vec3 eyeVec;
    #endif

    #if HAS_DIFFUSE_MAP_SCALE == 1
        uniform highp vec2  diffuseTexture_Scale;
    #endif

    #if HAS_DIFFUSE_MAP_OFFSET == 1
        uniform highp vec2  diffuseTexture_Offset;
    #endif

    #if HAS_DIFFUSE_MAP_OFFSET == 1 || HAS_DIFFUSE_MAP_SCALE == 1
        out highp vec2  diffuse_uv;
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
    gl_Position = mvp_matrix * vec4(vertex_position_skinned,1.0);


    
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
                diffuse_uv += diffuseTexture_Offset;
            #endif
                
            #if HAS_DIFFUSE_MAP_SCALE == 1
                diffuse_uv *= diffuseTexture_Scale;
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
                mediump vec3 eyeVec = normalize(camera_position_model_space - vertex_position_skinned);
                mediump vec3 incidenceVec = -eyeVec;
                reflectionVec = mat3(model_matrix) * (incidenceVec - 2.0 * dot(vertex_normal_skinned, incidenceVec) * vertex_normal_skinned);
            #endif
        #endif
    
        #ifdef NEED_EYEVEC
            eyeVec = normalize(camera_position_model_space - vertex_position_skinned);
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
                    spec_uv += specularTexture_Offset;
                #endif
                    
                #if HAS_SPEC_MAP_SCALE == 1
                    spec_uv *= specularTexture_Scale;
                #endif
            #endif
    
            // Scaled and translated reflection map UV's
            #if HAS_REFLECTION_MAP_OFFSET == 1 || HAS_REFLECTION_MAP_SCALE == 1
                reflection_uv = texCoord;
                #if HAS_REFLECTION_MAP_OFFSET == 1
                    reflection_uv += reflectionTexture_Offset;
                #endif
                    
                #if HAS_REFLECTION_MAP_SCALE == 1
                    reflection_uv *= reflectionTexture_Scale;
                #endif
            #endif
    
    
            #if SHADOW_QUALITY >= 1
                shadowMapCoord1 = shadow_mvp1 * vec4(vertex_position_skinned,1.0);
            #endif
    
            #if SHADOW_QUALITY >= 2
                shadowMapCoord2 = shadow_mvp2 * vec4(vertex_position_skinned,1.0);
            #endif
                
            #if SHADOW_QUALITY >= 3
                shadowMapCoord3 = shadow_mvp3 * vec4(vertex_position_skinned,1.0);
            #endif

            // ----------- Directional Light (Sun) -----------
            #if HAS_NORMAL_MAP == 1
                // ----- Calculate per-pixel lighting in tangent space, for normal mapping ------
                mediump vec3 a_bitangent = cross(vertex_normal_skinned, vertex_tangent_skinned);
                #if HAS_REFLECTION_CUBE_MAP == 0
                    // The cube map reflections also require an eyeVec as a varying attribute when normal mapping, so only re-calculate here when needed
                    mediump vec3 eyeVec = normalize(camera_position_model_space - vertex_position_skinned);
                #else
                    tangent_to_world_matrix[0] = vec3(model_inverse_transpose_matrix * vec4(vertex_tangent_skinned, 1.0));
                    tangent_to_world_matrix[1] = vec3(model_inverse_transpose_matrix * vec4(a_bitangent, 1.0));
                    tangent_to_world_matrix[2] = vec3(model_inverse_transpose_matrix * vec4(vertex_normal_skinned, 1.0));
                #endif
    
                lightVec = normalize(vec3(dot(light_direction_model_space, vertex_tangent_skinned), dot(light_direction_model_space, a_bitangent), dot(light_direction_model_space, vertex_normal_skinned)));
                halfVec = normalize(vec3(dot(eyeVec, vertex_tangent_skinned), dot(eyeVec, a_bitangent), dot(eyeVec, vertex_normal_skinned)));
                halfVec = normalize(halfVec + lightVec); // Normalizing anyways, no need to divide by 2
            #else
                // ------ Calculate per-pixel lighting without normal mapping ------
                normal = vertex_normal_skinned;
                lightVec = light_direction_model_space;
                halfVec = normalize((normalize(camera_position_model_space - vertex_position_skinned) + lightVec)); // Normalizing anyways, no need to divide by 2
            #endif
        #else
    
            // ------ Calculate per-vertex lighting ------
            mediump vec3 halfVec = normalize((normalize(camera_position_model_space - vertex_position_skinned) + light_direction_model_space)); // Normalizing anyways, no need to divide by 2
            lamberFactor = max(0.0,dot(light_direction_model_space, vertex_normal_skinned));
            specularFactor = max(0.0,pow(dot(halfVec,vertex_normal_skinned), material_shininess));
        #endif
    #endif
}
