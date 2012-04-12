//
//  KRPointLight.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-05.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include <iostream>

#import "KRPointLight.h"

KRPointLight::KRPointLight(std::string name) : KRLight(name)
{

}

KRPointLight::~KRPointLight()
{

}

std::string KRPointLight::getElementName() {
    return "point_light";
}
