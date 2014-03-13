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
    m_running = false;
    m_stop = false;
}

void KRMeshStreamer::startStreamer()
{
    if(!m_running) {
        m_running = true;
        
#if TARGET_OS_IPHONE
        // FIXME: need to add code check for iOS 7 and also this appears to cause crashing
        gMeshStreamerContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2 sharegroup: [EAGLContext currentContext].sharegroup];
        //gMeshStreamerContext.multiThreaded = TRUE;
#elif TARGET_OS_MAC
        NSOpenGLPixelFormatAttribute pixelFormatAttributes[] =
        {
//            NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersionLegacy,
            0
        };
        NSOpenGLPixelFormat *pixelFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes:pixelFormatAttributes] autorelease];
        gMeshStreamerContext = [[NSOpenGLContext alloc] initWithFormat: pixelFormat shareContext: [NSOpenGLContext currentContext] ];
#else
        #error Unsupported Platform
#endif
        
        m_thread = std::thread(&KRMeshStreamer::run, this);
    }
}

KRMeshStreamer::~KRMeshStreamer()
{
    if(m_running) {
        m_stop = true;
        m_thread.join();
        m_running = false;
    }
    
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
