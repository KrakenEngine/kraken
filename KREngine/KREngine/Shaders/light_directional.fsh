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


uniform sampler2D gbuffer_frame;
uniform sampler2D gbuffer_depth;

uniform highp vec3 lightDirection; // Must be normalized and converted to view space before entering shader
uniform highp mat4 model_to_view_matrix;
uniform mediump vec4 viewport;

void main()
{
    
    
    
   
    mediump vec2 gbuffer_uv = vec2(gl_FragCoord.xy / viewport.zw);
    
    highp vec3 view_space_vertex_position;
    view_space_vertex_position.xy = ((2.0 * gl_FragCoord.xy) - (2.0 * viewport.xy)) / (viewport.zw) - 1.0;
    /*view_space_vertex_position.z = -(2.0 *  texture2D(gbuffer_depth, gbuffer_uv).r - gl_DepthRange.near - gl_DepthRange.far) /
    (gl_DepthRange.far - gl_DepthRange.near) - 1.0;
     */
    view_space_vertex_position.z = -texture2D(gbuffer_depth, gbuffer_uv).r;
    
    
    lowp vec4 gbuffer_sample = texture2D(gbuffer_frame, gbuffer_uv);
    
    mediump vec3 gbuffer_normal = 2.0 * gbuffer_sample.rgb - 1.0;
    mediump float gbuffer_specular_exponent = gbuffer_sample.a * 100.0;
    
    mediump vec3 view_space_light = lightDirection; // vec3(model_to_view_matrix * vec4(lightDirection, 1.0));
    
    mediump float lamberFactor = max(0.0,dot(view_space_light, gbuffer_normal));
    
    
    
    
    highp vec3 halfVec = normalize((normalize(vec3(0.0, 0.0, 0.0) - view_space_vertex_position) + view_space_light)); // Normalizing anyways, no need to divide by 2

    //highp vec3 halfVec = normalize(view_space_light - normalize(view_space_vertex_position));
    //highp vec3 halfVec = normalize(vec3(0.0, 0.0, -1.0) - normalize(view_space_vertex_position));
    
    mediump float specularFactor = max(0.0,pow(clamp(dot(halfVec,normalize(gbuffer_normal)), 0.0, 1.0), gbuffer_specular_exponent));
    //mediump float specularFactor = 0.0;
    
    //specularFactor = halfVec.z;
    
    gl_FragColor = vec4(vec3(lamberFactor), specularFactor);
}
