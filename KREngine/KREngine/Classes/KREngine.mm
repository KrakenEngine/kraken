//
//  KREngine.mm
//  gldemo
//
//  Copyright 2011 Kearwood Gilbert. All rights reserved.
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

#import "KREngine.h"
#import "KRVector3.h"
#import "KRScene.h"

#import <string>
#import <sstream> 

using namespace std;



@interface KREngine (PrivateMethods)
//- (BOOL)loadObjects;
- (BOOL)loadShaders;
- (BOOL)createBuffers;
- (BOOL)compileShader:(GLuint *)shader type:(GLenum)type file:(NSString *)file;
- (BOOL)linkProgram:(GLuint)prog;
- (BOOL)validateProgram:(GLuint)prog;
- (void)renderPost;
- (BOOL)loadResource:(NSString *)path;
@end

@implementation KREngine
double const PI = 3.141592653589793f;

- (id)initForWidth: (GLuint)width Height: (GLuint)height
{
    debug_text = @"";
    sun_yaw = 4.333;
    sun_pitch = 0.55;
    m_iFrame = 0;
    m_cShadowBuffers = 0;
    backingWidth = width;
    backingHeight = height;
    memset(shadowFramebuffer, sizeof(GLuint) * 3, 0);
    memset(shadowDepthTexture, sizeof(GLuint) * 3, 0);

    
    m_postShaderProgram = 0;
    
    

    
    if ((self = [super init])) {
        NSString *vertShaderPathname = [[NSBundle mainBundle] pathForResource:@"ObjectShader" ofType:@"vsh"];
        NSString *fragShaderPathname = [[NSBundle mainBundle] pathForResource:@"ObjectShader" ofType:@"fsh"];
        GLchar * szVertShaderSource = (GLchar *)[[NSString stringWithContentsOfFile:vertShaderPathname encoding:NSUTF8StringEncoding error:nil] UTF8String];
        GLchar * szFragShaderSource = (GLchar *)[[NSString stringWithContentsOfFile:fragShaderPathname encoding:NSUTF8StringEncoding error:nil] UTF8String];
        
        m_pShaderManager = new KRShaderManager(szVertShaderSource, szFragShaderSource);
        m_pTextureManager = new KRTextureManager();
        m_pMaterialManager = new KRMaterialManager(m_pTextureManager, m_pShaderManager);
        m_pModelManager = new KRModelManager(m_pMaterialManager);
        
        if (![self createBuffers] || ![self loadShaders] /*|| ![self loadObjects]*/ )
        {
            [self release];
            return nil;
        }
    }
    
    return self;
}

- (void)allocateShadowBuffers
{
    // First deallocate buffers no longer needed
    for(int iShadow = m_cShadowBuffers; iShadow < KRENGINE_MAX_SHADOW_BUFFERS; iShadow++) {
        if (shadowDepthTexture[iShadow]) {
            glDeleteTextures(1, shadowDepthTexture + iShadow);
            shadowDepthTexture[iShadow] = 0;
        }
        
        if (shadowFramebuffer[iShadow]) {
            glDeleteFramebuffers(1, shadowFramebuffer + iShadow);
            shadowFramebuffer[iShadow] = 0;
        }
    }
    
    // Allocate newly required buffers
    for(int iShadow = 0; iShadow < m_cShadowBuffers; iShadow++) {
        if(!shadowDepthTexture[iShadow]) {
            shadowValid[iShadow] = false;
            
            glGenFramebuffers(1, shadowFramebuffer + iShadow);
            glGenTextures(1, shadowDepthTexture + iShadow);
            // ===== Create offscreen shadow framebuffer object =====
            
            glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer[iShadow]);
            
            // ----- Create Depth Texture for shadowFramebuffer -----
            glBindTexture(GL_TEXTURE_2D, shadowDepthTexture[iShadow]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, KRENGINE_SHADOW_MAP_WIDTH, KRENGINE_SHADOW_MAP_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
            
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepthTexture[iShadow], 0);
        }
    }
}

- (BOOL)createBuffers
{
    // ===== Create offscreen compositing framebuffer object =====
    glGenFramebuffers(1, &compositeFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, compositeFramebuffer);
    
    // ----- Create texture color buffer for compositeFramebuffer -----
	glGenTextures(1, &compositeColorTexture);
	glBindTexture(GL_TEXTURE_2D, compositeColorTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is necessary for non-power-of-two textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // This is necessary for non-power-of-two textures
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, backingWidth, backingHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, compositeColorTexture, 0);
    
    // ----- Create Depth Texture for compositeFramebuffer -----
    glGenTextures(1, &compositeDepthTexture);
	glBindTexture(GL_TEXTURE_2D, compositeDepthTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is necessary for non-power-of-two textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // This is necessary for non-power-of-two textures
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, backingWidth, backingHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, compositeDepthTexture, 0);

    [self allocateShadowBuffers];
    return TRUE;
}

- (void)destroyBuffers
{
    m_cShadowBuffers = 0;
    [self allocateShadowBuffers];
    
    if (compositeDepthTexture) {
        glDeleteTextures(1, &compositeDepthTexture);
        compositeDepthTexture = 0;
    }
    
    if (compositeColorTexture) {
		glDeleteTextures(1, &compositeColorTexture);
		compositeColorTexture = 0;
	}
    
    if (compositeFramebuffer) {
        glDeleteFramebuffers(1, &compositeFramebuffer);
        compositeFramebuffer = 0;
    }
}

- (void)renderScene: (KRScene *)pScene WithPosition: (Vector3)position Yaw: (GLfloat)yaw Pitch: (GLfloat)pitch Roll: (GLfloat)roll
{
    KRMat4 viewMatrix;
    viewMatrix.translate(-position.x, -position.y, -position.z);
    viewMatrix.rotate(yaw, Y_AXIS);
    viewMatrix.rotate(pitch, X_AXIS);
    viewMatrix.rotate(roll, Z_AXIS);
    [self renderScene: pScene WithViewMatrix: viewMatrix];
}

- (void)renderScene: (KRScene *)pScene WithViewMatrix: (KRMat4)viewMatrix
{
    KRMat4 invViewMatrix = viewMatrix;
    invViewMatrix.invert();
    
    Vector3 cameraPosition = invViewMatrix.dot(Vector3(0.0,0.0,0.0));

    Vector3 lightDirection(0.0, 0.0, 1.0);
    
    // ----- Render Model -----
    KRMat4 shadowvp;
    shadowvp.rotate(sun_pitch, X_AXIS);
    shadowvp.rotate(sun_yaw, Y_AXIS);
    lightDirection = shadowvp.dot(lightDirection);
    shadowvp.invert();
    
    
    lightDirection.normalize();
    
    [self allocateShadowBuffers];
    for(int iShadow=0; iShadow < m_cShadowBuffers; iShadow++) {
        if(!shadowValid[iShadow] || true) {
            shadowValid[iShadow] = true;
            
            GLfloat shadowMinDepths[3][3] = {{0.0, 0.0, 0.0},{0.0, 0.0, 0.0},{0.0, 0.05, 0.3}};
            GLfloat shadowMaxDepths[3][3] = {{0.0, 0.0, 1.0},{0.1, 0.0, 0.0},{0.1, 0.3, 1.0}};
            
            
            if(shadowMaxDepths[m_cShadowBuffers - 1][iShadow] == 0.0) {
                shadowmvpmatrix[iShadow] = pScene->getExtents().calcShadowProj(pScene, sun_yaw, sun_pitch);
            } else {
                KRBoundingVolume frustrumSliceVolume = KRBoundingVolume(viewMatrix, m_camera.perspective_fov, m_camera.perspective_aspect, m_camera.perspective_nearz + (m_camera.perspective_farz - m_camera.perspective_nearz) * shadowMinDepths[m_cShadowBuffers - 1][iShadow], m_camera.perspective_nearz + (m_camera.perspective_farz - m_camera.perspective_nearz) * shadowMaxDepths[m_cShadowBuffers - 1][iShadow]);
                shadowmvpmatrix[iShadow] = frustrumSliceVolume.calcShadowProj(pScene, sun_yaw, sun_pitch);
            }
            
            [self renderShadowBufferNumber: iShadow ForScene: pScene];
        }
    }
    
    /*
    NSLog(@"LightDirection: (%f, %f, %f)", lightDirection.x, lightDirection.y, lightDirection.z);
    NSLog(@"CameraPos: (%f, %f, %f)", cameraPos.x, cameraPos.y, cameraPos.z);
    NSLog(@"LightPosObject: (%f, %f, %f)", lightPosObject.x, lightPosObject.y, lightPosObject.z);
    NSLog(@"CameraPosObject: (%f, %f, %f)", cameraPosObject.x, cameraPosObject.y, cameraPosObject.z);
    */

    [self renderScene: pScene WithViewMatrix: viewMatrix LightDirection: lightDirection CameraPosition: cameraPosition];
    
    [self renderPost];
    
    m_iFrame++;
}

- (void)renderShadowBufferNumber: (int)iShadow ForScene: (KRScene *)pScene;
{
    glViewport(0, 0, KRENGINE_SHADOW_MAP_WIDTH-1, KRENGINE_SHADOW_MAP_HEIGHT-1);
    
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer[iShadow]);
    glClearDepthf(1.0f);
    glClear(GL_DEPTH_BUFFER_BIT);
    
    //glViewport(1, 1, 2046, 2046);
        
    glDisable(GL_DITHER);

    glCullFace(GL_BACK); // Enable frontface culling, which eliminates some self-cast shadow artifacts
    glEnable(GL_CULL_FACE);
    
    // Enable z-buffer test
    glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
    glDepthRangef(0.0, 1.0);
    
    // Disable alpha blending as we are using alpha channel for packed depth info
    glDisable(GL_BLEND);
    
    // Use shader program
    glUseProgram(m_shadowShaderProgram);
    
    // Sets the diffuseTexture variable to the first texture unit
    /*
	glUniform1i(glGetUniformLocation(m_shadowShaderProgram, "diffuseTexture"), 0);
     */
    
    // Validate program before drawing. This is a good check, but only really necessary in a debug build.
    // DEBUG macro must be defined in your debug configurations if that's not already the case.
#if defined(DEBUG)
    if (![self validateProgram:m_shadowShaderProgram])
    {
        NSLog(@"Failed to validate program: %d", m_shadowShaderProgram);
        return;
    }
#endif
    
    
    
    // Bind our modelmatrix variable to be a uniform called mvpmatrix in our shaderprogram
    glUniformMatrix4fv(m_shadowUniforms[KRENGINE_UNIFORM_SHADOWMVP1], 1, GL_FALSE, shadowmvpmatrix[iShadow].getPointer());
    
    
    // Calculate the bounding volume of the shadow map
    KRMat4 matInvShadow = shadowmvpmatrix[iShadow];
    matInvShadow.invert();

    Vector3 vertices[8];
    vertices[0] = Vector3(-1.0, -1.0, 0.0);
    vertices[1] = Vector3(1.0,  -1.0, 0.0);
    vertices[2] = Vector3(1.0,   1.0, 0.0);
    vertices[3] = Vector3(-1.0,  1.0, 0.0);
    vertices[4] = Vector3(-1.0, -1.0, 1.0);
    vertices[5] = Vector3(1.0,  -1.0, 1.0);
    vertices[6] = Vector3(1.0,   1.0, 1.0);
    vertices[7] = Vector3(-1.0,  1.0, 1.0);
    
    for(int iVertex=0; iVertex < 8; iVertex++) {
        vertices[iVertex] = matInvShadow.dot(vertices[iVertex]);
    }
    
    Vector3 cameraPosition;
    Vector3 lightDirection;
    KRBoundingVolume shadowVolume = KRBoundingVolume(vertices);
    pScene->render(&m_camera, shadowVolume, m_pMaterialManager, true, shadowmvpmatrix[iShadow], cameraPosition, lightDirection, shadowmvpmatrix, NULL, 0);
    
    glViewport(0, 0, 768, 1024);
}

- (void)renderScene: (KRScene *)pScene WithViewMatrix: (KRMat4)viewMatrix LightDirection: (Vector3)lightDirection CameraPosition: (Vector3)cameraPosition
{
    
    glBindFramebuffer(GL_FRAMEBUFFER, compositeFramebuffer);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Enable backface culling
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
    
    // Enable z-buffer test
    glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0, 1.0);
    
    // Enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    KRBoundingVolume frustrumVolume = KRBoundingVolume(viewMatrix, m_camera.perspective_fov, m_camera.perspective_aspect, m_camera.perspective_nearz, m_camera.perspective_farz);
    pScene -> render(&m_camera, frustrumVolume, m_pMaterialManager, false, viewMatrix, cameraPosition, lightDirection, shadowmvpmatrix, shadowDepthTexture, m_cShadowBuffers);
}

- (BOOL)compileShader:(GLuint *)shader type:(GLenum)type file:(NSString *)file withOptions: (NSString *)options
{
    GLint status;
    const GLchar *source[2];
    
    source[0] = (GLchar *)[[NSString stringWithContentsOfFile:file encoding:NSUTF8StringEncoding error:nil] UTF8String];
    if (!source[0])
    {
        NSLog(@"Failed to load vertex shader");
        return FALSE;
    }
    if(options) {
        source[1] = source[0];
        source[0] = [options UTF8String];
    }

    
    *shader = glCreateShader(type);
    glShaderSource(*shader, options ? 2 : 1, source, NULL);
    glCompileShader(*shader);
    
#if defined(DEBUG)
    GLint logLength;
    glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetShaderInfoLog(*shader, logLength, &logLength, log);
        NSLog(@"Shader compile log:\n%s", log);
        free(log);
    }
#endif
    
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);
    if (status == 0) {
        glDeleteShader(*shader);
        return FALSE;
    }
    
    return TRUE;
}

- (BOOL)linkProgram:(GLuint)prog
{
    GLint status;
    
    glLinkProgram(prog);
    
#if defined(DEBUG)
    GLint logLength;
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, log);
        NSLog(@"Program link log:\n%s", log);
        free(log);
    }
#endif
    
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if (status == 0)
        return FALSE;
    
    return TRUE;
}

- (BOOL)validateProgram:(GLuint)prog
{
    GLint logLength, status;
    
    glValidateProgram(prog);
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, log);
        NSLog(@"Program validate log:\n%s", log);
        free(log);
    }
    
    glGetProgramiv(prog, GL_VALIDATE_STATUS, &status);
    if (status == 0)
        return FALSE;
    
    return TRUE;
}


- (BOOL)loadVertexShader:(NSString *)vertexShaderName fragmentShader:(NSString *)fragmentShaderName forProgram:(GLuint *)programPointer withOptions:(NSString *)options;
{
    GLuint vertexShader, fragShader;
	
    NSString *vertShaderPathname, *fragShaderPathname;
    
    // Create shader program.
    *programPointer = glCreateProgram();
    
    // Create and compile vertex shader.
    
    vertShaderPathname = [[NSBundle mainBundle] pathForResource:vertexShaderName ofType:@"vsh"];
    if (![self compileShader:&vertexShader type:GL_VERTEX_SHADER file:vertShaderPathname withOptions: options])
    {
        NSLog(@"Failed to compile vertex shader");
        return FALSE;
    }
    
    // Create and compile fragment shader.
    fragShaderPathname = [[NSBundle mainBundle] pathForResource:fragmentShaderName ofType:@"fsh"];
    if (![self compileShader:&fragShader type:GL_FRAGMENT_SHADER file:fragShaderPathname withOptions: options])
    {
        NSLog(@"Failed to compile fragment shader");
        return FALSE;
    }
    
    // Attach vertex shader to program.
    glAttachShader(*programPointer, vertexShader);
    
    // Attach fragment shader to program.
    glAttachShader(*programPointer, fragShader);
    
    // Bind attribute locations.
    // This needs to be done prior to linking.
    glBindAttribLocation(*programPointer, KRShader::KRENGINE_ATTRIB_VERTEX, "position");
    glBindAttribLocation(*programPointer, KRShader::KRENGINE_ATTRIB_TEXUV, "inputTextureCoordinate");
    glBindAttribLocation(*programPointer, KRShader::KRENGINE_ATTRIB_VERTEX, "myVertex");
    glBindAttribLocation(*programPointer, KRShader::KRENGINE_ATTRIB_NORMAL, "myNormal");
    glBindAttribLocation(*programPointer, KRShader::KRENGINE_ATTRIB_TANGENT, "myTangent");
    glBindAttribLocation(*programPointer, KRShader::KRENGINE_ATTRIB_TEXUV, "myUV");
    
    // Link program.
    if (![self linkProgram:*programPointer])
    {
        NSLog(@"Failed to link program: %d", *programPointer);
        
        if (vertexShader)
        {
            glDeleteShader(vertexShader);
            vertexShader = 0;
        }
        if (fragShader)
        {
            glDeleteShader(fragShader);
            fragShader = 0;
        }
        if (*programPointer)
        {
            glDeleteProgram(*programPointer);
            *programPointer = 0;
        }
        
        return FALSE;
    }
    
    // Release vertex and fragment shaders.
    if (vertexShader)
	{
        glDeleteShader(vertexShader);
	}
    if (fragShader)
	{
        glDeleteShader(fragShader);		
	}
    
    return TRUE;
}

- (BOOL)loadShaders
{
    [self loadVertexShader:@"ShadowShader" fragmentShader:@"ShadowShader" forProgram:&m_shadowShaderProgram withOptions: NULL];
    
    m_shadowUniforms[KRENGINE_UNIFORM_SHADOWMVP1] = glGetUniformLocation(m_shadowShaderProgram, "myShadowMVPMatrix1");

    
    return TRUE;
}

- (BOOL)loadResource:(NSString *)path
{
    NSString *name = [[path lastPathComponent] stringByDeletingPathExtension];
    if([path hasSuffix: @".krobject"]) {
        NSLog(@"object: %@", path);
        m_pModelManager->loadModel([name UTF8String], [path UTF8String]);
    } else if([path hasSuffix: @".pvr"]) {
        NSLog(@"texture: %@", path);
        m_pTextureManager->loadTexture([name UTF8String], [path UTF8String]);
    } else if([path hasSuffix: @".mtl"]) {
        NSLog(@"material: %@", path);
        m_pMaterialManager->loadFile([path UTF8String]);
    }
    
    return TRUE;
}

- (void)dealloc
{    
    if(m_pModelManager) {
        delete m_pModelManager;
        m_pModelManager = NULL;
    }
    
    if(m_pTextureManager) {
        delete m_pTextureManager;
        m_pTextureManager = NULL;
    }
    
    if(m_pMaterialManager) {
        delete m_pMaterialManager;
        m_pMaterialManager = NULL;
    }
    
    if(m_pShaderManager) {
        delete m_pShaderManager;
        m_pShaderManager = NULL;
    }
    

    
    [self invalidatePostShader];
    [self destroyBuffers];
    [super dealloc];
}

- (void)invalidatePostShader
{
    if(m_postShaderProgram) {
        glDeleteProgram(m_postShaderProgram);
        m_postShaderProgram = 0;
    }
}

- (void)bindPostShader
{
    if(!m_postShaderProgram) {
        stringstream stream;
        stream.precision(std::numeric_limits<long double>::digits10);
        
        stream << "#define DOF_QUALITY " << m_camera.dof_quality;
        stream << "\n#define ENABLE_FLASH " << (m_camera.bEnableFlash ? "1" : "0");
        stream << "\n#define ENABLE_VIGNETTE " << (m_camera.bEnableVignette ? "1" : "0");
        stream.setf(ios::fixed,ios::floatfield);
        stream << "\n#define DOF_DEPTH " << m_camera.dof_depth;
        stream << "\n#define DOF_FALLOFF " << m_camera.dof_falloff;
        stream << "\n#define FLASH_DEPTH " << m_camera.flash_depth;
        stream << "\n#define FLASH_FALLOFF " << m_camera.flash_falloff;
        stream << "\n#define FLASH_INTENSITY " << m_camera.flash_intensity;
        stream << "\n#define VIGNETTE_RADIUS " << m_camera.vignette_radius;
        stream << "\n#define VIGNETTE_FALLOFF " << m_camera.vignette_falloff;

        stream << "\n";
        NSString *options = [NSString stringWithUTF8String: stream.str().c_str()];
        
        [self loadVertexShader:@"PostShader" fragmentShader:@"PostShader" forProgram:&m_postShaderProgram withOptions: options];
    }
    glUseProgram(m_postShaderProgram);
}

- (void)renderPost
{
    
    glBindFramebuffer(GL_FRAMEBUFFER, 1); // renderFramebuffer
    
    // Replace the implementation of this method to do your own custom drawing.
    static const GLfloat squareVertices[] = {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        -1.0f,  1.0f,
        1.0f,  1.0f,
    };
    
    static const GLfloat squareVerticesShadow[3][8] = {{
        -1.0f, -1.0f,
        -0.60f, -1.0f,
        -1.0f,  -0.60f,
        -0.60f,  -0.60f,
    },{
        -0.50f, -1.0f,
        -0.10f, -1.0f,
        -0.50f,  -0.60f,
        -0.10f,  -0.60f,
    },{
        0.00f, -1.0f,
        0.40f, -1.0f,
        0.00f,  -0.60f,
        0.40f,  -0.60f,
    }};
	
	static const GLfloat textureVertices[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f,  1.0f,
        1.0f,  1.0f,
    };
	
    glDisable(GL_DEPTH_TEST);
	[self bindPostShader];
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, compositeDepthTexture);
    glUniform1i(glGetUniformLocation(m_postShaderProgram, "depthFrame"), 0);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, compositeColorTexture);
    
    glUniform1i(glGetUniformLocation(m_postShaderProgram, "renderFrame"), 1);
	
	// Update attribute values.
	glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, squareVertices);
	glEnableVertexAttribArray(KRShader::KRENGINE_ATTRIB_VERTEX);
	glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_TEXUV, 2, GL_FLOAT, 0, 0, textureVertices);
	glEnableVertexAttribArray(KRShader::KRENGINE_ATTRIB_TEXUV);
	
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    
    if(m_camera.bShowShadowBuffer) {
        glDisable(GL_DEPTH_TEST);
        glUseProgram(m_postShaderProgram);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, compositeDepthTexture);
        glUniform1i(glGetUniformLocation(m_postShaderProgram, "depthFrame"), 0);
        
        
        glUniform1i(glGetUniformLocation(m_postShaderProgram, "renderFrame"), 1);
        
        // Update attribute values.

        glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_TEXUV, 2, GL_FLOAT, 0, 0, textureVertices);
        glEnableVertexAttribArray(KRShader::KRENGINE_ATTRIB_TEXUV);
        
        for(int iShadow=0; iShadow < m_cShadowBuffers; iShadow++) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, shadowDepthTexture[iShadow]);
            glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, squareVerticesShadow[iShadow]);
            glEnableVertexAttribArray(KRShader::KRENGINE_ATTRIB_VERTEX);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    
    
    const char *szText = [debug_text UTF8String];
    if(*szText) {
        KRTexture *pFontTexture = m_pTextureManager->getTexture("font");
        
        glDisable(GL_DEPTH_TEST);
        glUseProgram(m_postShaderProgram);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, compositeDepthTexture);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pFontTexture->getName());
        
        glUniform1i(glGetUniformLocation(m_postShaderProgram, "depthFrame"), 0);
        glUniform1i(glGetUniformLocation(m_postShaderProgram, "renderFrame"), 1);
        
        const char *pChar = szText;
        int iPos=0;
        double dScale = 1.0 / 24.0;
        double dTexScale = 1.0 / 16.0;
        while(*pChar) {
            int iChar = *pChar++ - '\0';
            int iCol = iChar % 16;
            int iRow = 15 - (iChar - iCol) / 16;
            
            GLfloat charVertices[] = {
                -1.0f,              dScale * iPos - 1.0,
                -1.0 + dScale,      dScale * iPos - 1.0,
                -1.0f,              dScale * iPos + dScale - 1.0,
                -1.0 + dScale,      dScale * iPos + dScale - 1.0,
            };
            
            /*
             GLfloat charTexCoords[] = {
                dTexScale * iCol,                 dTexScale * iRow,
                dTexScale * iCol + dTexScale,     dTexScale * iRow,
                dTexScale * iCol,                 dTexScale * iRow + dTexScale,
                dTexScale * iCol + dTexScale,     dTexScale * iRow + dTexScale
            };
             */
            
            GLfloat charTexCoords[] = {
                dTexScale * iCol,                 dTexScale * iRow + dTexScale,
                dTexScale * iCol,                 dTexScale * iRow,
                dTexScale * iCol + dTexScale,     dTexScale * iRow + dTexScale,
                dTexScale * iCol + dTexScale,     dTexScale * iRow
            };
            
            glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_TEXUV, 2, GL_FLOAT, 0, 0, charTexCoords);
            glEnableVertexAttribArray(KRShader::KRENGINE_ATTRIB_TEXUV);
            
            glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, charVertices);
            glEnableVertexAttribArray(KRShader::KRENGINE_ATTRIB_VERTEX);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            
            iPos++;
        }
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
}

-(int)getParameterCount
{
    return 30;
}

-(NSString *)getParameterNameWithIndex: (int)i
{
    NSString *parameter_names[30] = {
        @"camera_fov",
        @"sun_direction",
        @"sun_attitude",
        @"shadow_quality",
        @"enable_per_pixel",
        @"enable_diffuse_map",
        @"enable_normal_map",
        @"enable_spec_map",
        @"ambient_r",
        @"ambient_g",
        @"ambient_b",
        @"sun_r",
        @"sun_g",
        @"sun_b",
        @"dof_quality",
        @"dof_depth",
        @"dof_falloff",
        @"flash_enable",
        @"flash_intensity",
        @"flash_depth",
        @"flash_falloff",
        @"vignette_enable",
        @"vignette_radius",
        @"vignette_falloff",
        @"debug_shadowmap",
        @"debug_pssm",
        @"debug_enable_ambient",
        @"debug_enable_diffuse",
        @"debug_enable_specular",
        @"debug_super_shiny"
    };
    return parameter_names[i];
}
-(NSString *)getParameterLabelWithIndex: (int)i
{
    NSString *parameter_labels[30] = {
        @"Camera FOV",
        @"Sun Direction",
        @"Sun Attitude",
        @"Shadow Quality (0 - 2)",
        @"Enable per-pixel lighting",
        @"Enable diffuse map",
        @"Enable normal map",
        @"Enable specular map",
        @"Ambient light red intensity",
        @"Ambient light green intensity",
        @"Ambient light blue intensity",
        @"Sun red intensity",
        @"Sun green intensity",
        @"Sun blue intensity",
        @"DOF Quality",
        @"DOF Depth",
        @"DOF Falloff",
        @"Enable Night/Flash Effect",
        @"Flash Intensity",
        @"Flash Depth",
        @"Flash Falloff",
        @"Enable Vignette",
        @"Vignette Radius",
        @"Vignette Falloff",
        @"Debug - View Shadow Map",
        @"Debug - PSSM",
        @"Debug - Enable Ambient",
        @"Debug - Enable Diffuse",
        @"Debug - Enable Specular",
        @"Debug - Super Shiny"
    };
    return parameter_labels[i];
}
-(KREngineParameterType)getParameterTypeWithIndex: (int)i
{
    KREngineParameterType types[30] = {
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_INT,
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_INT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_BOOL
    };
    return types[i];
}
-(double)getParameterValueWithIndex: (int)i
{
    double values[30] = {
        m_camera.perspective_fov,
        sun_yaw,
        sun_pitch,
        (double)m_cShadowBuffers,
        m_camera.bEnablePerPixel ? 1.0f : 0.0f,
        m_camera.bEnableDiffuseMap ? 1.0f : 0.0f,
        m_camera.bEnableNormalMap ? 1.0f : 0.0f,
        m_camera.bEnableSpecMap ? 1.0f : 0.0f,
        m_camera.dAmbientR,
        m_camera.dAmbientG,
        m_camera.dAmbientB,
        m_camera.dSunR,
        m_camera.dSunG,
        m_camera.dSunB,
        m_camera.dof_quality,
        m_camera.dof_depth,
        m_camera.dof_falloff,
        m_camera.bEnableFlash ? 1.0f : 0.0f,
        m_camera.flash_intensity,
        m_camera.flash_depth,
        m_camera.flash_falloff,
        m_camera.bEnableVignette ? 1.0f : 0.0f,
        m_camera.vignette_radius,
        m_camera.vignette_falloff,
        m_camera.bShowShadowBuffer ? 1.0f : 0.0f,
        m_camera.bDebugPSSM ? 1.0f : 0.0f,
        m_camera.bEnableAmbient ? 1.0f : 0.0f,
        m_camera.bEnableDiffuse ? 1.0f : 0.0f,
        m_camera.bEnableSpecular ? 1.0f : 0.0f,
        m_camera.bDebugSuperShiny ? 1.0f : 0.0f
    };
    return values[i];
}
-(void)setParameterValueWithIndex: (int)i Value: (double)v
{
    bool bNewBoolVal = v > 0.5;
    NSLog(@"Set Parameter: (%s, %f)", [[self getParameterNameWithIndex: i] UTF8String], v);
    switch(i) {
        case 0: // FOV
            m_camera.perspective_fov = v;
            break;
        case 1: // sun_yaw
            if(sun_yaw != v) {
                sun_yaw = v;
                [self invalidateShadowBuffers];
            }
            break;
        case 2: // sun_pitch
            if(sun_pitch != v) {
                sun_pitch = v;
                [self invalidateShadowBuffers];
            }
            break;
        case 3: // Shadow Quality
            m_cShadowBuffers = (int)v;
            break;
        case 4:
            m_camera.bEnablePerPixel = bNewBoolVal;
            break;
        case 5:
            m_camera.bEnableDiffuseMap = bNewBoolVal;
            break;
        case 6:
            m_camera.bEnableNormalMap = bNewBoolVal;
            break;
        case 7:
            m_camera.bEnableSpecMap = bNewBoolVal;
            break;
        case 8:
            m_camera.dAmbientR = v;
            break;
        case 9:
            m_camera.dAmbientG = v;
            break;
        case 10:
            m_camera.dAmbientB = v;
            break;
        case 11:
            m_camera.dSunR = v;
            break;
        case 12:
            m_camera.dSunG = v;
            break;
        case 13:
            m_camera.dSunB = v;
            break;
        case 14:
            if(m_camera.dof_quality != (int)v) {
                m_camera.dof_quality = (int)v;
                [self invalidatePostShader];
            }
            break;
        case 15:
            if(m_camera.dof_depth != v) {
                m_camera.dof_depth = v;
                [self invalidatePostShader];
            }
            break;
        case 16:
            if(m_camera.dof_falloff != v) {
                m_camera.dof_falloff = v;
                [self invalidatePostShader];
            }
            break;
        case 17:
            if(m_camera.bEnableFlash != bNewBoolVal) {
                m_camera.bEnableFlash = bNewBoolVal;
                [self invalidatePostShader];
            }
            break;
        case 18:
            if(m_camera.flash_intensity != v) {
                m_camera.flash_intensity = v;
                [self invalidatePostShader];
            }
            break;
        case 19:
            if(m_camera.flash_depth != v) {
                m_camera.flash_depth = v;
                [self invalidatePostShader];
            }
            break;
        case 20:
            if(m_camera.flash_falloff != v) {
                m_camera.flash_falloff = v;
                [self invalidatePostShader];
            }
            break;
        case 21:
            if(m_camera.bEnableVignette != bNewBoolVal) {
                m_camera.bEnableVignette = bNewBoolVal;
                [self invalidatePostShader];
            }
            break;
        case 22:
            if(m_camera.vignette_radius != v) {
                m_camera.vignette_radius = v;
                [self invalidatePostShader];
            }
            break;
        case 23:
            if(m_camera.vignette_falloff != v) {
                m_camera.vignette_falloff = v;
                [self invalidatePostShader];
            }
            break;
        case 24:
            if(m_camera.bShowShadowBuffer != bNewBoolVal) {
                m_camera.bShowShadowBuffer = bNewBoolVal;
            }
            break;
        case 25:
            if(m_camera.bDebugPSSM != bNewBoolVal) {
                m_camera.bDebugPSSM = bNewBoolVal;
            }
            break;
        case 26:
            if(m_camera.bEnableAmbient != bNewBoolVal) {
                m_camera.bEnableAmbient = bNewBoolVal;
            }
            break;
        case 27:
            if(m_camera.bEnableDiffuse != bNewBoolVal) {
                m_camera.bEnableDiffuse = bNewBoolVal;
            }
            break;
        case 28:
            if(m_camera.bEnableSpecular != bNewBoolVal) {
                m_camera.bEnableSpecular = bNewBoolVal;
            }
            break;
        case 29:
            if(m_camera.bDebugSuperShiny != bNewBoolVal) {
                m_camera.bDebugSuperShiny = bNewBoolVal;
            }
            break;
    }
}

-(double)getParameterMinWithIndex: (int)i
{
    double minValues[30] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    return minValues[i];
}

-(double)getParameterMaxWithIndex: (int)i
{
    double maxValues[30] = {PI, 2.0f * PI, PI, 3.0f, 1.0f, 1.0f, 1.0f, 1.0f, 3.0f, 3.0f, 3.0f, 3.0f, 3.0f, 3.0f, 2.0f, 1.0f, 1.0f, 1.0f, 5.0f, 1.0f, 0.5f, 1.0f, 2.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    return maxValues[i];
}

-(void)setParameterValueWithName: (NSString *)name Value: (double)v
{
    int cParameters = [self getParameterCount];
    for(int i=0; i < cParameters; i++) {
        if([[self getParameterNameWithIndex: i] caseInsensitiveCompare:name] == NSOrderedSame) {
            [self setParameterValueWithIndex:i Value:v];
        }
    }
}

- (KRModelManager *)getModelManager {
    return m_pModelManager;
}

- (void)invalidateShadowBuffers {
    for(int i=0; i < m_cShadowBuffers; i++) {
        shadowValid[i] = false;
    }
}

- (void)setNearZ: (double)dNearZ
{
    if(m_camera.perspective_nearz != dNearZ) {
        m_camera.perspective_nearz = dNearZ;
        [self invalidateShadowBuffers];
    }
}
- (void)setFarZ: (double)dFarZ
{
    if(m_camera.perspective_farz != dFarZ) {
        m_camera.perspective_farz = dFarZ;
        [self invalidateShadowBuffers];
    }
}
- (void)setAspect: (double)dAspect
{
    if(m_camera.perspective_aspect != dAspect) {
        m_camera.perspective_aspect = dAspect;
        [self invalidateShadowBuffers];
    }
}

- (void)setDebugText: (NSString *)text
{
    debug_text = text;
}


@end
