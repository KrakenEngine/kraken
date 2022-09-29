//
//  Kraken Engine-common.h
//  Kraken Engine
//
//  Copyright 2022 Kearwood Gilbert. All rights reserved.
//  
//  Redistribution and use in source and binary forms, with or without modification, are
//  permitted provided that the following conditions are met:
//  
//  1. Redistributions of source code must retain the above copyright notice, this list of
//  conditions and the following disclaimer.
//  
//  2. Redistributions in binary form must reproduce the above copyright notice, this list
//  of conditions and the following disclaimer in the documentation and/or other materials
//  provided with the distribution.
//  
//  THIS SOFTWARE IS PROVIDED BY KEARWOOD GILBERT ''AS IS'' AND ANY EXPRESS OR IMPLIED
//  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL KEARWOOD GILBERT OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//  
//  The views and conclusions contained in the software and documentation are those of the
//  authors and should not be interpreted as representing official policies, either expressed
//  or implied, of Kearwood Gilbert.
//

#pragma once

#define KRENGINE_MAX_GPU_COUNT 4
#define KRENGINE_MAX_FRAMES_IN_FLIGHT 2
#define KRENGINE_DEBUG_GPU_LABELS 1
#define KRENGINE_DEBUG_GPU_LABEL_MAX_LEN 128

#include "public/kraken.h"
#include "KRHelpers.h"
using namespace kraken;

#include "hydra.h"

#include <stdint.h>
#include <vector>
#include <string>
#include <set>
#include <list>
#include <map>
#include <variant>
#include <algorithm>
#include <stack>
#include <queue>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdint.h>
#include <stdio.h>

#include "../3rdparty/tinyxml2/tinyxml2.h"
#include "../3rdparty/glslang/glslang/Public/ShaderLang.h"
#include "../3rdparty/glslang/SPIRV/GlslangToSpv.h"
#if defined(__APPLE__)

#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>

#include <AudioToolbox/AudioToolbox.h>
#include <AudioToolbox/AudioFile.h>
#include <AudioToolbox/ExtendedAudioFile.h>
#include <AudioToolbox/AUGraph.h>

#include "TargetConditionals.h"
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <Accelerate/Accelerate.h>
#define KRAKEN_HAVE_BLAS 1

#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>
#include <limits>
#include <iostream>

// NOMINMAX required to prevent windows headers from defining max() and min() macros
#define NOMINMAX

// _USE_MATH_DEFINES must be defined to get M_PI in Windows
#define _USE_MATH_DEFINES
#include <math.h>


#include <atomic>
#include <thread>
#include <mutex>



using std::vector;
using std::string;
using std::set;
using std::list;
using std::map;
using std::unique_ptr;
using std::shared_ptr;

using std::multimap;

using std::stack;
using std::queue;


#if !defined(__i386__) && defined(__arm__)
#define KRAKEN_USE_ARM_NEON
#endif


#include <unordered_map>
using std::unordered_map;
using std::unordered_multimap;
using std::hash;

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <volk.h>

#if defined(_WIN32) || defined(_WIN64)

#include <cstdint>
typedef int64_t __int64_t;
typedef uint64_t __uint64_t;
typedef int32_t __int32_t;
typedef uint32_t __uint32_t;
typedef int16_t __int16_t;
typedef uint16_t __uint16_t;
typedef int8_t __int8_t;
typedef uint8_t __uint8_t;

#endif


// TODO - Vulkan Refactoring
// If we hit any old GL calls, assert
#define GLDEBUG(x) \
assert(false);

#if defined(DEBUG) || defined(_DEBUG)
#define OSDEBUG(x) \
{ \
OSStatus e = x; \
if( e != noErr) \
{ \
fprintf(stderr, "Error at line number %d, in file %s. Returned %d for call %s\n",__LINE__, __FILE__, (int)e, #x ); \
} \
}
#else
#define OSDEBUG(x) x;
#endif

#define GL_PUSH_GROUP_MARKER(x)
#define GL_POP_GROUP_MARKER


typedef enum
{
  STREAM_LEVEL_OUT,
  STREAM_LEVEL_IN_LQ,
  STREAM_LEVEL_IN_HQ
} kraken_stream_level;

typedef int KrDeviceHandle;
typedef int KrSurfaceHandle;

#include "KRBehavior.h"

using namespace kraken;
