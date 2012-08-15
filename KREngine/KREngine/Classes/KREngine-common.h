//
//  KREngine-common.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-03-15.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//



#ifndef KREngine_KREngine_common_h
#define KREngine_KREngine_common_h

#import <stdint.h>
#import <vector>
#import <string>
#import <set>

#ifdef __APPLE__
#include "TargetConditionals.h"
#endif

#if TARGET_OS_IPHONE

#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

#else

#import <OpenGL/gl.h>
#import <OpenGL/glext.h>

#endif


#endif
