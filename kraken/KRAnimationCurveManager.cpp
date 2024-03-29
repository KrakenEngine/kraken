//
//  KRAnimationCurveManager.cpp
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

#include "KRAnimationCurveManager.h"
#include "KRAnimationCurve.h"

KRAnimationCurveManager::KRAnimationCurveManager(KRContext& context) : KRResourceManager(context)
{

}

KRAnimationCurveManager::~KRAnimationCurveManager()
{
  for (unordered_map<std::string, KRAnimationCurve*>::iterator itr = m_animationCurves.begin(); itr != m_animationCurves.end(); ++itr) {
    delete (*itr).second;
  }
}

void KRAnimationCurveManager::deleteAnimationCurve(KRAnimationCurve* curve)
{
  m_animationCurves.erase(curve->getName());
  delete curve;
}

KRResource* KRAnimationCurveManager::loadResource(const std::string& name, const std::string& extension, mimir::Block* data)
{
  if (extension.compare("kranimationcurve") == 0) {
    return loadAnimationCurve(name, data);
  }
  return nullptr;
}
KRResource* KRAnimationCurveManager::getResource(const std::string& name, const std::string& extension)
{
  if (extension.compare("kranimationcurve") == 0) {
    return getAnimationCurve(name);
  }
  return nullptr;
}

KRAnimationCurve* KRAnimationCurveManager::loadAnimationCurve(const std::string& name, mimir::Block* data)
{
  KRAnimationCurve* pAnimationCurve = KRAnimationCurve::Load(*m_pContext, name, data);
  if (pAnimationCurve) {
    m_animationCurves[name] = pAnimationCurve;
  }
  return pAnimationCurve;
}

KRAnimationCurve* KRAnimationCurveManager::getAnimationCurve(const std::string& name)
{
  unordered_map<std::string, KRAnimationCurve*>::iterator itr = m_animationCurves.find(name);
  if (itr == m_animationCurves.end()) {
    return NULL; // Not found
  } else {
    return (*itr).second;
  }
}

unordered_map<std::string, KRAnimationCurve*>& KRAnimationCurveManager::getAnimationCurves()
{
  return m_animationCurves;
}

void KRAnimationCurveManager::addAnimationCurve(KRAnimationCurve* new_animation_curve)
{
  assert(new_animation_curve != NULL);
  m_animationCurves[new_animation_curve->getName()] = new_animation_curve;
}

