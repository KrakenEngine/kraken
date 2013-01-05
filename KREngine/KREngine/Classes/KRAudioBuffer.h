//
//  KRAudioBuffer.h
//  KREngine
//
//  Created by Kearwood Gilbert on 2013-01-04.
//  Copyright (c) 2013 Kearwood Software. All rights reserved.
//

#ifndef KRAUDIO_BUFFER_H
#define KRAUDIO_BUFFER_H

#include "KREngine-common.h"
#include "KRDataBlock.h"

class KRAudioManager;
class KRAudioSample;

class KRAudioBuffer
{
public:
    KRAudioBuffer(KRAudioManager *manager, KRAudioSample *sound, int index, ALenum dataFormat, int frameCount, int frameRate, int bytesPerFrame, void (*fn_populate)(KRAudioSample *, int, void *));
    ~KRAudioBuffer();
    
    unsigned int getBufferID();
private:
    KRAudioManager *m_pSoundManager;
    
    
    ALenum m_dataFormat;
	int m_frameCount;
    int m_frameRate;
    int m_bytesPerFrame;
    KRDataBlock *m_pData;
    
    unsigned int m_bufferID;
};

#endif /* defined(KRAUDIO_BUFFER_H) */
