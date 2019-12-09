//
//  KRContext-osx.mm
//  Kraken
//
//  Created by Kearwood Gilbert on 11/1/2013.
//  Copyright (c) 2013 Kearwood Software. All rights reserved.
//

#include "KREngine-common.h"
#include <AppKit/AppKit.h>

#include "KRContext.h"

NSOpenGLContext *gStreamerContext = nil;
NSOpenGLContext *gRenderContext = nil;

void KRContext::destroyDeviceContexts()
{
  gStreamerContext = nil;
  gRenderContext = nil;
}

void createGLDeviceContexts()
{
    if(gRenderContext == nil) {
        
        /*
        NSOpenGLPixelFormatAttribute attribs[] =
        {
            NSOpenGLPFADoubleBuffer,
            NSOpenGLPFADepthSize, 32,
            NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
            0
        };
        */
        NSOpenGLPixelFormatAttribute attribs[] = {
            NSOpenGLPFADoubleBuffer,
            NSOpenGLPFAAccelerated,
            NSOpenGLPFAColorSize, 24,
            NSOpenGLPFAAlphaSize, 8,
            NSOpenGLPFADepthSize, 24,
            NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
            0
        };
        NSOpenGLPixelFormat *pixelFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attribs] autorelease];
        gRenderContext = [[NSOpenGLContext alloc] initWithFormat: pixelFormat shareContext: nil ];
        gStreamerContext = [[NSOpenGLContext alloc] initWithFormat: pixelFormat shareContext: gRenderContext ];
        
        // set synch to VBL to eliminate tearing
        GLint vblSynch = 1;
        [gRenderContext setValues:&vblSynch forParameter:NSOpenGLCPSwapInterval];
        
/*
        CGLEnable([gRenderContext CGLContextObj], kCGLCESurfaceBackingSize);
        
        const GLint dim[2] = {1920, 1080};
        
        [gRenderContext setValues: &dim[0] forParameter: NSOpenGLCPSurfaceBackingSize];
        [gRenderContext update];
*/
    }
}

void KRContext::activateStreamerContext()
{
    createGLDeviceContexts();
    [gStreamerContext makeCurrentContext];
}

void KRContext::activateRenderContext()
{
    createGLDeviceContexts();
    [gRenderContext update];
    [gRenderContext makeCurrentContext];
}

void KRContext::attachToView(void *view)
{
    createGLDeviceContexts();
    NSView *v = (NSView *)view;
    [gRenderContext setView: v];
    [gRenderContext update];
}
