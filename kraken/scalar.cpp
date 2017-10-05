//
//  KRFloat.cpp
//  Kraken
//
//  Created by Kearwood Gilbert on 2013-05-03.
//  Copyright (c) 2013 Kearwood Software. All rights reserved.
//

#include "public/kraken.h"

namespace kraken {

float SmoothStep(float a, float b, float t)
{
    float d = (3.0 * t * t - 2.0 * t * t * t);
    return a * (1.0f - d) + b * d;
}

} // namespace kraken
