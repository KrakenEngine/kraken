//
//  Shader.fsh
//  gldemo
//
// Based on http://www.fabiensanglard.net/bumpMapping/index.php


/*
 
// -- Per vertex lighting
 
uniform sampler2D 		diffuseTexture, specularTexture, normalTexture;
varying mediump vec2	texCoord;
varying mediump vec3	diffuse;
varying mediump vec3	specular;
void main (void)
{
    mediump vec3 texColour  = vec3(texture2D(diffuseTexture, texCoord));
    mediump vec3 specColor = vec3(texture2D(specularTexture, texCoord));
    mediump vec3 normalVal = vec3(texture2D(normalTexture, texCoord));
    //mediump vec3 colour = (texColour * diffuse) + specular;
    mediump vec3 colour = (texColour * diffuse) + (specColor * specular);
    gl_FragColor = vec4(colour, 1.0);
}
*/

/*
 
// -- Per Pixel lighting, test 1 --

uniform sampler2D 		diffuseTexture, specularTexture, normalTexture;
uniform mediump vec3    material_ambient, material_diffuse, material_specular;

varying mediump vec2	texCoord;
varying mediump vec3    normal;
varying mediump vec3      ePos;

const mediump float		shininess = 4.0;
const mediump vec3		LightPos = vec3(20.0, 00.0, 10.0);
const mediump vec3		LightCol = vec3(2.0,2.0,2.0);


//directional light function //
// spFlg	flag using specular or not.
// nrml		nrml vector in the eye coordinate.
// ePos		vertex position in the eye coordinate.
void DirectionalLight(inout mediump vec3 diffuse, inout mediump vec3 specular, in mediump int spFlg, in mediump vec3 nrml, in mediump vec3 ePos){
    // calculate the light direction vector.
    mediump vec3 lightDir = normalize(LightPos);
    // calculate the half vector between eye position and light position.
    mediump vec3 halfV = normalize(-ePos + LightPos);
    // calculate diffuse light intensity.
    mediump float dVP = max(dot(nrml,lightDir), 0.0);
    // calculate approximated specular light base intensity.
    mediump float dHV = max(dot(nrml,halfV),0.0);
    // if the diffuse is not zero and spFlg is On,
    // calculate specular light intensity with shininess,
    // or turn off the specular light.
    mediump float pf;
    if (dVP>.0 && spFlg==1) pf = pow(dHV, shininess);
    else     			     pf = 0.0;
    diffuse += dVP*LightCol;
    specular += pf*LightCol;
}

void main (void)
{
    
    mediump vec3 texColour  = vec3(texture2D(diffuseTexture, texCoord));
    mediump vec3 specColor = vec3(texture2D(specularTexture, texCoord));
    mediump vec3 normalVal = vec3(texture2D(normalTexture, texCoord));
    
    
    // initalize light intensity parameter.
    mediump vec3 diffuse = vec3(0.75);
    mediump vec3 specular = vec3(0.0);
    
    
    DirectionalLight(diffuse, specular, 1, normalize(normal + normalVal), ePos);

    specular *= material_specular;
    
    diffuse = diffuse * material_diffuse + material_ambient;
    
    
    //mediump vec3 colour = (texColour * diffuse) + specular;
    mediump vec3 colour = (texColour * diffuse) + (specColor * specular);
    gl_FragColor = vec4(colour, 1.0);

}

*/

/*
// -- Per Pixel lighting, test 2 --
uniform sampler2D 		diffuseTexture, specularTexture, normalTexture;
varying mediump vec2	texCoord;
varying mediump vec3    lightVec;
varying mediump vec3    halfVec;
//varying mediump vec3    eyeVec;
uniform mediump vec3    material_ambient, material_diffuse, material_specular;

void main()
{
    // lookup normal from normal map, move from [0,1] to  [-1, 1] range, normalize
	mediump vec3 normal = normalize(2.0 * texture2D(normalTexture,texCoord).rgb - 1.0);
    
	mediump float lamberFactor= max (dot (lightVec, normal), 0.0);
    
    
    gl_FragColor = vec4(0.0);
    
    if (lamberFactor > 0.0)
	{
        // compute diffuse lighting
        mediump vec4 diffuseMaterial = texture2D(diffuseTexture, texCoord);
        mediump vec4 diffuseLight  = vec4(material_diffuse, 1.0) + 1.0; // 1.0 added so there will not be complete darkness
        
        // compute specular lighting
		mediump vec4 specularLight = texture2D(specularTexture, texCoord); // Specular value comes from a texture 
		mediump float shininess = pow (max (dot (halfVec, normal), 0.0), 2.0);
        
		gl_FragColor = diffuseMaterial * diffuseLight * lamberFactor ;
		gl_FragColor +=	vec4(material_specular, 1.0) * specularLight * shininess;
        
	}
    
	// compute ambient
	gl_FragColor +=	vec4(material_ambient, 1.0);;
}
 */


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
