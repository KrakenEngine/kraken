//
//  KRSpotLight.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-05.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include <iostream>

#import "KRSpotLight.h"

KRSpotLight::KRSpotLight(std::string name) : KRLight(name)
{
}

KRSpotLight::~KRSpotLight()
{
    
}

bool KRSpotLight::save(const std::string& path)
{
    return true;
}
