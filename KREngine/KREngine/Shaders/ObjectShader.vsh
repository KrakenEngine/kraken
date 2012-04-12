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

attribute highp vec3	myVertex, myNormal;
attribute highp vec3    myTangent;
attribute mediump vec2	myUV;
attribute mediump vec2   shadowuv;
uniform highp mat4      myMVPMatrix, myShadowMVPMatrix1,myShadowMVPMatrix2,myShadowMVPMatrix3; // mvpmatrix is the result of multiplying the model, view, and projection matrices 
// uniform lowp vec3       material_ambient, material_diffuse, material_specular;
uniform highp vec3      lightDirection; // Must be normalized before entering shader
uniform highp vec3      cameraPosition;

#if ENABLE_PER_PIXEL == 0
uniform mediump float   material_shininess;
#endif

#if HAS_DIFFUSE_MAP == 1 || (HAS_NORMAL_MAP == 1 && ENABLE_PER_PIXEL == 1) || HAS_SPEC_MAP == 1
varying highp vec2	texCoord;
#endif

#if HAS_LIGHT_MAP == 1
varying mediump vec2  shadowCoord;
#endif


#if ENABLE_PER_PIXEL == 1
varying mediump vec3    lightVec;
varying mediump vec3    halfVec;
#else
varying mediump float   lamberFactor;
varying mediump float   specularFactor;
#endif

#if HAS_DIFFUSE_MAP_SCALE == 1
uniform highp vec2    diffuseTexture_Scale;
#endif

#if HAS_NORMAL_MAP_SCALE == 1
uniform highp vec2    normalTexture_Scale;
#endif

#if HAS_SPEC_MAP_SCALE == 1
uniform highp vec2    specularTexture_Scale;
#endif

#if HAS_NORMAL_MAP_OFFSET == 1
uniform highp vec2    normalTexture_Offset;
#endif

#if HAS_SPEC_MAP_OFFSET == 1
uniform highp vec2    specularTexture_Offset;
#endif

#if HAS_DIFFUSE_MAP_OFFSET == 1
uniform highp vec2    diffuseTexture_Offset;
#endif

#if HAS_NORMAL_MAP == 0 && ENABLE_PER_PIXEL == 1
varying mediump vec3      normal;
#endif

#if SHADOW_QUALITY >= 1
varying highp vec4	shadowMapCoord1;
#endif

#if SHADOW_QUALITY >= 2
varying highp vec4	shadowMapCoord2;
#endif

#if SHADOW_QUALITY >= 3
varying highp vec4	shadowMapCoord3;
#endif

#if (HAS_NORMAL_MAP_OFFSET == 1 || HAS_NORMAL_MAP_SCALE == 1) && ENABLE_PER_PIXEL == 1
varying highp vec2  normal_uv;
#endif

#if (HAS_SPEC_MAP_OFFSET == 1|| HAS_SPEC_MAP_SCALE == 1) && ENABLE_PER_PIXEL == 1
varying highp vec2 spec_uv;
#endif

#if HAS_DIFFUSE_MAP_OFFSET == 1 || HAS_DIFFUSE_MAP_SCALE == 1
varying highp vec2  diffuse_uv;
#endif

void main()
{
    // Transform position
    gl_Position = myMVPMatrix * vec4(myVertex,1.0);
    

    
#if HAS_DIFFUSE_MAP == 1 || (HAS_NORMAL_MAP == 1 && ENABLE_PER_PIXEL == 1) || HAS_SPEC_MAP == 1
    // Pass UV co-ordinates
    texCoord = myUV.st;
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


// Scaled and translated specular map UV's
#if (HAS_SPEC_MAP_OFFSET == 1 || HAS_SPEC_MAP_SCALE == 1) && ENABLE_PER_PIXEL == 1
    spec_uv = texCoord;
#if HAS_SPEC_MAP_OFFSET == 1
    spec_uv + specularTexture_Offset;
#endif
    
#if HAS_SPEC_MAP_SCALE == 1
    spec_uv *= specularTexture_Scale;
#endif
    
#endif    
    
    
    
#if HAS_LIGHT_MAP == 1
    // Pass shadow UV co-ordinates
    shadowCoord = shadowuv.st;
#endif
    
#if SHADOW_QUALITY >= 1
    shadowMapCoord1 = myShadowMVPMatrix1 * vec4(myVertex,1.0);
#endif
    
#if SHADOW_QUALITY >= 2
    shadowMapCoord2 = myShadowMVPMatrix2 * vec4(myVertex,1.0);
#endif
    
#if SHADOW_QUALITY >= 3
    shadowMapCoord3 = myShadowMVPMatrix3 * vec4(myVertex,1.0);
#endif
    

     // ----------- Directional Light (Sun) -----------
     
#if HAS_NORMAL_MAP == 1 && ENABLE_PER_PIXEL == 1
    // ----- Calculate per-pixel lighting in tangent space, for normal mapping ------
    mediump vec3 eyeVec = normalize(cameraPosition - myVertex);
    mediump vec3 a_bitangent = cross(myNormal, myTangent);
    lightVec = normalize(vec3(dot(lightDirection, myTangent), dot(lightDirection, a_bitangent), dot(lightDirection, myNormal)));
    halfVec = normalize(vec3(dot(eyeVec, myTangent), dot(eyeVec, a_bitangent), dot(eyeVec, myNormal)));
    halfVec = normalize(halfVec + lightVec); // Normalizing anyways, no need to divide by 2

#else

#if ENABLE_PER_PIXEL == 1
    // ------ Calculate per-pixel lighting without normal mapping ------
    normal = myNormal;
    lightVec = lightDirection;
    halfVec = normalize((normalize(cameraPosition - myVertex) + lightVec)); // Normalizing anyways, no need to divide by 2
#else
    // ------ Calculate per-vertex lighting ------
    mediump vec3 halfVec = normalize((normalize(cameraPosition - myVertex) + lightDirection)); // Normalizing anyways, no need to divide by 2
    lamberFactor = max(0.0,dot(lightDirection, myNormal));
    specularFactor = max(0.0,pow(dot(halfVec,myNormal), material_shininess));
#endif
#endif
}
