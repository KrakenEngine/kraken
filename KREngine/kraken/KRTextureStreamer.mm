//
//  KRTextureStreamer.cpp
//  Kraken
//
//  Created by Kearwood Gilbert on 11/1/2013.
//  Copyright (c) 2013 Kearwood Software. All rights reserved.
//

#include "KREngine-common.h"

#include "KRTextureStreamer.h"
#include "KRContext.h"

#include <chrono>


#if TARGET_OS_IPHONE

EAGLContext *gTextureStreamerContext = nil;

#elif TARGET_OS_MAC

NSOpenGLContext *gTextureStreamerContext = nil;

#else

#error Unsupported Platform
#endif

KRTextureStreamer::KRTextureStreamer(KRContext &context) : m_context(context)
{
    m_running = false;
    m_stop = false;
}

void KRTextureStreamer::startStreamer()
{
    if(!m_running) {
        m_running = true;
        
#if TARGET_OS_IPHONE
        
        gTextureStreamerContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2 sharegroup: [EAGLContext currentContext].sharegroup];
        gTextureStreamerContext.multiThreaded = TRUE;

        
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
        
        m_thread = std::thread(&KRTextureStreamer::run, this);
    }
}

KRTextureStreamer::~KRTextureStreamer()
{
    if(m_running) {
        m_stop = true;
        m_thread.join();
        m_running = false;
    }
    
    [gTextureStreamerContext release];
}

void KRTextureStreamer::run()
{
    pthread_setname_np("Kraken - Texture Streamer");
    
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
        if(m_context.getStreamingEnabled()) {
            m_context.getTextureManager()->doStreaming();
        }
        std::this_thread::sleep_for( sleep_duration );
    }
}
