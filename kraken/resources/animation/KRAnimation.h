//
//  KRAnimation.h
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

#pragma once

#include "KREngine-common.h"
#include "KRContextObject.h"
#include "block.h"
#include "resources/KRResource.h"
#include "KRAnimationLayer.h"


class KRAnimation : public KRResource
{

public:
  KRAnimation(KRContext& context, std::string name);
  virtual ~KRAnimation();

  virtual std::string getExtension();
  virtual bool save(mimir::Block& data);

  static KRAnimation* Load(KRContext& context, const std::string& name, mimir::Block* data);

  void addLayer(KRAnimationLayer* layer);
  unordered_map<std::string, KRAnimationLayer*>& getLayers();
  KRAnimationLayer* getLayer(const char* szName);
  bool getAutoPlay() const;
  void setAutoPlay(bool auto_play);
  bool getLooping() const;
  void setLooping(bool looping);
  void Play();
  void Stop();
  void update(float deltaTime);
  float getTime();
  void setTime(float time);
  float getDuration();
  void setDuration(float duration);
  float getStartTime();
  void setStartTime(float start_time);
  bool isPlaying();

  KRAnimation* split(const std::string& name, float start_time, float duration, bool strip_unchanging_attributes = true, bool clone_curves = true);
  void deleteCurves();

  void _lockData();
  void _unlockData();

private:
  unordered_map<std::string, KRAnimationLayer*> m_layers;
  bool m_auto_play;
  bool m_loop;
  bool m_playing;
  float m_local_time;
  float m_duration;
  float m_start_time;
};
