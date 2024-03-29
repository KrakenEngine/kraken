//
//  KRAnimationCurve.cpp
//  Kraken Engine
//
//  Copyright 2024 Kearwood Gilbert. All rights reserved.
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

#include "KRContext.h"
#include "KRAnimationCurve.h"
#include "block.h"

using namespace mimir;

KRAnimationCurve::KRAnimationCurve(KRContext& context, const std::string& name) : KRResource(context, name)
{
  m_pData = new Block();
  m_pData->expand(sizeof(animation_curve_header));
  m_pData->lock();
  animation_curve_header* header = (animation_curve_header*)m_pData->getStart();
  strcpy(header->szTag, "KRCURVE1.0     ");
  header->frame_rate = 30.0f;
  header->frame_start = 0;
  header->frame_count = 0;
  m_pData->unlock();
}

KRAnimationCurve::~KRAnimationCurve()
{
  m_pData->unload();
  delete m_pData;
}
bool KRAnimationCurve::load(Block* data)
{
  m_pData->unload();
  delete m_pData;
  m_pData = data;
  return true;
}

std::string KRAnimationCurve::getExtension()
{
  return "kranimationcurve";
}

bool KRAnimationCurve::save(const std::string& path)
{
  return m_pData->save(path);
}

bool KRAnimationCurve::save(Block& data)
{
  data.append(*m_pData);
  return true;
}

KRAnimationCurve* KRAnimationCurve::Load(KRContext& context, const std::string& name, Block* data)
{
  KRAnimationCurve* new_animation_curve = new KRAnimationCurve(context, name);
  if (new_animation_curve->load(data)) {
    return new_animation_curve;
  } else {
    delete new_animation_curve;
    delete data;
    return NULL;
  }
}

int KRAnimationCurve::getFrameCount()
{
  m_pData->lock();
  int frame_count = ((animation_curve_header*)m_pData->getStart())->frame_count;
  m_pData->unlock();

  return frame_count;
}

void KRAnimationCurve::setFrameCount(int frame_count)
{
  m_pData->lock();
  int prev_frame_count = getFrameCount();
  if (frame_count != prev_frame_count) {
    float fill_value = 0.0f;
    if (prev_frame_count > 0) {
      fill_value = getValue(prev_frame_count - 1);
    }
    m_pData->expand(sizeof(animation_curve_header) + sizeof(float) * frame_count - m_pData->getSize());
    float* frame_data = (float*)((char*)m_pData->getStart() + sizeof(animation_curve_header));
    for (int frame_number = prev_frame_count; frame_number < frame_count; frame_number++) {
      frame_data[frame_number] = fill_value;
    }
    ((animation_curve_header*)m_pData->getStart())->frame_count = frame_count;
  }
  m_pData->unlock();
}

float KRAnimationCurve::getFrameRate()
{
  m_pData->lock();
  float frame_rate = ((animation_curve_header*)m_pData->getStart())->frame_rate;
  m_pData->unlock();
  return frame_rate;
}

void KRAnimationCurve::setFrameRate(float frame_rate)
{
  m_pData->lock();
  ((animation_curve_header*)m_pData->getStart())->frame_rate = frame_rate;
  m_pData->unlock();
}

int KRAnimationCurve::getFrameStart()
{
  m_pData->lock();
  int frame_start = ((animation_curve_header*)m_pData->getStart())->frame_start;
  m_pData->unlock();
  return frame_start;
}

void KRAnimationCurve::setFrameStart(int frame_number)
{
  m_pData->lock();
  ((animation_curve_header*)m_pData->getStart())->frame_start = frame_number;
  m_pData->unlock();
}

float KRAnimationCurve::getValue(int frame_number)
{
  m_pData->lock();
  //printf("frame_number: %i\n", frame_number);
  int clamped_frame = frame_number - getFrameStart();
  if (clamped_frame < 0) {
    clamped_frame = 0;
  } else if (clamped_frame >= getFrameCount()) {
    clamped_frame = getFrameCount() - 1;
  }
  float* frame_data = (float*)((char*)m_pData->getStart() + sizeof(animation_curve_header));
  float v = frame_data[clamped_frame];
  m_pData->unlock();
  return v;
}

void KRAnimationCurve::setValue(int frame_number, float value)
{
  m_pData->lock();
  int clamped_frame = frame_number - getFrameStart();
  if (clamped_frame >= 0 && clamped_frame < getFrameCount()) {
    float* frame_data = (float*)((char*)m_pData->getStart() + sizeof(animation_curve_header));
    frame_data[clamped_frame] = value;
  }
  m_pData->unlock();
}

float KRAnimationCurve::getValue(float local_time)
{
  // TODO - Need to add interpolation for time values between frames.
  //        Must consider looping animations when determining which two frames to interpolate between.
  m_pData->lock();
  float v = getValue((int)(local_time * getFrameRate()));
  m_pData->unlock();
  return v;
}

bool KRAnimationCurve::valueChanges(float start_time, float duration)
{
  m_pData->lock();
  bool c = valueChanges((int)(start_time * getFrameRate()), (int)(duration * getFrameRate()));
  m_pData->unlock();
  return c;
}

bool KRAnimationCurve::valueChanges(int start_frame, int frame_count)
{
  m_pData->lock();
  float first_value = getValue(start_frame);

  bool change_found = false;

  // Range of frames is not inclusive of last frame
  for (int frame_number = start_frame + 1; frame_number < start_frame + frame_count && !change_found; frame_number++) {
    if (getValue(frame_number) != first_value) {
      change_found = true;
    }
  }

  m_pData->unlock();
  return change_found;
}

KRAnimationCurve* KRAnimationCurve::split(const std::string& name, float start_time, float duration)
{
  return split(name, (int)(start_time * getFrameRate()), (int)(duration * getFrameRate()));
}

KRAnimationCurve* KRAnimationCurve::split(const std::string& name, int start_frame, int frame_count)
{
  KRAnimationCurve* new_curve = new KRAnimationCurve(getContext(), name);

  new_curve->setFrameRate(getFrameRate());
  new_curve->setFrameStart(start_frame);
  new_curve->setFrameCount(frame_count);
  new_curve->m_pData->lock();

  // Range of frames is not inclusive of last frame
  for (int frame_number = start_frame; frame_number < start_frame + frame_count; frame_number++) {
    new_curve->setValue(frame_number, getValue(frame_number)); // TODO - MEMCPY here?
  }
  new_curve->m_pData->unlock();

  getContext().getAnimationCurveManager()->addAnimationCurve(new_curve);
  return new_curve;
}

void KRAnimationCurve::_lockData()
{
  m_pData->lock();
}

void KRAnimationCurve::_unlockData()
{
  m_pData->unlock();
}
