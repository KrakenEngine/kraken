//
//  KREngine.mm
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

#import "KREngine.h"
#import "KRVector3.h"
#import "KRScene.h"
#import "KRSceneManager.h"

#import <string>
#import <sstream> 

using namespace std;



@interface KREngine (PrivateMethods)
- (BOOL)loadShaders;
- (BOOL)createBuffers;
- (BOOL)compileShader:(GLuint *)shader type:(GLenum)type file:(NSString *)file;
- (BOOL)linkProgram:(GLuint)prog;
- (BOOL)validateProgram:(GLuint)prog;
- (void)renderPost;
- (BOOL)loadResource:(NSString *)path;
@end

@implementation KREngine
@synthesize context = m_pContext;
double const PI = 3.141592653589793f;

- (id)initForWidth: (GLuint)width Height: (GLuint)height
{
    debug_text = [[NSString alloc] init];
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
        
        m_pContext = new KRContext();

        
        if (![self createBuffers] || ![self loadShaders] )
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
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, backingWidth, backingHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
    //glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24_OES, backingWidth, backingHeight);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, compositeDepthTexture, 0);

    
    // ===== Create offscreen compositing framebuffer object =====
    glGenFramebuffers(1, &lightAccumulationBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, lightAccumulationBuffer);
    
    // ----- Create texture color buffer for compositeFramebuffer -----
	glGenTextures(1, &lightAccumulationTexture);
	glBindTexture(GL_TEXTURE_2D, lightAccumulationTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is necessary for non-power-of-two textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // This is necessary for non-power-of-two textures
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, backingWidth, backingHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightAccumulationTexture, 0);
    
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
    
    if (lightAccumulationTexture) {
        glDeleteTextures(1, &lightAccumulationTexture);
        lightAccumulationTexture = 0;
    }
    
    if (compositeFramebuffer) {
        glDeleteFramebuffers(1, &compositeFramebuffer);
        compositeFramebuffer = 0;
    }
    
    if (lightAccumulationBuffer) {
        glDeleteFramebuffers(1, &lightAccumulationBuffer);
        lightAccumulationBuffer = 0;
    }
}

- (void)renderScene: (KRScene *)pScene WithPosition: (KRVector3)position Yaw: (GLfloat)yaw Pitch: (GLfloat)pitch Roll: (GLfloat)roll
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
    viewMatrix.rotate(-90 * 0.0174532925199, Z_AXIS);
    
    KRMat4 invViewMatrix = viewMatrix;
    invViewMatrix.invert();
    
    KRVector3 cameraPosition = invViewMatrix.dot(KRVector3(0.0,0.0,0.0));

    KRVector3 lightDirection(0.0, 0.0, 1.0);
    
    // ----- Render Model -----
    KRMat4 shadowvp;
    shadowvp.rotate(sun_pitch, X_AXIS);
    shadowvp.rotate(sun_yaw, Y_AXIS);
    lightDirection = shadowvp.dot(lightDirection);
    shadowvp.invert();
    
    
    lightDirection.normalize();
    
    [self allocateShadowBuffers];
    int iOffset=m_iFrame % m_cShadowBuffers;
    for(int iShadow2=iOffset; iShadow2 < m_cShadowBuffers + iOffset; iShadow2++) {
        int iShadow = iShadow2 % m_cShadowBuffers;
        
        
        GLfloat shadowMinDepths[3][3] = {{0.0, 0.0, 0.0},{0.0, 0.0, 0.0},{0.0, 0.05, 0.3}};
        GLfloat shadowMaxDepths[3][3] = {{0.0, 0.0, 1.0},{0.1, 0.0, 0.0},{0.1, 0.3, 1.0}};
        
        
        KRMat4 newShadowMVP;
        if(shadowMaxDepths[m_cShadowBuffers - 1][iShadow] == 0.0) {
            KRBoundingVolume ext = KRBoundingVolume(pScene->getExtents(m_pContext));
            
            newShadowMVP = ext.calcShadowProj(pScene, m_pContext, sun_yaw, sun_pitch);
        } else {
            KRBoundingVolume frustrumSliceVolume = KRBoundingVolume(viewMatrix, m_camera.perspective_fov, m_camera.getViewportSize().x / m_camera.getViewportSize().y, m_camera.perspective_nearz + (m_camera.perspective_farz - m_camera.perspective_nearz) * shadowMinDepths[m_cShadowBuffers - 1][iShadow], m_camera.perspective_nearz + (m_camera.perspective_farz - m_camera.perspective_nearz) * shadowMaxDepths[m_cShadowBuffers - 1][iShadow]);
            newShadowMVP = frustrumSliceVolume.calcShadowProj(pScene, m_pContext, sun_yaw, sun_pitch);
        }
        
        if(!(shadowmvpmatrix[iShadow] == newShadowMVP)) {
            shadowValid[iShadow] = false;
        }
        
        if(!shadowValid[iShadow]) {
            shadowValid[iShadow] = true;
            
            shadowmvpmatrix[iShadow] = newShadowMVP;
            
            [self renderShadowBufferNumber: iShadow ForScene: pScene];
            
            break;
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
    
    
    // Calculate the bounding volume of the light map
    KRMat4 matInvShadow = shadowmvpmatrix[iShadow];
    matInvShadow.invert();

    KRVector3 vertices[8];
    vertices[0] = KRVector3(-1.0, -1.0, 0.0);
    vertices[1] = KRVector3(1.0,  -1.0, 0.0);
    vertices[2] = KRVector3(1.0,   1.0, 0.0);
    vertices[3] = KRVector3(-1.0,  1.0, 0.0);
    vertices[4] = KRVector3(-1.0, -1.0, 1.0);
    vertices[5] = KRVector3(1.0,  -1.0, 1.0);
    vertices[6] = KRVector3(1.0,   1.0, 1.0);
    vertices[7] = KRVector3(-1.0,  1.0, 1.0);
    
    for(int iVertex=0; iVertex < 8; iVertex++) {
        vertices[iVertex] = matInvShadow.dot(vertices[iVertex]);
    }
    
    KRVector3 cameraPosition;
    KRVector3 lightDirection;
    KRBoundingVolume shadowVolume = KRBoundingVolume(vertices);
    pScene->render(&m_camera, m_pContext, shadowVolume, true, shadowmvpmatrix[iShadow], cameraPosition, lightDirection, shadowmvpmatrix, NULL, m_cShadowBuffers, 0);
    glViewport(0, 0, backingWidth, backingHeight);
}

- (void)renderScene: (KRScene *)pScene WithViewMatrix: (KRMat4)viewMatrix LightDirection: (KRVector3)lightDirection CameraPosition: (KRVector3)cameraPosition
{
    m_camera.setViewportSize(KRVector2(backingWidth, backingHeight));

    

    
    KRBoundingVolume frustrumVolume = KRBoundingVolume(viewMatrix, m_camera.perspective_fov, m_camera.getViewportSize().x / m_camera.getViewportSize().y, m_camera.perspective_nearz, m_camera.perspective_farz);
    if(m_camera.bEnableDeferredLighting) {
        //  ----====---- Opaque Geometry, Deferred rendering Pass 1 ----====----
        
        // Set render target
        glBindFramebuffer(GL_FRAMEBUFFER, compositeFramebuffer);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Enable backface culling
        glCullFace(GL_BACK);
        glEnable(GL_CULL_FACE);
        
        // Enable z-buffer write
        glDepthMask(GL_TRUE);
        
        // Enable z-buffer test
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glDepthRangef(0.0, 1.0);
        
        // Disable alpha blending
        glDisable(GL_BLEND);
        
        // Render the geometry
        pScene->render(&m_camera, m_pContext, frustrumVolume, false, viewMatrix, cameraPosition, lightDirection, shadowmvpmatrix, shadowDepthTexture, m_cShadowBuffers, 1);
        
        //  ----====---- Opaque Geometry, Deferred rendering Pass 2 ----====----
        // Set render target
        glBindFramebuffer(GL_FRAMEBUFFER, lightAccumulationBuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, compositeDepthTexture, 0);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Enable additive blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        
        // Disable z-buffer write
        glDepthMask(GL_FALSE);
        
        // Set source to buffers from pass 1
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, compositeColorTexture);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, compositeDepthTexture);
        
        
        // Render the geometry
        pScene->render(&m_camera, m_pContext, frustrumVolume, false, viewMatrix, cameraPosition, lightDirection, shadowmvpmatrix, shadowDepthTexture, 0, 2);
        
        //  ----====---- Opaque Geometry, Deferred rendering Pass 3 ----====----
        // Set render target
        glBindFramebuffer(GL_FRAMEBUFFER, compositeFramebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, compositeDepthTexture, 0);
        
        // Disable alpha blending
        glDisable(GL_BLEND);
        
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Set source to buffers from pass 2
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, lightAccumulationTexture);
        
        // Enable backface culling
        glCullFace(GL_BACK);
        glEnable(GL_CULL_FACE);
        
        // Enable z-buffer test
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glDepthRangef(0.0, 1.0);
        
        // Enable z-buffer write
        glDepthMask(GL_TRUE);
        
        // Render the geometry
        pScene->render(&m_camera, m_pContext, frustrumVolume, false, viewMatrix, cameraPosition, lightDirection, shadowmvpmatrix, shadowDepthTexture, m_cShadowBuffers, 3);
        
        // Deactivate source buffer texture units
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, 0);
    } else {
        // ----====---- Opaque Geometry, Forward Rendering ----====---- 
        
        // Set render target
        glBindFramebuffer(GL_FRAMEBUFFER, compositeFramebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, compositeDepthTexture, 0);
        
        // Disable alpha blending
        glDisable(GL_BLEND);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        
        // Enable backface culling
        glCullFace(GL_BACK);
        glEnable(GL_CULL_FACE);
        
        // Enable z-buffer write
        glDepthMask(GL_TRUE);
        
        // Enable z-buffer test
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glDepthRangef(0.0, 1.0);
        

        
        // Render the geometry
        pScene->render(&m_camera, m_pContext, frustrumVolume, false, viewMatrix, cameraPosition, lightDirection, shadowmvpmatrix, shadowDepthTexture, m_cShadowBuffers, 0);
    }
    
    // ----====---- Transparent Geometry, Forward Rendering ----====----
    
    // Set render target
    glBindFramebuffer(GL_FRAMEBUFFER, compositeFramebuffer);
    
    // Enable backface culling
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    
    // Enable z-buffer write
    glDepthMask(GL_TRUE);
    
    // Enable z-buffer test
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0, 1.0);
    
    // Enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // TODO: Need to perform a forward render of all transparent geometry here...
    //pScene->render(&m_camera, m_pContext, frustrumVolume, false, viewMatrix, cameraPosition, lightDirection, shadowmvpmatrix, shadowDepthTexture, m_cShadowBuffers, 0);
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
    glBindAttribLocation(*programPointer, KRShader::KRENGINE_ATTRIB_TEXUVB, "vertex_lightmap_uv");
    glBindAttribLocation(*programPointer, KRShader::KRENGINE_ATTRIB_VERTEX, "vertex_position");
    glBindAttribLocation(*programPointer, KRShader::KRENGINE_ATTRIB_NORMAL, "vertex_normal");
    glBindAttribLocation(*programPointer, KRShader::KRENGINE_ATTRIB_TANGENT, "vertex_tangent");
    glBindAttribLocation(*programPointer, KRShader::KRENGINE_ATTRIB_TEXUVA, "vertex_uv");
    
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
    NSFileManager* fileManager = [NSFileManager defaultManager];
    NSString *bundle_directory = [[NSBundle mainBundle] bundlePath];
    for (NSString* fileName in [fileManager contentsOfDirectoryAtPath: bundle_directory error:nil]) {
        if([fileName hasSuffix: @".vsh"] || [fileName hasSuffix: @".fsh"]) {
            NSString* path = [NSString stringWithFormat:@"%@/%@", bundle_directory, fileName];
            m_pContext->loadResource([path UTF8String]);
        }
    }
    
    [self loadVertexShader:@"ShadowShader" fragmentShader:@"ShadowShader" forProgram:&m_shadowShaderProgram withOptions: NULL];
    
    m_shadowUniforms[KRENGINE_UNIFORM_SHADOWMVP1] = glGetUniformLocation(m_shadowShaderProgram, "shadow_mvp1");

    
    return TRUE;
}

- (BOOL)loadResource:(NSString *)path
{
    m_pContext->loadResource([path UTF8String]);
    
    return TRUE;
}

- (void)dealloc
{
    if(m_pContext) {
        delete m_pContext;
        m_pContext = NULL;
    }
    
    [self invalidatePostShader];
    [self destroyBuffers];
    [debug_text release];
    debug_text = nil;
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
    
    // Disable alpha blending
    glDisable(GL_BLEND);
    
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
    //glBindTexture(GL_TEXTURE_2D, lightAccumulationTexture);
    
    glUniform1i(glGetUniformLocation(m_postShaderProgram, "renderFrame"), 1);
	
	// Update attribute values.
	glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, squareVertices);
	glEnableVertexAttribArray(KRShader::KRENGINE_ATTRIB_VERTEX);
	glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_TEXUVA, 2, GL_FLOAT, 0, 0, textureVertices);
	glEnableVertexAttribArray(KRShader::KRENGINE_ATTRIB_TEXUVA);
	
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

        glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_TEXUVA, 2, GL_FLOAT, 0, 0, textureVertices);
        glEnableVertexAttribArray(KRShader::KRENGINE_ATTRIB_TEXUVA);
        
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
        KRTexture *pFontTexture = m_pContext->getTextureManager()->getTexture("font");
        
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
            
            GLfloat charTexCoords[] = {
                dTexScale * iCol,                 dTexScale * iRow + dTexScale,
                dTexScale * iCol,                 dTexScale * iRow,
                dTexScale * iCol + dTexScale,     dTexScale * iRow + dTexScale,
                dTexScale * iCol + dTexScale,     dTexScale * iRow
            };
            
            glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_TEXUVA, 2, GL_FLOAT, 0, 0, charTexCoords);
            glEnableVertexAttribArray(KRShader::KRENGINE_ATTRIB_TEXUVA);
            
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
    return 32;
}

-(NSString *)getParameterNameWithIndex: (int)i
{
    NSString *parameter_names[32] = {
        @"camera_fov",
        @"sun_direction",
        @"sun_attitude",
        @"shadow_quality",
        @"enable_per_pixel",
        @"enable_diffuse_map",
        @"enable_normal_map",
        @"enable_spec_map",
        @"enable_light_map",
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
        @"debug_super_shiny",
        @"enable_deferred_lighting"
    };
    return parameter_names[i];
}
-(NSString *)getParameterLabelWithIndex: (int)i
{
    NSString *parameter_labels[32] = {
        @"Camera FOV",
        @"Sun Direction",
        @"Sun Attitude",
        @"Shadow Quality (0 - 2)",
        @"Enable per-pixel lighting",
        @"Enable diffuse map",
        @"Enable normal map",
        @"Enable specular map",
        @"Enable light map",
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
        @"Debug - View Shadow Volume",
        @"Debug - PSSM",
        @"Debug - Enable Ambient",
        @"Debug - Enable Diffuse",
        @"Debug - Enable Specular",
        @"Debug - Super Shiny",
        @"Enable Deferred Lighting"
    };
    return parameter_labels[i];
}
-(KREngineParameterType)getParameterTypeWithIndex: (int)i
{
    KREngineParameterType types[32] = {
        
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_FLOAT,
        KRENGINE_PARAMETER_INT,
        KRENGINE_PARAMETER_BOOL,
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
        KRENGINE_PARAMETER_BOOL,
        KRENGINE_PARAMETER_BOOL
    };
    return types[i];
}
-(double)getParameterValueWithIndex: (int)i
{
    double values[32] = {
        m_camera.perspective_fov,
        sun_yaw,
        sun_pitch,
        (double)m_cShadowBuffers,
        m_camera.bEnablePerPixel ? 1.0f : 0.0f,
        m_camera.bEnableDiffuseMap ? 1.0f : 0.0f,
        m_camera.bEnableNormalMap ? 1.0f : 0.0f,
        m_camera.bEnableSpecMap ? 1.0f : 0.0f,
        m_camera.bEnableLightMap ? 1.0f : 0.0f,
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
        m_camera.bDebugSuperShiny ? 1.0f : 0.0f,
        m_camera.bEnableDeferredLighting ? 1.0f : 0.0f
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
            m_camera.bEnableLightMap = bNewBoolVal;
            break;
        case 9:
            m_camera.dAmbientR = v;
            break;
        case 10:
            m_camera.dAmbientG = v;
            break;
        case 11:
            m_camera.dAmbientB = v;
            break;
        case 12:
            m_camera.dSunR = v;
            break;
        case 13:
            m_camera.dSunG = v;
            break;
        case 14:
            m_camera.dSunB = v;
            break;
        case 15:
            if(m_camera.dof_quality != (int)v) {
                m_camera.dof_quality = (int)v;
                [self invalidatePostShader];
            }
            break;
        case 16:
            if(m_camera.dof_depth != v) {
                m_camera.dof_depth = v;
                [self invalidatePostShader];
            }
            break;
        case 17:
            if(m_camera.dof_falloff != v) {
                m_camera.dof_falloff = v;
                [self invalidatePostShader];
            }
            break;
        case 18:
            if(m_camera.bEnableFlash != bNewBoolVal) {
                m_camera.bEnableFlash = bNewBoolVal;
                [self invalidatePostShader];
            }
            break;
        case 19:
            if(m_camera.flash_intensity != v) {
                m_camera.flash_intensity = v;
                [self invalidatePostShader];
            }
            break;
        case 20:
            if(m_camera.flash_depth != v) {
                m_camera.flash_depth = v;
                [self invalidatePostShader];
            }
            break;
        case 21:
            if(m_camera.flash_falloff != v) {
                m_camera.flash_falloff = v;
                [self invalidatePostShader];
            }
            break;
        case 22:
            if(m_camera.bEnableVignette != bNewBoolVal) {
                m_camera.bEnableVignette = bNewBoolVal;
                [self invalidatePostShader];
            }
            break;
        case 23:
            if(m_camera.vignette_radius != v) {
                m_camera.vignette_radius = v;
                [self invalidatePostShader];
            }
            break;
        case 24:
            if(m_camera.vignette_falloff != v) {
                m_camera.vignette_falloff = v;
                [self invalidatePostShader];
            }
            break;
        case 25:
            if(m_camera.bShowShadowBuffer != bNewBoolVal) {
                m_camera.bShowShadowBuffer = bNewBoolVal;
            }
            break;
        case 26:
            if(m_camera.bDebugPSSM != bNewBoolVal) {
                m_camera.bDebugPSSM = bNewBoolVal;
            }
            break;
        case 27:
            if(m_camera.bEnableAmbient != bNewBoolVal) {
                m_camera.bEnableAmbient = bNewBoolVal;
            }
            break;
        case 28:
            if(m_camera.bEnableDiffuse != bNewBoolVal) {
                m_camera.bEnableDiffuse = bNewBoolVal;
            }
            break;
        case 29:
            if(m_camera.bEnableSpecular != bNewBoolVal) {
                m_camera.bEnableSpecular = bNewBoolVal;
            }
            break;
        case 30:
            if(m_camera.bDebugSuperShiny != bNewBoolVal) {
                m_camera.bDebugSuperShiny = bNewBoolVal;
            }
            break;
        case 31:
            if(m_camera.bEnableDeferredLighting != bNewBoolVal) {
                m_camera.bEnableDeferredLighting = bNewBoolVal;
            }
    }
}

-(double)getParameterMinWithIndex: (int)i
{
    double minValues[32] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    return minValues[i];
}

-(double)getParameterMaxWithIndex: (int)i
{
    double maxValues[32] = {PI, 2.0f * PI, PI, 3.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 3.0f, 3.0f, 3.0f, 3.0f, 3.0f, 3.0f, 2.0f, 1.0f, 1.0f, 1.0f, 5.0f, 1.0f, 0.5f, 1.0f, 2.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
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

- (void)setDebugText: (NSString *)text
{
    [debug_text release];
    debug_text = [text retain];
}


@end
