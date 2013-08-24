//
//  KRAnimationManager.cpp
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

#include "KRAnimationManager.h"
#include "KRAnimation.h"

KRAnimationManager::KRAnimationManager(KRContext &context) : KRContextObject(context)
{

}

KRAnimationManager::~KRAnimationManager() {
    for(unordered_map<std::string, KRAnimation *>::iterator itr = m_animations.begin(); itr != m_animations.end(); ++itr){
        delete (*itr).second;
    }
}

void KRAnimationManager::startFrame(float deltaTime)
{
    for(std::set<KRAnimation *>::iterator itr = m_animationsToUpdate.begin(); itr != m_animationsToUpdate.end(); itr++) {
        KRAnimation *animation = *itr;
        std::set<KRAnimation *>::iterator active_animations_itr = m_activeAnimations.find(animation);
        if(animation->isPlaying()) {
            // Add playing animations to the active animations list
            if(active_animations_itr == m_activeAnimations.end()) {
                m_activeAnimations.insert(animation);
            }
        } else {
            // Remove stopped animations from the active animations list
            if(active_animations_itr != m_activeAnimations.end()) {
                m_activeAnimations.erase(active_animations_itr);
            }
        }
    }
    
    m_animationsToUpdate.clear();
    
    for(std::set<KRAnimation *>::iterator active_animations_itr = m_activeAnimations.begin(); active_animations_itr != m_activeAnimations.end(); active_animations_itr++) {
        KRAnimation *animation = *active_animations_itr;
        animation->update(deltaTime);
    }
}

void KRAnimationManager::endFrame(float deltaTime)
{
    
}


KRAnimation *KRAnimationManager::loadAnimation(const char *szName, KRDataBlock *data) {
    KRAnimation *pAnimation = KRAnimation::Load(*m_pContext, szName, data);
    addAnimation(pAnimation);
    return pAnimation;
}

KRAnimation *KRAnimationManager::getAnimation(const char *szName) {
    return m_animations[szName];
}

unordered_map<std::string, KRAnimation *> &KRAnimationManager::getAnimations() {
    return m_animations;
}

void KRAnimationManager::addAnimation(KRAnimation *new_animation)
{
    m_animations[new_animation->getName()] = new_animation;
    updateActiveAnimations(new_animation);
}

void KRAnimationManager::updateActiveAnimations(KRAnimation *animation)
{
    m_animationsToUpdate.insert(animation);
}

void KRAnimationManager::deleteAnimation(KRAnimation *animation, bool delete_curves)
{
    if(delete_curves)
    {
        animation->deleteCurves();
    }
    m_animations.erase(animation->getName());
    delete animation;
}
