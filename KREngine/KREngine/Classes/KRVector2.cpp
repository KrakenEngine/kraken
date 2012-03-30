//
//  KRVector2.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 12-03-22.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include <iostream>

#include "KRVector2.h"

KRVector2::KRVector2() {
    x = 0.0;
    y = 0.0;
}

KRVector2::KRVector2(float X, float Y) {
    x = X;
    y = Y;
}

KRVector2::~KRVector2() {

}

bool operator== (KRVector2 &v1, KRVector2 &v2) {
    return v1.x == v2.x && v1.y == v2.y;
    
}
bool operator!= (KRVector2 &v1, KRVector2 &v2) {
    return !(v1 == v2);
}