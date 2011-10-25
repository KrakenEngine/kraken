#define SHADOW_BIAS 0.01


attribute highp vec3	myVertex;
/*
attribute mediump vec2	myUV;
varying mediump vec2	texCoord;
*/
uniform highp mat4      myShadowMVPMatrix1; // Shadowmvpmatrix is the result of multiplying the model, view, and projection matrices 



void main()
{
    // Transform position
    /*
    position = myShadowMVPMatrix1 * vec4(myVertex,1.0);
     */
    gl_Position = myShadowMVPMatrix1 * vec4(myVertex,1.0);
    gl_Position.z += SHADOW_BIAS;
    /*
    // Pass UV co-ordinates
    texCoord = myUV.st;
    */
   
}
