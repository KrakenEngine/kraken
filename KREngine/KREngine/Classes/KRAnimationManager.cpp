//
//  KRAnimationManager.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-11-30.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KRAnimationManager.h"
#include "KRAnimation.h"

KRAnimationManager::KRAnimationManager(KRContext &context) : KRContextObject(context)
{

}

KRAnimationManager::~KRAnimationManager() {
    for(map<std::string, KRAnimation *>::iterator itr = m_animations.begin(); itr != m_animations.end(); ++itr){
        delete (*itr).second;
    }
}

void KRAnimationManager::startFrame()
{
    
}


KRAnimation *KRAnimationManager::loadAnimation(const char *szName, KRDataBlock *data) {
    KRAnimation *pAnimation = KRAnimation::Load(*m_pContext, szName, data);
    m_animations[szName] = pAnimation;
    return pAnimation;
}

KRAnimation *KRAnimationManager::getAnimation(const char *szName) {
    return m_animations[szName];
}

std::map<std::string, KRAnimation *> KRAnimationManager::getAnimations() {
    return m_animations;
}

