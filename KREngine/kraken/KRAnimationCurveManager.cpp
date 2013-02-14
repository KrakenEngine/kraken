//
//  KRAnimationCurveManager.cpp
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

#include "KRAnimationCurveManager.h"
#include "KRAnimationCurve.h"

KRAnimationCurveManager::KRAnimationCurveManager(KRContext &context) : KRContextObject(context)
{
    
}

KRAnimationCurveManager::~KRAnimationCurveManager() {
    for(map<std::string, KRAnimationCurve *>::iterator itr = m_animationCurves.begin(); itr != m_animationCurves.end(); ++itr){
        delete (*itr).second;
    }
}

KRAnimationCurve *KRAnimationCurveManager::loadAnimationCurve(const char *szName, KRDataBlock *data) {
    KRAnimationCurve *pAnimationCurve = KRAnimationCurve::Load(*m_pContext, szName, data);
    m_animationCurves[szName] = pAnimationCurve;
    return pAnimationCurve;
}

KRAnimationCurve *KRAnimationCurveManager::getAnimationCurve(const char *szName) {
    return m_animationCurves[szName];
}

std::map<std::string, KRAnimationCurve *> &KRAnimationCurveManager::getAnimationCurves() {
    return m_animationCurves;
}

void KRAnimationCurveManager::addAnimationCurve(KRAnimationCurve *new_animation_curve)
{
    m_animationCurves[new_animation_curve->getName()] = new_animation_curve;
}
