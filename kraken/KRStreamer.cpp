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


KRStreamer::KRStreamer(KRContext &context) : m_context(context)
{
    m_running = false;
    m_stop = false;
}

void KRStreamer::startStreamer()
{
    if(!m_running) {
        m_running = true;
        KRContext::activateStreamerContext();
        
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
}

void KRStreamer::run()
{

#if defined(ANDROID)
    // TODO - Set thread names on Android
#elif defined(_WIN32) || defined(_WIN64)
    // TODO - Set thread names on windows
#else
   pthread_setname_np("Kraken - Streamer");
#endif

    std::chrono::microseconds sleep_duration( 15000 );
    
    KRContext::activateStreamerContext();

    while(!m_stop)
    {
        m_context.doStreaming();
        std::this_thread::sleep_for( sleep_duration );
    }
}
