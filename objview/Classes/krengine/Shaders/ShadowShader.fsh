// -- Per Pixel lighting, test 3 --
uniform sampler2D 		diffuseTexture, specularTexture, normalTexture;
varying mediump vec2	texCoord;
varying mediump vec3    lightVec;
varying mediump vec3    halfVec;
uniform lowp vec3       material_ambient, material_diffuse, material_specular;

void main()
{
    // compute diffuse lighting
    mediump vec4 diffuseMaterial = texture2D(diffuseTexture, texCoord);
    
    /*
     // lookup normal from normal map, move from [0,1] to  [-1, 1] range, normalize
     mediump vec3 normal = normalize(2.0 * texture2D(normalTexture,texCoord).rgb - 1.0);
     
     mediump float lamberFactor= max (dot (lightVec, normal), 0.0);
     
     // Add ambient light and alpha component from diffuse texture map
     gl_FragColor =	vec4(material_ambient, diffuseMaterial.w);
     
     // compute specular lighting
     mediump vec3 specularLight = vec3(texture2D(specularTexture, texCoord)); // Specular value comes from a texture 
     mediump float shininess = pow (max (dot (halfVec, normal), 0.0), 2.0);
     
     // Add diffuse light
     gl_FragColor += vec4(vec3(diffuseMaterial) * material_diffuse * lamberFactor, 0.0);
     
     // Add specular light
     gl_FragColor += vec4(material_specular * specularLight * shininess, 0.0);
     */
    
    //gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    gl_FragColor = diffuseMaterial;
}