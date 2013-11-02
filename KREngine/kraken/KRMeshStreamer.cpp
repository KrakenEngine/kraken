//
//  KRMeshStreamer.cpp
//  Kraken
//
//  Created by Kearwood Gilbert on 11/1/2013.
//  Copyright (c) 2013 Kearwood Software. All rights reserved.
//

#include "KRMeshStreamer.h"

#include <chrono>

KRMeshStreamer::KRMeshStreamer(KRContext &context) : m_context(context)
{
    m_stop = false;
    m_thread = std::thread(&KRMeshStreamer::run, this);
}

KRMeshStreamer::~KRMeshStreamer()
{
    m_stop = true;
    m_thread.join();
}

void KRMeshStreamer::run()
{
    std::chrono::microseconds sleep_duration( 100 );
    
    while(!m_stop)
    {
        std::this_thread::sleep_for( sleep_duration );
    }
}
