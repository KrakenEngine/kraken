uniform lowp vec3       material_ambient, material_diffuse, material_specular;
uniform lowp float   material_alpha;

#if ENABLE_PER_PIXEL == 1
uniform mediump float   material_shininess;
#endif

#if HAS_DIFFUSE_MAP == 1
uniform sampler2D 		diffuseTexture;
#endif

#if HAS_SPEC_MAP == 1
uniform sampler2D 		specularTexture;
#endif

#if HAS_NORMAL_MAP == 1 && ENABLE_PER_PIXEL == 1
uniform sampler2D 		normalTexture;
#endif

#if SHADOW_QUALITY >= 1
uniform sampler2D   shadowTexture1;
varying highp vec4  shadowMapCoord1;
#endif

#if SHADOW_QUALITY >= 2
uniform sampler2D   shadowTexture2;
varying highp vec4	shadowMapCoord2;
#endif

#if SHADOW_QUALITY >= 3
uniform sampler2D   shadowTexture3;
varying highp vec4  shadowMapCoord3;
#endif

#if HAS_DIFFUSE_MAP == 1 || (HAS_NORMAL_MAP == 1 && ENABLE_PER_PIXEL == 1) || HAS_SPEC_MAP == 1
varying mediump vec2    texCoord;
#endif

#if HAS_NORMAL_MAP == 0 && ENABLE_PER_PIXEL == 1
varying mediump vec3    normal;
#endif

#if ENABLE_PER_PIXEL == 1
varying mediump vec3    lightVec;
varying mediump vec3    halfVec;
#else
varying mediump float   lamberFactor;
varying mediump float   specularFactor;
#endif

void main()
{
#if HAS_NORMAL_MAP == 1 && ENABLE_PER_PIXEL == 1
    // lookup normal from normal map, move from [0,1] to  [-1, 1] range, normalize
	mediump vec3 normal = normalize(2.0 * texture2D(normalTexture,texCoord).rgb - 1.0);
#endif
    
#if ENABLE_PER_PIXEL == 1
    mediump float lamberFactor = max(0.0,dot(lightVec, normal));
    mediump float specularFactor = 0.0;
    if(material_shininess > 0.0) {
        specularFactor = max(0.0,pow(dot(halfVec,normal), material_shininess));
    }
#endif

#if HAS_DIFFUSE_MAP == 1
    mediump vec4 diffuseMaterial = vec4(vec3(texture2D(diffuseTexture, texCoord)), material_alpha);
#else
    mediump vec4 diffuseMaterial = vec4(vec3(1.0), material_alpha);
#endif

#if SHADOW_QUALITY == 1
    highp float shadowMapDepth = 1.0;
    highp float vertexShadowDepth = 1.0;
    highp vec2 shadowMapPos = ((shadowMapCoord1 / shadowMapCoord1.w + 1.0) / 2.0).st;
    
    if(shadowMapCoord1.x >= -1.0 && shadowMapCoord1.x <= 1.0 && shadowMapCoord1.y >= -1.0 && shadowMapCoord1.y <= 1.0 && shadowMapCoord1.z >= 0.0 && shadowMapCoord1.z <= 1.0) {
#if DEBUG_PSSM == 1
        diffuseMaterial = diffuseMaterial * vec4(0.75, 0.75, 0.5, 1.0) + vec4(0.0, 0.0, 0.5, 0.0);
#endif
        shadowMapDepth =  texture2D(shadowTexture1, shadowMapPos).z; // unpack(texture2D(shadowTexture1, shadowMapPos));
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
    }
    
    else if(shadowMapCoord2.s >= -1.0 && shadowMapCoord2.s <= 1.0 && shadowMapCoord2.t >= -1.0 && shadowMapCoord2.t <= 1.0 && shadowMapCoord2.z >= 0.0 && shadowMapCoord2.z <= 1.0) {
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
#if HAS_SPEC_MAP == 1
    gl_FragColor += vec4(material_specular * vec3(texture2D(specularTexture, texCoord)) * specularFactor, 0.0);
#else
    gl_FragColor += vec4(material_specular * specularFactor, 0.0);
#endif
    
#endif
    
}
