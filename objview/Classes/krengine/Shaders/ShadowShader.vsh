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