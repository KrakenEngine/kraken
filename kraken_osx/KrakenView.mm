//
//  KrakenView.mm
//  Kraken Engine
//
//  Copyright 2021 Kearwood Gilbert. All rights reserved.
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

#import "KrakenView.h"
#include "KREngine-common.h"

#include "KREngine.h"

#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>

@interface KrakenView() {
    CVDisplayLinkRef displayLink;
};

- (void)getFrameForTime:(const CVTimeStamp *)outputTime;
- (void)drawFrameWithDeltaTime: (float)deltaTime;

@end

@implementation KrakenView

- (void)dealloc
{
    CVDisplayLinkRelease(displayLink);
}


- (void)drawFrameWithDeltaTime: (float)deltaTime
{
    
    if(_camera) {
        KRContext::activateRenderContext();
        NSOpenGLContext *currentContext = [NSOpenGLContext currentContext];
        
        // must lock GL context because display link is threaded
        CGLLockContext((CGLContextObj)[currentContext CGLContextObj]);
        
        GLint hasDrawable = 0;
        [currentContext getValues: &hasDrawable forParameter:NSOpenGLCPHasDrawable];
        
        if(!hasDrawable) {
            KRContext::attachToView(self);
            const GLint dim[2] = {1920, 1080};
            
            [currentContext setValues: &dim[0] forParameter: NSOpenGLCPSurfaceBackingSize];
            [currentContext update];
        }
        
        GLint backingDim[2] = {0, 0};
        [currentContext getValues: &backingDim[0] forParameter:NSOpenGLCPSurfaceBackingSize];
        
        GLint rendererID = 0;
        [currentContext getValues: &rendererID forParameter:NSOpenGLCPCurrentRendererID];
        
        
        GLint renderBufferWidth = 0, renderBufferHeight = 0;
//        GLDEBUG(glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &renderBufferWidth));
//        GLDEBUG(glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &renderBufferHeight));
        
        renderBufferWidth = 1920;
        renderBufferHeight = 1080;
        
        glViewport(0, 0, renderBufferWidth, renderBufferHeight);
        GLDEBUG(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
        GLDEBUG(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        
        [[KREngine sharedInstance] renderScene: &_camera->getScene() WithDeltaTime:deltaTime AndWidth:renderBufferWidth AndHeight:renderBufferHeight];
        //[[KREngine sharedInstance] renderScene: &_camera->getScene() WithDeltaTime:deltaTime];
        
        [currentContext flushBuffer];
        
        CGLUnlockContext((CGLContextObj)[currentContext CGLContextObj]);
    }
}

- (void)viewDidMoveToSuperview
{
//    KRContext::attachToView(self);
    
    // set up the display link
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    CVDisplayLinkSetOutputCallback(displayLink, MyDisplayLinkCallback, self);
    /*
     CGLContextObj cglContext = (CGLContextObj)[context CGLContextObj];
     CGLPixelFormatObj cglPixelFormat = (CGLPixelFormatObj)[[context.pixelFormat] CGLPixelFormatObj];
     CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
     */
    
    // ----====---- activate the display link ----====----
    CVDisplayLinkStart(displayLink);
}

static CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp *now,
                                      const CVTimeStamp *outputTime, CVOptionFlags flagsIn,
                                      CVOptionFlags *flagsOut, void *displayLinkContext)
{
    // go back to Obj-C for easy access to instance variables
    [(KrakenView *)displayLinkContext getFrameForTime:outputTime];
    return kCVReturnSuccess;
}

- (void)getFrameForTime:(const CVTimeStamp *)outputTime
{
    // deltaTime is unused in this bare bones demo, but here's how to calculate it using display link info
    float deltaTime = 1.0 / (outputTime->rateScalar * (double)outputTime->videoTimeScale / (double)outputTime->videoRefreshPeriod);
    
    [self drawFrameWithDeltaTime: deltaTime];
}

/*
 - (id)initWithCoder:(NSCoder *)aDecoder
 {
 self = [super initWithCoder:aDecoder];
 if (self == nil)
 {
 NSLog(@"Unable to create a windowed OpenGL context.");
 exit(0);
 }
 
 [self commonInit];
 
 return self;
 
 }
 
 - (id)initWithFrame:(NSRect)frameRect
 {
 // context setup
 NSOpenGLPixelFormat        *windowedPixelFormat;
 NSOpenGLPixelFormatAttribute    attribs[] = {
 NSOpenGLPFAWindow,
 NSOpenGLPFAColorSize, 32,
 NSOpenGLPFADepthSize, 24,
 NSOpenGLPFAAccelerated,
 NSOpenGLPFADoubleBuffer,
 NSOpenGLPFASingleRenderer,
 0 };
 
 windowedPixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
 if (windowedPixelFormat == nil)
 {
 NSLog(@"Unable to create windowed pixel format.");
 exit(0);
 }
 self = [super initWithFrame:frameRect pixelFormat:windowedPixelFormat];
 if (self == nil)
 {
 NSLog(@"Unable to create a windowed OpenGL context.");
 exit(0);
 }
 [windowedPixelFormat release];
 
 [self commonInit];
 
 return self;
 }
 
 - (void)reshape
 {
 NSSize    viewBounds = [self bounds].size;
 viewWidth = viewBounds.width;
 viewHeight = viewBounds.height;
 
 NSOpenGLContext    *currentContext = [self openGLContext];
 [currentContext makeCurrentContext];
 
 // remember to lock the context before we touch it since display link is threaded
 CGLLockContext((CGLContextObj)[currentContext CGLContextObj]);
 
 // let the context know we've changed size
 [[self openGLContext] update];
 
 CGLUnlockContext((CGLContextObj)[currentContext CGLContextObj]);
 }
 */

@end
