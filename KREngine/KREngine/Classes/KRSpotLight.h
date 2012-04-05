//
//  KRSpotLight.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-05.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KREngine_KRSpotLight_h
#define KREngine_KRSpotLight_h

#import "KRLight.h"

class KRSpotLight : public KRLight {
public:
    KRSpotLight(std::string name);
    ~KRSpotLight();
    
    virtual bool save(const std::string& path);
};


#endif
