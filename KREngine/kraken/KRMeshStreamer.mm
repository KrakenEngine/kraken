//
//  KRMeshStreamer.cpp
//  Kraken
//
//  Created by Kearwood Gilbert on 11/1/2013.
//  Copyright (c) 2013 Kearwood Software. All rights reserved.
//

#include "KRMeshStreamer.h"

#include "KREngine-common.h"
#include "KRContext.h"

#include <chrono>

#if TARGET_OS_IPHONE

EAGLContext *gMeshStreamerContext = nil;

#elif TARGET_OS_MAC

NSOpenGLContext *gMeshStreamerContext = nil;

#else

#error Unsupported Platform
#endif

KRMeshStreamer::KRMeshStreamer(KRContext &context) : m_context(context)
{
    
#if TARGET_OS_IPHONE
    gMeshStreamerContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2 sharegroup: [EAGLContext currentContext].sharegroup];
#elif TARGET_OS_MAC
    NSOpenGLPixelFormatAttribute pixelFormatAttributes[] =
    {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
        0
    };
    NSOpenGLPixelFormat *pixelFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes:pixelFormatAttributes] autorelease];
    gMeshStreamerContext = [[[NSOpenGLContext alloc] initWithFormat: pixelFormat shareContext: [NSOpenGLContext currentContext] ] autorelease];
#else
    #error Unsupported Platform
#endif
    
    m_stop = false;
    m_thread = std::thread(&KRMeshStreamer::run, this);
}

KRMeshStreamer::~KRMeshStreamer()
{
    m_stop = true;
    m_thread.join();
    
    [gMeshStreamerContext release];
}

void KRMeshStreamer::run()
{
    pthread_setname_np("Kraken - Mesh Streamer");
    
    std::chrono::microseconds sleep_duration( 100 );
    
#if TARGET_OS_IPHONE
    [EAGLContext setCurrentContext: gMeshStreamerContext];
#elif TARGET_OS_MAC
    [gMeshStreamerContext makeCurrentContext];
#else
    #error Unsupported Platform
#endif
    
    
    while(!m_stop)
    {
        if(m_context.getStreamingEnabled()) {
        
        }
        std::this_thread::sleep_for( sleep_duration );
    }
}
