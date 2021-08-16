//
//  post_fx.frag
//  Kraken Engine
//
//  Copyright 2021 Kearwood Gilbert. All rights reserved.
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


#define ENABLE_VIDEO_BG 0
#define PIXEL_SHIFT_1 0.001
#define PIXEL_SHIFT_2 0.002
#define PIXEL_SHIFT_3 0.003
#define PIXEL_SHIFT_4 0.004

in vec2 textureCoordinate;

#if ENABLE_VIDEO_BG == 1
uniform sampler2D videoFrame;
#endif

#if ENABLE_FADE_COLOR == 1
uniform vec4 fade_color;
#endif

uniform sampler2D renderFrame;
uniform sampler2D depthFrame;

#if VOLUMETRIC_ENVIRONMENT_DOWNSAMPLED == 1
uniform sampler2D volumetricEnvironmentFrame;
#endif

out vec4 colorOut;

void main()
{
    
    vec4 renderColor = texture(renderFrame, textureCoordinate);
#if DOF_QUALITY > 0 || ENABLE_FLASH == 1
    float depth = texture(depthFrame, textureCoordinate).r;
#endif
    vec4 pixelColor = renderColor;
    
    
#if DOF_QUALITY == 2
    
    // Render high quality circle of confusion
    // __XXX__
    // _XXXXX_
    // _XXXXX_
    // _XXXXX_
    // __XXX__
    float cf1 = PIXEL_SHIFT_1;
    float cf2 = PIXEL_SHIFT_2;
    
    float bx1 = textureCoordinate.s + cf1;
    float bx2 = textureCoordinate.s + cf2;
    float bxm1 = textureCoordinate.s - cf1;
    float bxm2 = textureCoordinate.s - cf2;
    
    float by1 = textureCoordinate.t + cf1;
    float by2 = textureCoordinate.t + cf2;
    float bym1 = textureCoordinate.t - cf1;
    float bym2 = textureCoordinate.t - cf2;
    
    pixelColor += texture(renderFrame, vec2(bx1, textureCoordinate.t));
    pixelColor += texture(renderFrame, vec2(bxm1, textureCoordinate.t));
    pixelColor += texture(renderFrame, vec2(bx2, textureCoordinate.t));
    pixelColor += texture(renderFrame, vec2(bxm2, textureCoordinate.t));
    
    pixelColor += texture(renderFrame, vec2(textureCoordinate.s, by1));
    pixelColor += texture(renderFrame, vec2(bx1, by1));
    pixelColor += texture(renderFrame, vec2(bxm1, by1));
    pixelColor += texture(renderFrame, vec2(bx2, by1));
    pixelColor += texture(renderFrame, vec2(bxm2, by1));
    
    pixelColor += texture(renderFrame, vec2(textureCoordinate.s, by2));
    pixelColor += texture(renderFrame, vec2(bx1, by2));
    pixelColor += texture(renderFrame, vec2(bxm1, by2));
    
    pixelColor += texture(renderFrame, vec2(textureCoordinate.s,bym1));
    pixelColor += texture(renderFrame, vec2(bx1,bym1));
    pixelColor += texture(renderFrame, vec2(bxm1,bym1));
    pixelColor += texture(renderFrame, vec2(bx2,bym1));
    pixelColor += texture(renderFrame, vec2(bxm2,bym1));
    
    pixelColor += texture(renderFrame, vec2(bx1, bym2));
    pixelColor += texture(renderFrame, vec2(bx1, bym2));  
    pixelColor += texture(renderFrame, vec2(bxm1, bym2));
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
    pixelColor += texture(renderFrame, textureCoordinate + vec2(0, -PIXEL_SHIFT_2));
    pixelColor += texture(renderFrame, textureCoordinate + vec2(-PIXEL_SHIFT_1, -PIXEL_SHIFT_1));
    pixelColor += texture(renderFrame, textureCoordinate + vec2(0, -PIXEL_SHIFT_1));
    pixelColor += texture(renderFrame, textureCoordinate + vec2(+PIXEL_SHIFT_1, -PIXEL_SHIFT_1));
    pixelColor += texture(renderFrame, textureCoordinate + vec2(-PIXEL_SHIFT_2, 0));
    pixelColor += texture(renderFrame, textureCoordinate + vec2(-PIXEL_SHIFT_1, 0));
    pixelColor += texture(renderFrame, textureCoordinate + vec2(+PIXEL_SHIFT_1, 0));
    pixelColor += texture(renderFrame, textureCoordinate + vec2(+PIXEL_SHIFT_2, 0));
    pixelColor += texture(renderFrame, textureCoordinate + vec2(-PIXEL_SHIFT_1, -PIXEL_SHIFT_1));
    pixelColor += texture(renderFrame, textureCoordinate + vec2(0, -PIXEL_SHIFT_1));
    pixelColor += texture(renderFrame, textureCoordinate + vec2(+PIXEL_SHIFT_1, -PIXEL_SHIFT_1));
    pixelColor += texture(renderFrame, textureCoordinate + vec2(0, -PIXEL_SHIFT_2));
    pixelColor /= 13.0;
     
#endif
// DOF_QUALITY == 1
    
#if DOF_QUALITY > 0
    float focusDepth = texture(depthFrame, vec2(0.5, 0.5)).r;
    float blurAmount = clamp((depth - DOF_DEPTH - focusDepth) / DOF_FALLOFF, 0.0, 1.0);
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
    
    
    // ---- VIDEO_BG END ----

#if VOLUMETRIC_ENVIRONMENT_DOWNSAMPLED == 1
    pixelColor += texture(volumetricEnvironmentFrame, textureCoordinate);
#endif
    

    // ---- VIGNETTE START ----
    
    // Render vignette effect
    
#if ENABLE_VIGNETTE == 1
    pixelColor *= vec4(vec3(clamp(1.0 - (distance(textureCoordinate, vec2(0.5, 0.5)) - VIGNETTE_RADIUS) / VIGNETTE_FALLOFF, 0.0, 1.0)), 1.0);
#endif
    // ---- VIGNETTE END ----
    
#if ENABLE_FADE_COLOR == 1
    pixelColor.rgb = mix(pixelColor.rgb, fade_color.rgb, fade_color.a);
#endif
    
    colorOut = pixelColor;
    
     //PASSTHROUGH STATEMENT
    // colorOut = texture(depthFrame, textureCoordinate);
    
    //colorOut = vec4(vec3(blurAmount), 1.0);
    
    colorOut = vec4(0.0, 0.0, 1.0, 1.0); // FINDME, KIP!!, HACK!!
}

