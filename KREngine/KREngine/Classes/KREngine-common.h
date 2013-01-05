//
//  KREngine-common.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-03-15.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//



#ifndef KRENGINE_COMMON_H
#define KRENGINE_COMMON_H

#define KRENGINE_MAX_TEXTURE_UNITS 8

float const PI = 3.141592653589793f;
float const D2R = PI * 2 / 360;

#import <stdint.h>
#import <vector>
#import <string>
#import <set>
#import <list>

#ifdef __APPLE__
#include "TargetConditionals.h"
#import <Accelerate/Accelerate.h>
#endif

#if TARGET_OS_IPHONE

#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>


#else

#import <OpenGL/gl3.h>
#import <OpenGL/gl3ext.h>

// OpenGL ES 2.0 mapping to OpenGL 3.2 mappings
#define glDepthRangef glDepthRange
#define glClearDepthf glClearDepth
#define glDeleteQueriesEXT glDeleteQueries
#define glGenQueriesEXT glGenQueries
#define glBeginQueryEXT glBeginQuery
#define glEndQueryEXT glEndQuery
#define glGetQueryObjectuivEXT glGetQueryObjectuiv
#define GL_ANY_SAMPLES_PASSED_EXT GL_ANY_SAMPLES_PASSED
#define GL_QUERY_RESULT_EXT GL_QUERY_RESULT

#endif

#import <AudioToolbox/AudioToolbox.h>
#import <AudioToolbox/AudioFile.h>
#import <AudioToolbox/ExtendedAudioFile.h>
#import <OpenAL/al.h>
#import <OpenAL/alc.h>
#import <OpenAL/oalMacOSX_OALExtensions.h>

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

#define KRMIN(x,y) ((x) < (y) ? (x) : (y))
#define KRMAX(x,y) ((x) > (y) ? (x) : (y))

#include "KRVector3.h"
#include "KRVector2.h"