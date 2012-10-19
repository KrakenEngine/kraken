//
//  KREngine-common.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-03-15.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//



#ifndef KREngine_KREngine_common_h
#define KREngine_KREngine_common_h

#define KRENGINE_MAX_TEXTURE_UNITS 8

#import <stdint.h>
#import <vector>
#import <string>
#import <set>
#import <list>

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


#define GLDEBUG(x) \
x; \
{ \
GLenum e; \
while( (e=glGetError()) != GL_NO_ERROR) \
{ \
fprintf(stderr, "Error at line number %d, in file %s. glGetError() returned %i for call %s\n",__LINE__, __FILE__, e, #x ); \
} \
}