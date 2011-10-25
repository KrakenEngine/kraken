//
//  Shader.vsh
//  gldemo
//
//  Created by Kearwood Gilbert on 10-09-16.
//  Copyright (c) 2010 Kearwood Software. All rights reserved.
//
/*
 
 // -- Per vertex lighting
 
 attribute highp vec3	myVertex, myNormal;
 attribute mediump vec2	myUV;
 uniform mediump mat4	myMVPMatrix, myModelView; // mvpmatrix is the result of multiplying the model, view, and projection matrices 
 uniform mediump mat3	myModelViewIT;
 uniform mediump vec3    material_ambient, material_diffuse, material_specular;
 
 varying mediump vec2	texCoord;
 varying mediump vec3	diffuse, specular;
 varying mediump vec3      normal;
 const mediump float		shininess = 4.0;
 const mediump float		cutoff = 0.975, exp = 100.0;
 const mediump vec3		LightPos = vec3(20.0, 00.0, 10.0);
 const mediump vec3		LightCol = vec3(2.0,2.0,2.0);
 
 //directional light function //
 // spFlg	flag using specular or not.
 // nrml		nrml vector in the eye coordinate.
 // ePos		vertex position in the eye coordinate.
 void DirectionalLight(in mediump int spFlg, in mediump vec3 nrml, in mediump vec3 ePos){
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
 
 // main function //
 void main(void){
 // transform the normal vector from the model coordinate to the eye coordinate.
 normal = normalize(myModelViewIT * myNormal);
 // calculate the vertex position in the eye coordinate.
 highp vec3 ePos = vec3(myModelView * vec4(myVertex,1.0));
 // initalize light intensity parameter.
 //diffuse = material_ambient;
 diffuse = vec3(0.75);
 
 specular = vec3(0.0);
 
 DirectionalLight(1, normal, ePos);
 
 specular *= material_specular;
 
 diffuse = diffuse * material_diffuse + material_ambient;
 
 
 // Transform position
 gl_Position = myMVPMatrix * vec4(myVertex,1.0);
 // Pass UV co-ordinates
 texCoord = myUV.st;
 }
 
*/

/*
 
 // -- Per Pixel lighting, test 1 --

attribute highp vec3	myVertex, myNormal, myTangent;
attribute mediump vec2	myUV;
uniform mediump mat4	myMVPMatrix, myModelView; // mvpmatrix is the result of multiplying the model, view, and projection matrices 
uniform mediump mat3	myModelViewIT;
uniform mediump vec3    material_ambient, material_diffuse, material_specular;

varying mediump vec2	texCoord;
varying mediump vec3    normal;
varying mediump vec3    ePos;


// main function //
void main(void){
    // Transform position
    gl_Position = myMVPMatrix * vec4(myVertex,1.0);
    // Pass UV co-ordinates
    texCoord = myUV.st;
    
    // transform the normal vector from the model coordinate to the eye coordinate.
    normal = normalize(myModelViewIT * myNormal);
    
    // calculate the vertex position in the eye coordinate.
    ePos = vec3(myModelView * vec4(myVertex,1.0));
    
    //mat_ambient = material_ambient;
    //mat_diffuse = material_diffuse;
    //mat_specular = material_specular;
}

*/


// -- Per Pixel lighting, test 2 --

const mediump vec3		LightPos = vec3(40, 20.0, -90.0);

attribute highp vec3	myVertex, myNormal;
attribute highp vec3    myTangent;
attribute mediump vec2	myUV;
uniform highp mat4      myMVPMatrix, myModelView; // mvpmatrix is the result of multiplying the model, view, and projection matrices 
uniform highp mat3      myModelViewIT;
uniform lowp vec3       material_ambient, material_diffuse, material_specular;

varying mediump vec2	texCoord;
/*
varying mediump vec3    lightVec;
varying mediump vec3    halfVec;
*/


//varying mediump vec3    eyeVec;

void main()
{
    // Transform position
    gl_Position = myMVPMatrix * vec4(myVertex,1.0);
    
    // Pass UV co-ordinates
    texCoord = myUV.st;
    
    /*
    
    // Building the matrix Eye Space -> Tangent Space
    vec3 n = normalize(vec3(myModelView * vec4(myNormal, 1.0)));
	vec3 t = normalize(vec3(myModelView * vec4(myTangent, 1.0)));
	vec3 b = cross(n, t);
    
    vec3 vertexPosition = vec3(myMVPMatrix * vec4(myVertex, 1.0));
	vec3 lightDir = normalize(LightPos - vertexPosition);
    
    // transform light and half angle vectors by tangent basis
	vec3 v;
	v.x = dot(lightDir, t);
	v.y = dot(lightDir, b);
	v.z = dot(lightDir, n);
	lightVec = normalize(v);
    
//  v.x = dot(vertexPosition, t);
//	v.y = dot(vertexPosition, b);
//	v.z = dot(vertexPosition, n);
//	eyeVec = normalize(v);
    
    vertexPosition = normalize(vertexPosition);
    
    // Normalize the halfVector to pass it to the fragment shader
	vec3 halfVector = normalize((vertexPosition + lightDir) / 2.0);
	v.x = dot (halfVector, t);
	v.y = dot (halfVector, b);
	v.z = dot (halfVector, n);
	halfVec = normalize (v);
    */
}
