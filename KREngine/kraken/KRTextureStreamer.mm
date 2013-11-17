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

EAGLContext *gTextureStreamerContext = nil;

KRTextureStreamer::KRTextureStreamer(KRContext &context) : m_context(context)
{
    m_running = false;
    m_stop = false;
}

void KRTextureStreamer::startStreamer()
{
    if(!m_running) {
        m_running = true;
        gTextureStreamerContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2 sharegroup: [EAGLContext currentContext].sharegroup];
        m_thread = std::thread(&KRTextureStreamer::run, this);
    }
}

KRTextureStreamer::~KRTextureStreamer()
{
    m_stop = true;
    m_thread.join();
    
    [gTextureStreamerContext release];
}

void KRTextureStreamer::run()
{
    pthread_setname_np("Kraken - Texture Streamer");
    
    std::chrono::microseconds sleep_duration( 100 );
    [EAGLContext setCurrentContext: gTextureStreamerContext];

    while(!m_stop)
    {
        if(m_context.getStreamingEnabled()) {
            m_context.getTextureManager()->doStreaming();
        }
        std::this_thread::sleep_for( sleep_duration );
    }
    
    m_running = false;
}
