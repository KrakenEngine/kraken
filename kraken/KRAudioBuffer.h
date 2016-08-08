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
    KRAudioBuffer(KRAudioManager *manager, KRAudioSample *sound, int index, int frameCount, int frameRate, int bytesPerFrame, void (*fn_populate)(KRAudioSample *, int, void *));
    ~KRAudioBuffer();
    
    int getFrameCount();
    int getFrameRate();
    signed short *getFrameData();
    
    KRAudioSample *getAudioSample();
    int getIndex();
private:
    KRAudioManager *m_pSoundManager;
    
    int m_index;
	  int m_frameCount;
    int m_frameRate;
    int m_bytesPerFrame;
    KRDataBlock *m_pData;
    
    KRAudioSample *m_audioSample;
};

#endif /* defined(KRAUDIO_BUFFER_H) */
