//
//  KREngine.mm
//  gldemo
//
//  Created by Kearwood Gilbert on 10-09-16.
//  Copyright (c) 2010 Kearwood Software. All rights reserved.
//

#import "KREngine.h"




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

- (id)initForWidth: (GLuint)width Height: (GLuint)height
{
    backingWidth = width;
    backingHeight = height;
    
    if ((self = [super init]))
    {
        
        m_pTextureManager = new KRTextureManager();
        m_pMaterialManager = new KRMaterialManager(m_pTextureManager);
        
        if (![self createBuffers] || ![self loadShaders]/* || ![self loadObjects] */)
        {
            [self release];
            return nil;
        }
    }
    
    return self;
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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is necessary for non-power-of-two textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // This is necessary for non-power-of-two textures
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, backingWidth, backingHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, compositeDepthTexture, 0);

    
    
    // ===== Create offscreen shadow framebuffer object =====
    glGenFramebuffers(1, &shadowFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer);
    
    // ----- Create Depth Texture for shadowFramebuffer -----
    glGenTextures(1, &shadowDepthTexture);
	glBindTexture(GL_TEXTURE_2D, shadowDepthTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is necessary for non-power-of-two textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // This is necessary for non-power-of-two textures
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, backingWidth, backingHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepthTexture, 0);
    
    return TRUE;
}

- (void)destroyBuffers
{
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
    
    if (shadowDepthTexture) {
		glDeleteTextures(1, &shadowDepthTexture);
		shadowDepthTexture = 0;
	}
    
    if (shadowFramebuffer) {
        glDeleteFramebuffers(1, &shadowFramebuffer);
        shadowFramebuffer = 0;
    }
}

- (void)renderWithModelMatrix: (KRMat4)modelMatrix
{    
    /* An identity matrix we use to perform the equivalant of glLoadIdentity */
    KRMat4 identitymatrix;
    
    KRMat4 projectionmatrix; /* Our projection matrix starts with all 0s */
    // KRMat4 modelmatrix; /* Our model matrix  */
    KRMat4 mvpmatrix; /* Our MVP matrix  */
    
    
    /* Create our projection matrix with a 45 degree field of view
     * a width to height ratio of 1 and view from .1 to 800 infront of us */
    
    projectionmatrix.perspective(45.0f, 1.3333, 0.01f, 800.0f);
    
    // Replace the implementation of this method to do your own custom drawing
    static std::map<std::string, KRModel *>::iterator model_itr;
    
    model_itr = m_models.begin();
    
    
    
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
    
    

    
    // Use shader program
    glUseProgram(m_objectShaderProgram);
    
    // Sets the diffuseTexture variable to the first texture unit
	glUniform1i(glGetUniformLocation(m_objectShaderProgram, "diffuseTexture"), 0);
    
    // Sets the specularTexture variable to the second texture unit
    glUniform1i(glGetUniformLocation(m_objectShaderProgram, "specularTexture"), 1);
    
    // Sets the normalTexture variable to the third texture unit
    glUniform1i(glGetUniformLocation(m_objectShaderProgram, "normalTexture"), 2);
    
    // Validate program before drawing. This is a good check, but only really necessary in a debug build.
    // DEBUG macro must be defined in your debug configurations if that's not already the case.
#if defined(DEBUG)
    if (![self validateProgram:m_objectShaderProgram])
    {
        NSLog(@"Failed to validate program: %d", m_objectShaderProgram);
        return;
    }
#endif
    
    // ----- Render Model -----
    
    KRMat4 modelmatrix = identitymatrix;
    // Load the identity matrix into modelmatrix. rotate the model, and move it back 3
    KRModel *pModel = (*model_itr).second;
    modelmatrix.translate(pModel->getMinX() - pModel->getMaxX(), pModel->getMinY() - pModel->getMaxY(), pModel->getMinZ() - pModel->getMaxZ());
    modelmatrix.scale(1.0/pModel-> getMaxDimension());
    modelmatrix.translate(0.15, 0.1, -0.4);

    
    modelmatrix *= modelMatrix;
    
    mvpmatrix = modelmatrix;
    
    
    
    // multiply our modelmatrix and our projectionmatrix.
    mvpmatrix *= projectionmatrix;
    mvpmatrix.rotate(-90 * 0.0174532925199, Z_AXIS);
    
    
    // Bind our modelmatrix variable to be a uniform called mvpmatrix in our shaderprogram
    glUniformMatrix4fv(m_uniforms[KRENGINE_UNIFORM_MVP], 1, GL_FALSE, mvpmatrix.getPointer());
    glUniformMatrix4fv(m_uniforms[KRENGINE_UNIFORM_MODEL], 1, GL_FALSE, modelmatrix.getPointer());
    glUniformMatrix3fv(m_uniforms[KRENGINE_UNIFORM_MODELIT], 1, GL_FALSE, modelmatrix.getPointer());
    
    
    
    (*model_itr).second -> render(m_objectShaderProgram, (GLuint)KRENGINE_ATTRIB_VERTEX, (GLuint)KRENGINE_ATTRIB_NORMAL, (GLuint)KRENGINE_ATTRIB_TANGENT, (GLuint)KRENGINE_ATTRIB_TEXUV, m_pMaterialManager);
    
    // This application only creates a single color renderbuffer which is already bound at this point.
    // This call is redundant, but needed if dealing with multiple renderbuffers.
    /*
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    [context presentRenderbuffer:GL_RENDERBUFFER];
    */
    
    [self renderPost];
}

- (BOOL)compileShader:(GLuint *)shader type:(GLenum)type file:(NSString *)file
{
    GLint status;
    const GLchar *source;
    
    source = (GLchar *)[[NSString stringWithContentsOfFile:file encoding:NSUTF8StringEncoding error:nil] UTF8String];
    if (!source)
    {
        NSLog(@"Failed to load vertex shader");
        return FALSE;
    }
    
    *shader = glCreateShader(type);
    glShaderSource(*shader, 1, &source, NULL);
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
    if (status == 0)
    {
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


- (BOOL)loadVertexShader:(NSString *)vertexShaderName fragmentShader:(NSString *)fragmentShaderName forProgram:(GLuint *)programPointer;
{
    GLuint vertexShader, fragShader;
	
    NSString *vertShaderPathname, *fragShaderPathname;
    
    // Create shader program.
    *programPointer = glCreateProgram();
    
    // Create and compile vertex shader.
    vertShaderPathname = [[NSBundle mainBundle] pathForResource:vertexShaderName ofType:@"vsh"];
    if (![self compileShader:&vertexShader type:GL_VERTEX_SHADER file:vertShaderPathname])
    {
        NSLog(@"Failed to compile vertex shader");
        return FALSE;
    }
    
    // Create and compile fragment shader.
    fragShaderPathname = [[NSBundle mainBundle] pathForResource:fragmentShaderName ofType:@"fsh"];
    if (![self compileShader:&fragShader type:GL_FRAGMENT_SHADER file:fragShaderPathname])
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
    glBindAttribLocation(*programPointer, KRENGINE_ATTRIB_VERTEX, "position");
    glBindAttribLocation(*programPointer, KRENGINE_ATTRIB_TEXUV, "inputTextureCoordinate");
    
    
    glBindAttribLocation(*programPointer, KRENGINE_ATTRIB_VERTEX, "myVertex");
    glBindAttribLocation(*programPointer, KRENGINE_ATTRIB_NORMAL, "myNormal");
    glBindAttribLocation(*programPointer, KRENGINE_ATTRIB_TANGENT, "myTangent");
    glBindAttribLocation(*programPointer, KRENGINE_ATTRIB_TEXUV, "myUV");
    
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
    [self loadVertexShader:@"PostShader" fragmentShader:@"PostShader" forProgram:&m_postShaderProgram];
    [self loadVertexShader:@"ShadowShader" fragmentShader:@"ShadowShader" forProgram:&m_shadowShaderProgram];
    [self loadVertexShader:@"ObjectShader" fragmentShader:@"ObjectShader" forProgram:&m_objectShaderProgram];
        
    // Get uniform locations
    m_uniforms[KRENGINE_UNIFORM_MATERIAL_AMBIENT] = glGetUniformLocation(m_objectShaderProgram, "material_ambient");
    m_uniforms[KRENGINE_UNIFORM_MATERIAL_DIFFUSE] = glGetUniformLocation(m_objectShaderProgram, "material_diffuse");
    m_uniforms[KRENGINE_UNIFORM_MATERIAL_SPECULAR] = glGetUniformLocation(m_objectShaderProgram, "material_specular");
    m_uniforms[KRENGINE_UNIFORM_MVP] = glGetUniformLocation(m_objectShaderProgram, "myMVPMatrix");
    m_uniforms[KRENGINE_UNIFORM_MODEL] = glGetUniformLocation(m_objectShaderProgram, "myModelView");
    m_uniforms[KRENGINE_UNIFORM_MODELIT] = glGetUniformLocation(m_objectShaderProgram, "myModelViewIT");
    
    return TRUE;
}

- (BOOL)loadResource:(NSString *)path
{
    NSString *name = [[path lastPathComponent] stringByDeletingPathExtension];
    if([path hasSuffix: @".pack"]) {
        NSLog(@"object: %@", path);
        m_models[[name UTF8String]] = new KRModel([path UTF8String], m_pMaterialManager);
    } else if([path hasSuffix: @".pvr"]) {
        NSLog(@"texture: %@", path);
        m_pTextureManager->loadTexture([name UTF8String], [path UTF8String]);
    } else if([path hasSuffix: @".mtl"]) {
        NSLog(@"material: %@", path);
        m_pMaterialManager->loadFile([path UTF8String]);
    }
    
    return TRUE;
}

/*
- (BOOL)loadObjects
{
    NSString *documentsDirectory = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0];
    NSFileManager* fileManager = [NSFileManager defaultManager];
    
    for (NSString* fileName in [fileManager contentsOfDirectoryAtPath: documentsDirectory error:nil]) {
        if([fileName hasSuffix: @".pvr"]) {
            NSString* path = [NSString stringWithFormat:@"%@/%@", documentsDirectory, fileName];
            [self loadResource: path];
        }
    }
    
    for (NSString* fileName in [fileManager contentsOfDirectoryAtPath: documentsDirectory error:nil]) {
        if([fileName hasSuffix: @".mtl"]) {
            NSString* path = [NSString stringWithFormat:@"%@/%@", documentsDirectory, fileName];
            [self loadResource: path];
        }
    }
    
    for (NSString* fileName in [fileManager contentsOfDirectoryAtPath: documentsDirectory error:nil]) {
        if([fileName hasSuffix: @".pack"]) {
            NSString* path = [NSString stringWithFormat:@"%@/%@", documentsDirectory, fileName];
            [self loadResource: path];
        }
    }
    

    
    return TRUE;
}
*/

/*
 NSArray *paths = [[NSBundle mainBundle] pathsForResourcesOfType:@"pack" inDirectory:nil];
 for (NSString *path in paths) {
 [self loadResource: path];
 }
 
 NSArray *paths = [[NSBundle mainBundle] pathsForResourcesOfType:@"pvr" inDirectory:nil];
 for (NSString *path in paths) {
 [self loadResource: path];
 }
 
 paths = [[NSBundle mainBundle] pathsForResourcesOfType:@"mtl" inDirectory:nil];
 for (NSString *path in paths) {
 [self loadResource: path];
 }
 */

- (void)dealloc
{
    if (m_objectShaderProgram) {
        glDeleteProgram(m_objectShaderProgram);
        m_objectShaderProgram = 0;
    }
    
    for(std::map<std::string, KRModel *>::iterator itr=m_models.begin(); itr != m_models.end(); itr++) {
        delete (*itr).second;
    }
    m_models.empty();
    
    if(m_pTextureManager) {
        delete m_pTextureManager;
        m_pTextureManager = NULL;
    }
    
    if(m_pMaterialManager) {
        delete m_pMaterialManager;
        m_pMaterialManager = NULL;
    }
    
    
    [self destroyBuffers];
    [super dealloc];
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
	
	static const GLfloat textureVertices[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f,  1.0f,
        1.0f,  1.0f,
    };
    /*
     static const GLfloat textureVertices[] = {
     1.0f, 1.0f,
     1.0f, 0.0f,
     0.0f,  1.0f,
     0.0f,  0.0f,
     };
     */
	
	
    glDisable(GL_DEPTH_TEST);
	glUseProgram(m_postShaderProgram);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, compositeDepthTexture);
    glUniform1i(glGetUniformLocation(m_postShaderProgram, "depthFrame"), 0);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, compositeColorTexture);
    glUniform1i(glGetUniformLocation(m_postShaderProgram, "renderFrame"), 1);
    
    /*
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, videoFrameTexture);
    glUniform1i(glGetUniformLocation(m_postShaderProgram, "videoFrame"), 2);
	*/
	
	// Update attribute values.
	glVertexAttribPointer(KRENGINE_ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, squareVertices);
	glEnableVertexAttribArray(KRENGINE_ATTRIB_VERTEX);
	glVertexAttribPointer(KRENGINE_ATTRIB_TEXUV, 2, GL_FLOAT, 0, 0, textureVertices);
	glEnableVertexAttribArray(KRENGINE_ATTRIB_TEXUV);
	
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    /*
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, 0);
    */
}

@end
