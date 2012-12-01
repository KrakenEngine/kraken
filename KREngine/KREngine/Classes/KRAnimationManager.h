//
//  KRAnimationManager.h
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-11-30.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KRANIMATIONMANAGER_H
#define KRANIMATIONMANAGER_H

#import "KREngine-common.h"

#include "KRAnimation.h"
#include "KRContextObject.h"
#include "KRDataBlock.h"

#include <map>
#import <string>

using std::map;

class KRAnimationManager : public KRContextObject {
public:
    KRAnimationManager(KRContext &context);
    virtual ~KRAnimationManager();
    
    KRAnimation *loadAnimation(const char *szName, KRDataBlock *data);
    KRAnimation *getAnimation(const char *szName);
    std::map<std::string, KRAnimation *> getAnimations();
    
    void startFrame();
    
private:
    map<std::string, KRAnimation *> m_animations;    
};



#endif
