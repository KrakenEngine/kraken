//
//  KRContext-ios.mm
//  Kraken
//
//  Created by Kearwood Gilbert on 11/1/2013.
//  Copyright (c) 2013 Kearwood Software. All rights reserved.
//

#include "KREngine-common.h"

#include "KRContext.h"

EAGLContext *gStreamerContext = nil;
EAGLContext *gRenderContext = nil;

void KRContext::destroyDeviceContexts()
{
    [gStreamerContext release];
    [gRenderContext release];
}

void KRContext::createDeviceContexts()
{
    gRenderContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    gStreamerContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2 sharegroup: gStreamerContext.sharegroup];
    
    // FIXME: need to add code check for iOS 7 and also this appears to cause crashing
    
    //gTextureStreamerContext.multiThreaded = TRUE;
}

void KRContext::activateStreamerContext()
{
    [EAGLContext setCurrentContext: gStreamerContext];
}

void KRContext::activateRenderContext()
{
    [EAGLContext setCurrentContext: gRenderContext];
}
