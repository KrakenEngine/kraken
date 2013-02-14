//
//  KRAudioSample.h
//  KREngine
//
//  Copyright 2012 Kearwood Gilbert. All rights reserved.
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

#ifndef KRAUDIOSAMPLE_H
#define KRAUDIOSAMPLE_H

#include "KREngine-common.h"
#include "KRContextObject.h"
#include "KRDataBlock.h"
#include "KRResource.h"

class KRAudioBuffer;

class KRAudioSample : public KRResource {
    
public:
    KRAudioSample(KRContext &context, std::string name, std::string extension);
    KRAudioSample(KRContext &context, std::string name, std::string extension, KRDataBlock *data);
    virtual ~KRAudioSample();
    
    virtual std::string getExtension();
    
    virtual bool save(KRDataBlock &data);
    
    float getDuration();
    KRAudioBuffer *getBuffer(int index);
    int getBufferCount();
    
    // Siren audio engine interface
    int getChannelCount();
    int getFrameCount(int frame_rate);
    float sample(int frame_offset, int frame_rate, int channel);
    void sample(int frame_offset, int frame_rate, int frame_count, int channel, float *buffer);
    
private:
    
    std::string m_extension;
    KRDataBlock *m_pData;
    
    AudioFileID m_audio_file_id;
    ExtAudioFileRef m_fileRef;
    
    int m_bufferCount;
    
    SInt64 m_totalFrames;
    int m_frameRate;
    int m_bytesPerFrame;
    int m_channelsPerFrame;
    ALenum m_dataFormat;
    
    void openFile();
    void closeFile();
    
    static OSStatus ReadProc( // AudioFile_ReadProc
        void *		inClientData,
        SInt64		inPosition,
        UInt32	requestCount,
        void *		buffer,
        UInt32 *	actualCount);
    
    static OSStatus WriteProc( // AudioFile_WriteProc
        void * 		inClientData,
        SInt64		inPosition,
        UInt32		requestCount,
        const void *buffer,
        UInt32    * actualCount);
    
    static SInt64 GetSizeProc( // AudioFile_GetSizeProc
        void * 		inClientData);
    
    
    static OSStatus SetSizeProc( // AudioFile_SetSizeProc
        void *		inClientData,
        SInt64		inSize);
    
    
    static void PopulateBuffer(KRAudioSample *sound, int index, void *data);
};

#endif /* defined(KRAUDIOSAMPLE_H) */
