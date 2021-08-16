//
//  KRContext_osx.mm
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
