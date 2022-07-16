//
//  dust_particle_osx.vsh
//  Kraken Engine
//
//  Copyright 2022 Kearwood Gilbert. All rights reserved.
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


in mediump vec2	vertex_uv;
uniform highp mat4      mvp_matrix; // mvp_matrix is the result of multiplying the model, view, and projection matrices
uniform mediump vec4    viewport;
uniform mediump float   flare_size;
in vec4          vertex_position;
uniform highp vec3      particle_origin;

uniform highp mat4      shadow_mvp1;
out mediump vec4	shadowMapCoord1;

out mediump vec2    texCoord;
uniform highp float     time_absolute;
out lowp float      intensity_modulate;

void main() {
    highp vec4 offset_center = vertex_position + vec4(particle_origin, 0.0);
    offset_center.xyz += vec3(sin((time_absolute + vertex_position.x * 100.0) * 0.05), sin((time_absolute + vertex_position.y * 100.0) * 0.07), sin((time_absolute + vertex_position.z * 100.0) * 0.03)) * 0.05;
    offset_center = vec4(mod(offset_center.x + 1.0, 2.0) - 1.0, mod(offset_center.y + 1.0, 2.0) - 1.0, mod(offset_center.z + 1.0, 2.0) - 1.0, 1.0);
    highp vec4 particle_center = mvp_matrix * offset_center;
    texCoord = vertex_uv * 3.46410161513775; // 3.46410161513775 = 2 * sqrt(3); 1 / (2 * sqrt(3)) is the radius of a circle encompased by a equilateral triangle with a side length of 1.
    gl_Position = particle_center + vec4(vertex_uv.x * viewport.w / viewport.z * 2.0 - 1.0, vertex_uv.y * 2.0 - 1.0, 0.0, 0.0) * flare_size;
    
    shadowMapCoord1 = shadow_mvp1 * offset_center;
    
    intensity_modulate = sin(time_absolute + mod(vertex_position.x * 100.0, 6.28318530717959)) * 0.5 + 0.5;
}
