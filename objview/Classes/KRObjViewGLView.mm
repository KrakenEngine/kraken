//
//  KRObjViewGLView.m
//  KRObjView
//
//  Created by Mac on 11-05-01.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "KRObjViewGLView.h"

#import <OpenGLES/EAGLDrawable.h>
#import <QuartzCore/QuartzCore.h>

@implementation KRObjViewGLView

// Override the class method to return the OpenGL layer, as opposed to the normal CALayer
+ (Class) layerClass 
{
	return [CAEAGLLayer class];
}

- (id)initWithFrame:(CGRect)frame {
    
    self = [super initWithFrame:frame];
    if (self) {

		// Do OpenGL Core Animation layer setup
		CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
		
		// Set scaling to account for Retina display	
        if ([self respondsToSelector:@selector(setContentScaleFactor:)])
        {
            self.contentScaleFactor = [[UIScreen mainScreen] scale];
        } 
        
		eaglLayer.opaque = YES;
		eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];		
		context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
		
        if (!context) 
		{
			[self release];
			return nil;
		}
        
		if (![EAGLContext setCurrentContext:context]) 
		{
			[self release];
			return nil;
		}
        
        if (![self createFramebuffers]) 
		{
			[self release];
			return nil;
		}
        
        // Initialize KREngine
        renderEngine = [[KREngine alloc] initForWidth: backingWidth Height: backingHeight];
        [self loadObjects];
    }
    return self;
}

- (void)dealloc {
    if(renderEngine) {
        [renderEngine release];
        renderEngine = nil;
    }
    
    [super dealloc];
}

#pragma mark -
#pragma mark OpenGL drawing

- (BOOL)loadObjects
{
    NSString *documentsDirectory = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0];
    NSFileManager* fileManager = [NSFileManager defaultManager];
    
    for (NSString* fileName in [fileManager contentsOfDirectoryAtPath: documentsDirectory error:nil]) {
        if([fileName hasSuffix: @".pvr"]) {
            NSString* path = [NSString stringWithFormat:@"%@/%@", documentsDirectory, fileName];
            [renderEngine loadResource: path];
        }
    }
    
    for (NSString* fileName in [fileManager contentsOfDirectoryAtPath: documentsDirectory error:nil]) {
        if([fileName hasSuffix: @".mtl"]) {
            NSString* path = [NSString stringWithFormat:@"%@/%@", documentsDirectory, fileName];
            [renderEngine loadResource: path];
        }
    }
    
    for (NSString* fileName in [fileManager contentsOfDirectoryAtPath: documentsDirectory error:nil]) {
        if([fileName hasSuffix: @".krobject"]) {
            NSString* path = [NSString stringWithFormat:@"%@/%@", documentsDirectory, fileName];
            [renderEngine loadResource: path];
        }
    }
    
    KRModelManager *pModelManager = [renderEngine getModelManager];
    m_scene.addInstance(pModelManager->getModel("fachwerkhaus12"), KRMat4());
    
    [renderEngine setNearZ: 25.0];
    [renderEngine setFarZ: 5000.0];
    /*
     
     startPos 156.0 -55.0 -825.0
     touchScale 95.0
     nearZ 25.0
     farZ 5000.0
     
     */
    
    // [renderEngine setParameterValueWithName: @];
    return TRUE;
}

- (BOOL)createFramebuffers
{	
	
	// ===== Create onscreen framebuffer object =====
	glGenFramebuffers(1, &viewFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, viewFramebuffer);
    
    // ----- Create color buffer for viewFramebuffer -----
    glGenRenderbuffers(1, &viewRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, viewRenderbuffer);
	[context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer*)self.layer];
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidth);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight);
	NSLog(@"Backing width: %d, height: %d", backingWidth, backingHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, viewRenderbuffer);
    
        
    
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		NSLog(@"Failure with depth buffer generation");
		return NO;
	}
	
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
		NSLog(@"Incomplete FBO: %d", status);
        exit(1);
    }
     
	
	return TRUE;
}

- (void)destroyFramebuffer;
{	
	if (viewFramebuffer)
	{
		glDeleteFramebuffers(1, &viewFramebuffer);
		viewFramebuffer = 0;
	}
	
	if (viewRenderbuffer)
	{
		glDeleteRenderbuffers(1, &viewRenderbuffer);
		viewRenderbuffer = 0;
	}
}

- (void)setDisplayFramebuffer;
{
    if (context)
    {
        if (!viewFramebuffer)
		{
            [self createFramebuffers];
		}
        
        glBindFramebuffer(GL_FRAMEBUFFER, viewFramebuffer);
        
        glViewport(0, 0, backingWidth, backingHeight);
        
        /*
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        */
    }
}

- (BOOL)presentFramebuffer;
{
    BOOL success = FALSE;
    
    if (context)
    {
        glBindRenderbuffer(GL_RENDERBUFFER, viewRenderbuffer);
        
        success = [context presentRenderbuffer:GL_RENDERBUFFER];
    }
    
    return success;
}

- (KREngine *)getEngine;
{
    return renderEngine;
}

- (KRScene *)getScene;
{
    return &m_scene;
}

#pragma mark -
#pragma mark Accessors


@end
