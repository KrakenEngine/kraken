//
//  KRDirectionalLight.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-05.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KREngine_KRDirectionalLight_h
#define KREngine_KRDirectionalLight_h

#import "KRLight.h"

class KRDirectionalLight : public KRLight {
    
public:
    KRDirectionalLight(std::string name);
    virtual ~KRDirectionalLight();
    
    virtual std::string getElementName();
};


#endif
