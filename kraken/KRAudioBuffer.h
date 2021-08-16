//
//  KRAudioBuffer.h
//  Kraken Engine
//
//  Copyright 2021 Kearwood Gilbert. All rights reserved.
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
