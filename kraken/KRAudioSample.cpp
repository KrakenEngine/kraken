//
//  KRAudioSample.cpp
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

#include "KRAudioSample.h"
#include "KRAudioManager.h"
#include "KRDataBlock.h"
#include "KRAudioBuffer.h"
#include "KRContext.h"
#include "KRDSP.h"

KRAudioSample::KRAudioSample(KRContext& context, std::string name, std::string extension) : KRResource(context, name)
{
  m_pData = new KRDataBlock();
  m_extension = extension;

#ifdef __APPLE__
  // Apple Audio Toolbox
  m_audio_file_id = 0;
  m_fileRef = NULL;
#endif
  m_totalFrames = 0;
  m_bytesPerFrame = 0;
  m_frameRate = 0;
  m_bufferCount = 0;

  m_last_frame_used = 0;
}

KRAudioSample::KRAudioSample(KRContext& context, std::string name, std::string extension, KRDataBlock* data) : KRResource(context, name)
{
  m_pData = data;
  m_extension = extension;

#ifdef __APPLE__
  // Apple Audio Toolbox
  m_audio_file_id = 0;
  m_fileRef = NULL;
#endif
  m_totalFrames = 0;
  m_bytesPerFrame = 0;
  m_frameRate = 0;
  m_bufferCount = 0;

  m_last_frame_used = 0;
}

KRAudioSample::~KRAudioSample()
{
  closeFile();
  delete m_pData;
}

int KRAudioSample::getChannelCount()
{
  loadInfo();
  return m_channelsPerFrame;
}

__int64_t KRAudioSample::getFrameCount()
{
  loadInfo();
  //return (int)((__int64_t)m_totalFrames * (__int64_t)frame_rate / (__int64_t)m_frameRate);
  return m_totalFrames;
}

float KRAudioSample::sample(int frame_offset, int frame_rate, int channel)
{
  loadInfo();

  int c = KRMIN(channel, m_channelsPerFrame - 1);

  if (frame_offset < 0) {
    return 0.0f; // Past the beginning of the recording
  } else {
    int sample_frame;
    if (m_frameRate == frame_rate) {
      // No resampling required
      sample_frame = frame_offset;
    } else {
      // Need to resample from m_frameRate to frame_rate
      sample_frame = (int)((__int64_t)frame_offset * (__int64_t)m_frameRate / (__int64_t)frame_rate);
    }
    int maxFramesPerBuffer = KRENGINE_AUDIO_MAX_BUFFER_SIZE / m_bytesPerFrame;
    int buffer_index = sample_frame / maxFramesPerBuffer;
    if (buffer_index >= m_bufferCount) {
      return 0.0f; // Past the end of the recording
    } else {
      __int64_t buffer_offset = frame_offset - buffer_index * maxFramesPerBuffer;

      KRAudioBuffer* buffer = getContext().getAudioManager()->getBuffer(*this, buffer_index);
      if (buffer == NULL) {
        return 0.0f;
      } else if (buffer_offset >= buffer->getFrameCount()) {
        return 0.0f; // past the end of the recording
      } else {
        short* frame = buffer->getFrameData() + (buffer_offset * m_channelsPerFrame);
        return frame[c] / 32767.0f;
      }
    }
  }
}

void KRAudioSample::sample(__int64_t frame_offset, int frame_count, int channel, float* buffer, float amplitude, bool loop)
{
  loadInfo();

  m_last_frame_used = (int)getContext().getAudioManager()->getAudioFrame();

  if (loop) {
    int buffer_offset = 0;
    int frames_left = frame_count;
    int sample_length = (int)getFrameCount();
    while (frames_left) {
      int next_frame = (int)(((__int64_t)frame_offset + (__int64_t)buffer_offset) % sample_length);
      if (next_frame + frames_left >= sample_length) {
        int frames_processed = sample_length - next_frame;
        sample(next_frame, frames_processed, channel, buffer + buffer_offset, amplitude, false);
        frames_left -= frames_processed;
        buffer_offset += frames_processed;
      } else {
        sample(next_frame, frames_left, channel, buffer + buffer_offset, amplitude, false);
        frames_left = 0;
      }
    }
  } else {
    int c = KRMIN(channel, m_channelsPerFrame - 1);

    if (frame_offset + frame_count <= 0) {
      // Range is entirely before the sample
      memset(buffer, 0, frame_count * sizeof(float));
    } else if (frame_offset >= m_totalFrames) {
      // Range is entirely after the sample
      memset(buffer, 0, frame_count * sizeof(float));
    } else {
      int start_frame = (int)(frame_offset < 0 ? 0 : frame_offset);
      int prefix_frames = (int)(frame_offset < 0 ? -frame_offset : 0);
      if (prefix_frames > 0) {
        // Prefix with padding of 0's
        memset(buffer, 0, prefix_frames * sizeof(float));
      }

      int frames_per_buffer = KRENGINE_AUDIO_MAX_BUFFER_SIZE / m_bytesPerFrame;

      int buffer_index = start_frame / frames_per_buffer;
      int buffer_offset = start_frame % frames_per_buffer;
      int processed_frames = prefix_frames;
      while (processed_frames < frame_count) {
        int frames_left = frame_count - processed_frames;
        if (buffer_index >= m_bufferCount) {
          // Suffix with padding of 0's
          memset(buffer + processed_frames, 0, frames_left * sizeof(float));
          processed_frames += frames_left;
        } else {
          KRAudioBuffer* source_buffer = getContext().getAudioManager()->getBuffer(*this, buffer_index);
          int frames_to_copy = source_buffer->getFrameCount() - buffer_offset;
          if (frames_to_copy > frames_left) frames_to_copy = frames_left;
          if (frames_to_copy > 0) {
            signed short* source_data = source_buffer->getFrameData() + buffer_offset * m_channelsPerFrame + c;
            KRDSP::Int16ToFloat(source_data, m_channelsPerFrame, buffer + processed_frames, 1, frames_to_copy);
            //memcpy(buffer + processed_frames, source_buffer->getFrameData() + buffer_offset, frames_to_copy * m_channelsPerFrame * sizeof(float));
            processed_frames += frames_to_copy;
          }
          buffer_index++;
          buffer_offset = 0;
        }
      }
    }

    float scale = amplitude / 32768.0f;
    KRDSP::Scale(buffer, scale, frame_count);
  }
}

#ifdef __APPLE__
// Apple Audio Toolbox
OSStatus KRAudioSample::ReadProc( // AudioFile_ReadProc
                                       void* inClientData,
                                       SInt64		inPosition,
                                       UInt32	requestCount,
                                       void* buffer,
                                       UInt32* actualCount)
{
  KRAudioSample* sound = (KRAudioSample*)inClientData;
  UInt32 max_count = sound->m_pData->getSize() - inPosition;
  *actualCount = requestCount < max_count ? requestCount : max_count;
  sound->m_pData->copy(buffer, inPosition, *actualCount);
  return noErr;
}

SInt64 KRAudioSample::GetSizeProc( // AudioFile_GetSizeProc
                                        void* inClientData)
{
  KRAudioSample* sound = (KRAudioSample*)inClientData;
  return sound->m_pData->getSize();
}

OSStatus KRAudioSample::SetSizeProc( // AudioFile_SetSizeProc
                            void* inClientData,
                            SInt64		inSize)
{
  return -1; // Writing not supported
}

OSStatus KRAudioSample::WriteProc( // AudioFile_WriteProc
                            void* inClientData,
                            SInt64		inPosition,
                            UInt32		requestCount,
                            const void* buffer,
                            UInt32* actualCount)
{
  return -1; // Writing not supported
}
#endif // Apple Audio Toolbox

void KRAudioSample::openFile()
{
#ifdef __APPLE__
  // Apple Audio Toolbox

      //    AudioFileInitializeWithCallbacks
  if (m_fileRef == NULL) {

    //        printf("Call to KRAudioSample::openFile() with extension: %s\n", m_extension.c_str());
    // The m_extension is valid (it's either wav or mp3 for the files in Circa project)
    // so we can key off the extension and use a different data handler for mp3 files if we want to
    //

            // Temp variables
    UInt32 propertySize;

    // ---- Open audio file ----
    assert(AudioFileOpenWithCallbacks((void*)this, ReadProc, WriteProc, GetSizeProc, SetSizeProc, 0, &m_audio_file_id) == noErr);
    assert(ExtAudioFileWrapAudioFileID(m_audio_file_id, false, &m_fileRef) == noErr);

    // ---- Get file format information ----
    AudioStreamBasicDescription inputFormat;
    propertySize = sizeof(inputFormat);
    ExtAudioFileGetProperty(m_fileRef, kExtAudioFileProperty_FileDataFormat, &propertySize, &inputFormat);

    // ---- Set up output format ----
    AudioStreamBasicDescription outputFormat;
    // Set the client format to 16 bit signed integer (native-endian) data
    // Maintain the channel count and sample rate of the original source format
    outputFormat.mSampleRate = inputFormat.mSampleRate;
    outputFormat.mChannelsPerFrame = inputFormat.mChannelsPerFrame;
    outputFormat.mFormatID = kAudioFormatLinearPCM;
    outputFormat.mBytesPerPacket = 2 * outputFormat.mChannelsPerFrame;
    outputFormat.mFramesPerPacket = 1;
    outputFormat.mBytesPerFrame = 2 * outputFormat.mChannelsPerFrame;
    outputFormat.mBitsPerChannel = 16;
    outputFormat.mFormatFlags = kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked | kAudioFormatFlagIsSignedInteger;
    ExtAudioFileSetProperty(m_fileRef, kExtAudioFileProperty_ClientDataFormat, sizeof(outputFormat), &outputFormat);

    // ---- Get the buffer size and format parameters ----
    propertySize = sizeof(m_totalFrames);
    ExtAudioFileGetProperty(m_fileRef, kExtAudioFileProperty_FileLengthFrames, &propertySize, &m_totalFrames);

    m_bytesPerFrame = outputFormat.mBytesPerFrame;
    m_frameRate = outputFormat.mSampleRate;

    int maxFramesPerBuffer = KRENGINE_AUDIO_MAX_BUFFER_SIZE / m_bytesPerFrame;
    m_bufferCount = (m_totalFrames + maxFramesPerBuffer - 1) / maxFramesPerBuffer; // CEIL(_totalFrames / maxFramesPerBuffer)

    m_channelsPerFrame = outputFormat.mChannelsPerFrame;

    getContext().getAudioManager()->_registerOpenAudioSample(this);
  }
#else
#pragma message ( "TODO - implement for Windows" )
#endif
}

void KRAudioSample::closeFile()
{
#ifdef __APPLE__
  // Apple Audio Toolbox
  if (m_fileRef) {
    ExtAudioFileDispose(m_fileRef);
    m_fileRef = NULL;
  }

  if (m_audio_file_id) {
    AudioFileClose(m_audio_file_id);
    m_audio_file_id = 0;
  }
#endif

  getContext().getAudioManager()->_registerCloseAudioSample(this);
}

void KRAudioSample::loadInfo()
{
  if (m_frameRate == 0) {
    openFile();
    closeFile();
  }
}

std::string KRAudioSample::getExtension()
{
  return m_extension;
}

bool KRAudioSample::save(KRDataBlock& data)
{
  data.append(*m_pData);
  return true;
}

float KRAudioSample::getDuration()
{
  loadInfo();
  return (float)m_totalFrames / (float)m_frameRate;
}

int KRAudioSample::getBufferCount()
{
  loadInfo();
  return m_bufferCount;
}

void KRAudioSample::PopulateBuffer(KRAudioSample* sound, int index, void* data)
{
  int maxFramesPerBuffer = KRENGINE_AUDIO_MAX_BUFFER_SIZE / sound->m_bytesPerFrame;
  int startFrame = index * maxFramesPerBuffer;
  __uint32_t frameCount = (__uint32_t)KRMIN(sound->m_totalFrames - startFrame, maxFramesPerBuffer);

#ifdef __APPLE__
  // Apple Audio Toolbox
  AudioBufferList outputBufferInfo;
  outputBufferInfo.mNumberBuffers = 1;
  outputBufferInfo.mBuffers[0].mDataByteSize = frameCount * sound->m_bytesPerFrame;
  outputBufferInfo.mBuffers[0].mNumberChannels = sound->m_channelsPerFrame;
  outputBufferInfo.mBuffers[0].mData = data;

  // Read the data into an AudioBufferList
  ExtAudioFileSeek(sound->m_fileRef, startFrame);
  ExtAudioFileRead(sound->m_fileRef, (UInt32*)&frameCount, &outputBufferInfo);
#endif
}

KRAudioBuffer* KRAudioSample::getBuffer(int index)
{
  openFile();

  int maxFramesPerBuffer = KRENGINE_AUDIO_MAX_BUFFER_SIZE / m_bytesPerFrame;
  int startFrame = index * maxFramesPerBuffer;
  __uint32_t frameCount = (__uint32_t)KRMIN(m_totalFrames - startFrame, maxFramesPerBuffer);

  KRAudioBuffer* buffer = new KRAudioBuffer(getContext().getAudioManager(), this, index, frameCount, m_frameRate, m_bytesPerFrame, PopulateBuffer);

  if (m_bufferCount == 1) {
    //        [self closeFile]; // We don't need to hold on to a file handle if not streaming
  }
  return buffer;
}

void KRAudioSample::_endFrame()
{
  const __int64_t AUDIO_SAMPLE_EXPIRY_FRAMES = 500;
  __int64_t current_frame = getContext().getAudioManager()->getAudioFrame();
  if (current_frame > m_last_frame_used + AUDIO_SAMPLE_EXPIRY_FRAMES) {
    closeFile();
  }
}
