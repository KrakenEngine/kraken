//
//  KRLight.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-05.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KREngine_KRLight_h
#define KREngine_KRLight_h

#import "KRResource.h"

class KRLight : public KRResource {
public:
    ~KRLight();
    
    virtual std::string getExtension();
    virtual bool save(const std::string& path) = 0;
protected:
    KRLight(std::string name);
};

#endif
