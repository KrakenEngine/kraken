//
//  KRStreamer.cpp
//  Kraken
//
//  Created by Kearwood Gilbert on 11/1/2013.
//  Copyright (c) 2013 Kearwood Software. All rights reserved.
//

#include "KREngine-common.h"

#include "KRStreamer.h"
#include "KRContext.h"

#include <chrono>


#if TARGET_OS_IPHONE

EAGLContext *gTextureStreamerContext = nil;

#elif TARGET_OS_MAC

NSOpenGLContext *gTextureStreamerContext = nil;

#else

#error Unsupported Platform
#endif

KRStreamer::KRStreamer(KRContext &context) : m_context(context)
{
    m_running = false;
    m_stop = false;
}

void KRStreamer::startStreamer()
{
    if(!m_running) {
        m_running = true;
        
#if TARGET_OS_IPHONE
        
        gTextureStreamerContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2 sharegroup: [EAGLContext currentContext].sharegroup];
        // FIXME: need to add code check for iOS 7 and also this appears to cause crashing

        //gTextureStreamerContext.multiThreaded = TRUE;

        
#elif TARGET_OS_MAC
        
        NSOpenGLPixelFormatAttribute pixelFormatAttributes[] =
        {
//            NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersionLegacy,
            0
        };
        NSOpenGLPixelFormat *pixelFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes:pixelFormatAttributes] autorelease];
        gTextureStreamerContext = [[NSOpenGLContext alloc] initWithFormat: pixelFormat shareContext: [NSOpenGLContext currentContext] ];
        
#else
        
    #error Unsupported Platform
#endif
        
        m_thread = std::thread(&KRStreamer::run, this);
    }
}

KRStreamer::~KRStreamer()
{
    if(m_running) {
        m_stop = true;
        m_thread.join();
        m_running = false;
    }
    
    [gTextureStreamerContext release];
}

void KRStreamer::run()
{
    pthread_setname_np("Kraken - Streamer");

    std::chrono::microseconds sleep_duration( 100 );
    
#if TARGET_OS_IPHONE
    [EAGLContext setCurrentContext: gTextureStreamerContext];
#elif TARGET_OS_MAC
    [gTextureStreamerContext makeCurrentContext];
#else
#error Unsupported Platform
#endif

    while(!m_stop)
    {
        m_context.doStreaming();
        std::this_thread::sleep_for( sleep_duration );
    }
}
