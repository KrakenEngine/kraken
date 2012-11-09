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

attribute highp vec4	vertex_position;
uniform highp mat4      inv_mvp_matrix;

uniform highp vec2      slice_depth_scale; // First component is the depth for the nearest plane, in view space.  Second component is the distance between planes, in view space

uniform highp mat4  shadow_mvp1;
varying highp vec4	shadowMapCoord1;

uniform highp mat4  projection_matrix;

void main()
{    
    highp vec4 d = projection_matrix * vec4(0.0, 0.0, slice_depth_scale.x + vertex_position.z * slice_depth_scale.y, 1.0);
    d /= d.w;
    gl_Position = vec4(vertex_position.x, vertex_position.y, d.z, 1.0);

    
    shadowMapCoord1 = inv_mvp_matrix * gl_Position;
    shadowMapCoord1 /= shadowMapCoord1.w;
    shadowMapCoord1.w = 1.0;
    shadowMapCoord1 = shadow_mvp1 * shadowMapCoord1;
    
}
