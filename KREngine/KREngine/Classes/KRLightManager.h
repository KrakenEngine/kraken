//
//  KRLightManager.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-05.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KREngine_KRLightManager_h
#define KREngine_KRLightManager_h

class KRLightManager {
public:
    KRLightManager();
    ~KRLightManager();
    
    bool loadFile(const char *szPath);
};

#endif
