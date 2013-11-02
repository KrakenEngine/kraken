//
//  KRTextureStreamer.cpp
//  Kraken
//
//  Created by Kearwood Gilbert on 11/1/2013.
//  Copyright (c) 2013 Kearwood Software. All rights reserved.
//

#include "KREngine-common.h"

#include "KRTextureStreamer.h"

#include <chrono>

EAGLContext *gTextureStreamerContext;

KRTextureStreamer::KRTextureStreamer(KRContext &context) : m_context(context)
{
    gTextureStreamerContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2 sharegroup: [EAGLContext currentContext].sharegroup];
    m_stop = false;
    m_thread = std::thread(&KRTextureStreamer::run, this);
}

KRTextureStreamer::~KRTextureStreamer()
{
    m_stop = true;
    m_thread.join();
    
    [gTextureStreamerContext release];
}

void KRTextureStreamer::run()
{
    std::chrono::microseconds sleep_duration( 100 );
    [EAGLContext setCurrentContext: gTextureStreamerContext];

    while(!m_stop)
    {
        std::this_thread::sleep_for( sleep_duration );
    }
}
