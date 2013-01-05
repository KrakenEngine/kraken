//
//  KRAudioBuffer.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2013-01-04.
//  Copyright (c) 2013 Kearwood Software. All rights reserved.
//

#include "KRAudioBuffer.h"
#include "KRAudioManager.h"


KRAudioBuffer::KRAudioBuffer(KRAudioManager *manager, KRAudioSample *sound, int index, ALenum dataFormat, int frameCount, int frameRate, int bytesPerFrame, void (*fn_populate)(KRAudioSample *, int, void *))
{
    m_bufferID = 0;
    m_pSoundManager = manager;
    m_dataFormat = dataFormat;
    m_frameCount = frameCount;
    m_frameRate = frameRate;
    m_bytesPerFrame = bytesPerFrame;
    m_pData = NULL;
    
    m_pData = m_pSoundManager->getBufferData(m_frameCount * m_bytesPerFrame);
    fn_populate(sound, index, m_pData->getStart());
    
    m_pSoundManager->makeCurrentContext();
    alGenBuffers(1, &m_bufferID);
    alBufferData(m_bufferID, m_dataFormat, m_pData->getStart(), m_frameCount * m_bytesPerFrame, m_frameRate);
}

KRAudioBuffer::~KRAudioBuffer()
{
    if(m_bufferID) {
        alDeleteBuffers(1, &m_bufferID);
        m_bufferID = 0;
    }
    
    m_pSoundManager->recycleBufferData(m_pData);
}


unsigned int KRAudioBuffer::getBufferID()
{
    return m_bufferID;
}
