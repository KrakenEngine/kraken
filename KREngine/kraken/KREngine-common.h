//
//  KREngine-common.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-03-15.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//



#ifndef KRENGINE_COMMON_H
#define KRENGINE_COMMON_H

float const PI = 3.141592653589793f;
float const D2R = PI * 2 / 360;


#include <stdint.h>
#include <vector>
#include <string>
#include <set>
#include <list>
#include <map>


#include <stack>
#include <queue>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>
#include <limits>
#include <unistd.h>
#include <iostream>
#include <math.h>
#include <pthread.h>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/signals2/mutex.hpp>

#include <atomic>
#include <thread>

#include "tinyxml2.h"


using std::vector;
using std::string;
using std::set;
using std::list;
using std::map;

using std::multimap;

using std::stack;
using std::queue;

#ifdef __APPLE__
#include "TargetConditionals.h"
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <Accelerate/Accelerate.h>
#define KRAKEN_HAVE_BLAS 1
#endif

#define KRENGINE_MAX_TEXTURE_UNITS 8


#if !defined(__i386__) && defined(__arm__)
#define KRAKEN_USE_ARM_NEON
#endif


#include <unordered_map>
using std::unordered_map;
using std::unordered_multimap;
using std::hash;

#if TARGET_OS_IPHONE

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>


#else

#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>

// OpenGL ES 2.0 mapping to OpenGL 3.2 mappings
#define glDepthRangef glDepthRange
#define glClearDepthf glClearDepth
#define glDeleteQueriesEXT glDeleteQueries
#define glGenQueriesEXT glGenQueries
#define glBeginQueryEXT glBeginQuery
#define glEndQueryEXT glEndQuery
#define glGetQueryObjectuivEXT glGetQueryObjectuiv
#define glTexStorage2DEXT glTexStorage2D
#define GL_ANY_SAMPLES_PASSED_EXT GL_ANY_SAMPLES_PASSED
#define GL_QUERY_RESULT_EXT GL_QUERY_RESULT

#endif

#include <Accelerate/Accelerate.h>
#include <AudioToolbox/AudioToolbox.h>
#include <AudioToolbox/AudioFile.h>
#include <AudioToolbox/ExtendedAudioFile.h>
#include <AudioToolbox/AUGraph.h>
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#if TARGET_OS_IPHONE
#include <OpenAL/oalMacOSX_OALExtensions.h>
#else
#include <OpenAL/MacOSX_OALExtensions.h>
#endif


#if DEBUG
#define GLDEBUG(x) \
x; \
{ \
GLenum e; \
while( (e=glGetError()) != GL_NO_ERROR) \
{ \
fprintf(stderr, "Error at line number %d, in file %s. glGetError() returned %i for call %s\n",__LINE__, __FILE__, e, #x ); \
} \
}
#else
#define GLDEBUG(x) x;
#endif


#if DEBUG
#define ALDEBUG(x) \
x; \
{ \
GLenum e; \
while( (e=alGetError()) != AL_NO_ERROR) \
{ \
fprintf(stderr, "Error at line number %d, in file %s. alGetError() returned %i for call %s\n",__LINE__, __FILE__, e, #x ); \
} \
}
#else
#define ALDEBUG(x) x;
#endif

#if DEBUG
#define OSDEBUG(x) \
{ \
OSStatus e = x; \
if( e != noErr) \
{ \
fprintf(stderr, "Error at line number %d, in file %s. Returned %d for call %s\n",__LINE__, __FILE__, e, #x ); \
} \
}
#else
#define OSDEBUG(x) x;
#endif


#if GL_EXT_debug_marker && DEBUG

#define GL_PUSH_GROUP_MARKER(x) glPushGroupMarkerEXT(0, x)
#define GL_POP_GROUP_MARKER glPopGroupMarkerEXT()

#else

#define GL_PUSH_GROUP_MARKER(x)
#define GL_POP_GROUP_MARKER

#endif

#define KRMIN(x,y) ((x) < (y) ? (x) : (y))
#define KRMAX(x,y) ((x) > (y) ? (x) : (y))
#define KRCLAMP(x, min, max) (KRMAX(KRMIN(x, max), min))
#define KRALIGN(x) ((x + 3) & ~0x03)

typedef enum {
    STREAM_LEVEL_OUT,
    STREAM_LEVEL_IN_LQ,
    STREAM_LEVEL_IN_HQ
} kraken_stream_level;

#include "KRVector4.h"
#include "KRVector3.h"
#include "KRVector2.h"
#include "KRBehavior.h"

#endif
