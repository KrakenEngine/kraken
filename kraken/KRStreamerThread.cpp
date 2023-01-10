//
//  KRStreamerThread.cpp
//  Kraken Engine
//
//  Copyright 2023 Kearwood Gilbert. All rights reserved.
//  
//  Redistribution and use in source and binary forms, with or without modification, are
//  permitted provided that the following conditions are met:
//  
//  1. Redistributions of source code must retain the above copyright notice, this list of
//  conditions and the following disclaimer.
//  
//  2. Redistributions in binary form must reproduce the above copyright notice, this list
//  of conditions and the following disclaimer in the documentation and/or other materials
//  provided with the distribution.
//  
//  THIS SOFTWARE IS PROVIDED BY KEARWOOD GILBERT ''AS IS'' AND ANY EXPRESS OR IMPLIED
//  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL KEARWOOD GILBERT OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//  
//  The views and conclusions contained in the software and documentation are those of the
//  authors and should not be interpreted as representing official policies, either expressed
//  or implied, of Kearwood Gilbert.
//

#include "KREngine-common.h"

#include "KRStreamerThread.h"
#include "KRContext.h"

#include <chrono>


KRStreamerThread::KRStreamerThread(KRContext& context) : m_context(context)
{
  m_running = false;
  m_stop = false;
}

void KRStreamerThread::start()
{
  if (!m_running) {
    m_running = true;

    m_thread = std::thread(&KRStreamerThread::run, this);
  }
}

void KRStreamerThread::stop()
{
  if (m_running) {
    m_stop = true;
    m_thread.join();
    m_running = false;
  }
}

KRStreamerThread::~KRStreamerThread()
{
  stop();
}

void KRStreamerThread::run()
{

#if defined(ANDROID)
  // TODO - Set thread names on Android
#elif defined(_WIN32) || defined(_WIN64)
  // TODO - Set thread names on windows
#else
  pthread_setname_np("Kraken - Streamer");
#endif

  std::chrono::microseconds sleep_duration(15000);

  while (!m_stop) {
    m_context.doStreaming();
    std::this_thread::sleep_for(sleep_duration);
  }
}
