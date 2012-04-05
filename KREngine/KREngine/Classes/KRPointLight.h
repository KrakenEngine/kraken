//
//  KRPointLight.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-05.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KREngine_KRPointLight_h
#define KREngine_KRPointLight_h

#import "KRLight.h"

class KRPointLight : public KRLight {
    
public:
    KRPointLight(std::string name);
    ~KRPointLight();
    
    virtual bool save(const std::string& path);
};

#endif
