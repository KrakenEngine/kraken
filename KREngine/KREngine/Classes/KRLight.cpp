//
//  KRLight.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-05.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include <iostream>

#import "KRLight.h"

KRLight::KRLight(std::string name) : KRResource(name)
{

}

KRLight::~KRLight()
{

}

std::string KRLight::getExtension()
{
    return "krlight";
}