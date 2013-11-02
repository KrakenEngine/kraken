//
//  KRTextureStreamer.cpp
//  Kraken
//
//  Created by Kearwood Gilbert on 11/1/2013.
//  Copyright (c) 2013 Kearwood Software. All rights reserved.
//

#include "KRTextureStreamer.h"

#include <chrono>

KRTextureStreamer::KRTextureStreamer(KRContext &context) : m_context(context)
{
    m_stop = false;
    m_thread = std::thread(&KRTextureStreamer::run, this);
}

KRTextureStreamer::~KRTextureStreamer()
{
    m_stop = true;
    m_thread.join();
}

void KRTextureStreamer::run()
{
    std::chrono::microseconds sleep_duration( 100 );

    while(!m_stop)
    {
        std::this_thread::sleep_for( sleep_duration );
    }
}
