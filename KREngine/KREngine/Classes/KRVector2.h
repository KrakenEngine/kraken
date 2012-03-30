//
//  KRVector2.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-03-22.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KREngine_KRVector2_h
#define KREngine_KRVector2_h

class KRVector2 {
public:
    float x, y;
    KRVector2();
    KRVector2(float X, float Y);
    ~KRVector2();
    
    friend bool operator== (KRVector2 &v1, KRVector2 &v2);
    friend bool operator!= (KRVector2 &v1, KRVector2 &v2);
    
private:
    
    
};


#endif
