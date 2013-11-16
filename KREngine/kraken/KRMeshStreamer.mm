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

EAGLContext *gMeshStreamerContext;

KRMeshStreamer::KRMeshStreamer(KRContext &context) : m_context(context)
{
    gMeshStreamerContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2 sharegroup: [EAGLContext currentContext].sharegroup];
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
    [EAGLContext setCurrentContext: gMeshStreamerContext];
    
    while(!m_stop)
    {
        if(m_context.getStreamingEnabled()) {
        
        }
        std::this_thread::sleep_for( sleep_duration );
    }
}
