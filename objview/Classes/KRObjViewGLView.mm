//
//  KRObjViewGLView.m
//  KRObjView
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

#import "KRObjViewGLView.h"


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
		_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
		
        if (!_context)
		{
			[self release];
			return nil;
		}
        
		if (![EAGLContext setCurrentContext:_context])
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
        _engine = [[KREngine alloc] initForWidth: backingWidth Height: backingHeight];
        [self loadObjects];
    }
    return self;
}

- (void)dealloc {
    [_engine release]; _engine = nil;
    [_context release]; _context = nil;
    
    [super dealloc];
}

#pragma mark -
#pragma mark OpenGL drawing

- (BOOL)loadObjects
{    
    NSString *documentsDirectory = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0];
    NSFileManager* fileManager = [NSFileManager defaultManager];
    
    
    for (NSString* fileName in [fileManager contentsOfDirectoryAtPath: documentsDirectory error:nil]) {
        NSString* path = [NSString stringWithFormat:@"%@/%@", documentsDirectory, fileName];
        [self.engine loadResource: path];
    }

    [self.engine setNearZ: 5.0];
    [self.engine setFarZ: 5000.0];
    //[renderEngine setNearZ: 1.0];
    //[renderEngine setFarZ: 3000.0];

    
    return TRUE;
}

- (BOOL)createFramebuffers
{	
	
	// ===== Create onscreen framebuffer object =====
	GLDEBUG(glGenFramebuffers(1, &viewFramebuffer));
    GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, viewFramebuffer));
    
    // ----- Create color buffer for viewFramebuffer -----
    GLDEBUG(glGenRenderbuffers(1, &viewRenderbuffer));
    GLDEBUG(glBindRenderbuffer(GL_RENDERBUFFER, viewRenderbuffer));
	[_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer*)self.layer];
    GLDEBUG(glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidth));
	GLDEBUG(glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight));
	NSLog(@"Backing width: %d, height: %d", backingWidth, backingHeight);
    GLDEBUG(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, viewRenderbuffer));
    
        
    
    GLDEBUG(
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            NSLog(@"Failure with depth buffer generation");
            return NO;
        }
    );
	
    GLDEBUG(
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            NSLog(@"Incomplete FBO: %d", status);
            exit(1);
        }
    );
	
	return TRUE;
}

- (void)destroyFramebuffer;
{	
	if (viewFramebuffer)
	{
		GLDEBUG(glDeleteFramebuffers(1, &viewFramebuffer));
		viewFramebuffer = 0;
	}
	
	if (viewRenderbuffer)
	{
		GLDEBUG(glDeleteRenderbuffers(1, &viewRenderbuffer));
		viewRenderbuffer = 0;
	}
}

- (void)setDisplayFramebuffer;
{
    if (_context)
    {
        if (!viewFramebuffer)
		{
            [self createFramebuffers];
		}
        
        //GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, viewFramebuffer));
        
        GLDEBUG(glViewport(0, 0, backingWidth, backingHeight));
    }
}

- (BOOL)presentFramebuffer;
{
    BOOL success = FALSE;
    
    if (_context)
    {
        //GLDEBUG(glBindRenderbuffer(GL_RENDERBUFFER, viewRenderbuffer));
        
        success = [_context presentRenderbuffer:GL_RENDERBUFFER];
    }
    
    return success;
}

- (KRScene *)getScene
{
    return self.engine.context->getSceneManager()->getFirstScene();
}

#pragma mark -
#pragma mark Accessors


@end
