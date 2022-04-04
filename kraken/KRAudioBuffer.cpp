//
//  KRAudioBuffer.cpp
//  Kraken Engine
//
//  Copyright 2022 Kearwood Gilbert. All rights reserved.
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

#include "KRAudioBuffer.h"
#include "KRAudioManager.h"


KRAudioBuffer::KRAudioBuffer(KRAudioManager *manager, KRAudioSample *sound, int index, int frameCount, int frameRate, int bytesPerFrame, void (*fn_populate)(KRAudioSample *, int, void *))
{
    m_pSoundManager = manager;
    m_frameCount = frameCount;
    m_frameRate = frameRate;
    m_bytesPerFrame = bytesPerFrame;
    m_pData = NULL;
    m_audioSample = sound;
    m_index = index;
    
    m_pSoundManager->makeCurrentContext();
    m_pData = m_pSoundManager->getBufferData(m_frameCount * m_bytesPerFrame);
    fn_populate(sound, index, m_pData->getStart());
}

KRAudioBuffer::~KRAudioBuffer()
{   
    m_pSoundManager->recycleBufferData(m_pData);
}

KRAudioSample *KRAudioBuffer::getAudioSample()
{
    return m_audioSample;
}

int KRAudioBuffer::getFrameCount()
{
    return m_frameCount;
}

int KRAudioBuffer::getFrameRate()
{
    return m_frameRate;
}

signed short *KRAudioBuffer::getFrameData()
{
    return (signed short *)m_pData->getStart();
}

int KRAudioBuffer::getIndex()
{
    return m_index;
}