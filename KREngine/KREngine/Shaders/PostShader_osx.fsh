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

#version 120
#define lowp
#define mediump
#define highp

#define ENABLE_VIDEO_BG 0
#define PIXEL_SHIFT_1 0.001
#define PIXEL_SHIFT_2 0.002
#define PIXEL_SHIFT_3 0.003
#define PIXEL_SHIFT_4 0.004

varying mediump vec2 textureCoordinate;
precision lowp float;

#if ENABLE_VIDEO_BG == 0
uniform lowp sampler2D videoFrame;
#endif

uniform lowp sampler2D renderFrame;
uniform lowp sampler2D depthFrame;

#if VOLUMETRIC_ENVIRONMENT_DOWNSAMPLED == 1
uniform lowp sampler2D volumetricEnvironmentFrame;
#endif

void main()
{
    
    lowp vec4 renderColor = texture2D(renderFrame, textureCoordinate);
#if DOF_QUALITY > 0 || ENABLE_FLASH == 1
    mediump float depth = texture2D(depthFrame, textureCoordinate).r;
#endif
    mediump vec4 pixelColor = renderColor;
    
    
#if DOF_QUALITY == 2
    
    // Render high quality circle of confusion
    // __XXX__
    // _XXXXX_
    // _XXXXX_
    // _XXXXX_
    // __XXX__
    mediump float cf1 = PIXEL_SHIFT_1;
    mediump float cf2 = PIXEL_SHIFT_2;
    
    mediump float bx1 = textureCoordinate.s + cf1;
    mediump float bx2 = textureCoordinate.s + cf2;
    mediump float bxm1 = textureCoordinate.s - cf1;
    mediump float bxm2 = textureCoordinate.s - cf2;
    
    mediump float by1 = textureCoordinate.t + cf1;
    mediump float by2 = textureCoordinate.t + cf2;
    mediump float bym1 = textureCoordinate.t - cf1;
    mediump float bym2 = textureCoordinate.t - cf2;
    
    pixelColor += texture2D(renderFrame, vec2(bx1, textureCoordinate.t));
    pixelColor += texture2D(renderFrame, vec2(bxm1, textureCoordinate.t));
    pixelColor += texture2D(renderFrame, vec2(bx2, textureCoordinate.t));
    pixelColor += texture2D(renderFrame, vec2(bxm2, textureCoordinate.t));
    
    pixelColor += texture2D(renderFrame, vec2(textureCoordinate.s, by1));
    pixelColor += texture2D(renderFrame, vec2(bx1, by1));
    pixelColor += texture2D(renderFrame, vec2(bxm1, by1));
    pixelColor += texture2D(renderFrame, vec2(bx2, by1));
    pixelColor += texture2D(renderFrame, vec2(bxm2, by1));
    
    pixelColor += texture2D(renderFrame, vec2(textureCoordinate.s, by2));
    pixelColor += texture2D(renderFrame, vec2(bx1, by2));
    pixelColor += texture2D(renderFrame, vec2(bxm1, by2));
    
    pixelColor += texture2D(renderFrame, vec2(textureCoordinate.s,bym1));
    pixelColor += texture2D(renderFrame, vec2(bx1,bym1));
    pixelColor += texture2D(renderFrame, vec2(bxm1,bym1));
    pixelColor += texture2D(renderFrame, vec2(bx2,bym1));
    pixelColor += texture2D(renderFrame, vec2(bxm2,bym1));
    
    pixelColor += texture2D(renderFrame, vec2(bx1, bym2));
    pixelColor += texture2D(renderFrame, vec2(bx1, bym2));  
    pixelColor += texture2D(renderFrame, vec2(bxm1, bym2));
    pixelColor /= 21.0;

#endif
// DOF_QUALITY == 2

#if DOF_QUALITY == 1
    
    // Render low quality circle of confusion
    // ___X___
    // __XXX__
    // _XXXXX_
    // __XXX__
    // ___X___
    pixelColor += texture2D(renderFrame, textureCoordinate + vec2(0, -PIXEL_SHIFT_2));
    pixelColor += texture2D(renderFrame, textureCoordinate + vec2(-PIXEL_SHIFT_1, -PIXEL_SHIFT_1));
    pixelColor += texture2D(renderFrame, textureCoordinate + vec2(0, -PIXEL_SHIFT_1));
    pixelColor += texture2D(renderFrame, textureCoordinate + vec2(+PIXEL_SHIFT_1, -PIXEL_SHIFT_1));
    pixelColor += texture2D(renderFrame, textureCoordinate + vec2(-PIXEL_SHIFT_2, 0));
    pixelColor += texture2D(renderFrame, textureCoordinate + vec2(-PIXEL_SHIFT_1, 0));
    pixelColor += texture2D(renderFrame, textureCoordinate + vec2(+PIXEL_SHIFT_1, 0));
    pixelColor += texture2D(renderFrame, textureCoordinate + vec2(+PIXEL_SHIFT_2, 0));
    pixelColor += texture2D(renderFrame, textureCoordinate + vec2(-PIXEL_SHIFT_1, -PIXEL_SHIFT_1));
    pixelColor += texture2D(renderFrame, textureCoordinate + vec2(0, -PIXEL_SHIFT_1));
    pixelColor += texture2D(renderFrame, textureCoordinate + vec2(+PIXEL_SHIFT_1, -PIXEL_SHIFT_1));
    pixelColor += texture2D(renderFrame, textureCoordinate + vec2(0, -PIXEL_SHIFT_2));
    pixelColor /= 13.0;
     
#endif
// DOF_QUALITY == 1
    
#if DOF_QUALITY > 0
    mediump float focusDepth = texture2D(depthFrame, vec2(0.5, 0.5)).r;
    mediump float blurAmount = clamp((depth - DOF_DEPTH - focusDepth) / DOF_FALLOFF, 0.0, 1.0);
    pixelColor = pixelColor * blurAmount + renderColor * (1.0 - blurAmount);
#endif
    
    // ---- DEPTH_OF_FIELD END ----
    
    
    
    
    // ---- NIGHT_TIME START ----
#if ENABLE_FLASH == 1
    // Un-comment to enable night time / flash effect
    pixelColor *= vec4(vec3(1.0 - clamp((depth - FLASH_DEPTH) / FLASH_FALLOFF, 0.0, 1.0)) * FLASH_INTENSITY, 1.0);
    //pixelColor *= vec4(vec3(clamp(1.0 / (depth - FLASH_DEPTH) * FLASH_FALLOFF, 0.0, 1.0) * FLASH_INTENSITY), 1.0);
    
#endif
    // ---- NIGHT_TIME END ----
    
    // ---- VIDEO_BG START ----
    
    /*
    // Overlay video background
    if(depth == 1.0) {
        //FILTER COLOR BY CALCULATING PER PIXEL DOT PRODUCT
        pixelColor = vec4(dot(vec3(texture2D(videoFrame, textureCoordinate)), vec3(.222, .707, .071)));
    }
     */
    
    // ---- VIDEO_BG END ----

#if VOLUMETRIC_ENVIRONMENT_DOWNSAMPLED == 1
    pixelColor += texture2D(volumetricEnvironmentFrame, textureCoordinate);
#endif
    

    // ---- VIGNETTE START ----
    
    // Render vignette effect
    
#if ENABLE_VIGNETTE
    pixelColor *= vec4(vec3(clamp(1.0 - (distance(textureCoordinate, vec2(0.5, 0.5)) - VIGNETTE_RADIUS) / VIGNETTE_FALLOFF, 0.0, 1.0)), 1.0);
#endif
    // ---- VIGNETTE END ----
    
    gl_FragColor = pixelColor;
    
    
     //PASSTHROUGH STATEMENT
    // gl_FragColor = texture2D(depthFrame, textureCoordinate);
    
    //gl_FragColor = vec4(vec3(blurAmount), 1.0);
     
}
