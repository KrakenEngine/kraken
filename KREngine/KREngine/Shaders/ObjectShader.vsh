//
//  Shader.vsh
//  gldemo
//
//  Created by Kearwood Gilbert on 10-09-16.
//  Copyright (c) 2010 Kearwood Software. All rights reserved.
//

attribute highp vec3	myVertex, myNormal;
attribute highp vec3    myTangent;
attribute mediump vec2	myUV;
uniform highp mat4      myMVPMatrix, myShadowMVPMatrix1,myShadowMVPMatrix2,myShadowMVPMatrix3; // mvpmatrix is the result of multiplying the model, view, and projection matrices 
// uniform lowp vec3       material_ambient, material_diffuse, material_specular;
uniform highp vec3      lightDirection; // Must be normalized before entering shader
uniform highp vec3      cameraPosition;

#if ENABLE_PER_PIXEL == 0
uniform mediump float   material_shininess;
#endif

#if HAS_DIFFUSE_MAP == 1 || (HAS_NORMAL_MAP == 1 && ENABLE_PER_PIXEL == 1) || HAS_SPEC_MAP == 1
varying mediump vec2	texCoord;
#endif

#if ENABLE_PER_PIXEL == 1
varying mediump vec3    lightVec;
varying mediump vec3    halfVec;
#else
varying mediump float   lamberFactor;
varying mediump float   specularFactor;
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

void main()
{
    // Transform position
    gl_Position = myMVPMatrix * vec4(myVertex,1.0);
    

    
#if HAS_DIFFUSE_MAP == 1 || (HAS_NORMAL_MAP == 1 && ENABLE_PER_PIXEL == 1) || HAS_SPEC_MAP == 1
    // Pass UV co-ordinates
    texCoord = myUV.st;
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
    specularFactor = 0.0;
    if(material_shininess > 0.0) {
        specularFactor = max(0.0,pow(dot(halfVec,myNormal), material_shininess));
    }
#endif
#endif
}
