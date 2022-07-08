//
//  KRHelpers.h
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

#ifndef KRHELPERS_H
#define KRHELPERS_H

#include "vector2.h"
#include "vector3.h"
#include "matrix4.h"
#include <string>

#if defined(ANDROID)
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#elif defined(_WIN32) || defined(_WIN64)
#include <glad/glad.h>
#elif (defined(__linux__) || defined(__unix__) || defined(__posix__))
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#elif defined(__APPLE__)
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#endif
#include "../3rdparty/tinyxml2/tinyxml2.h"

#define KRMIN(x,y) ((x) < (y) ? (x) : (y))
#define KRMAX(x,y) ((x) > (y) ? (x) : (y))
#define KRCLAMP(x, min, max) (KRMAX(KRMIN(x, max), min))
#define KRALIGN(x) ((x + 3) & ~0x03)

float const PI = 3.141592653589793f;
float const D2R = PI * 2 / 360;

namespace kraken {
  void setXMLAttribute(const std::string &base_name, ::tinyxml2::XMLElement *e, const Vector3 &value, const Vector3 &default_value);
  const Vector3 getXMLAttribute(const std::string &base_name, ::tinyxml2::XMLElement *e, const Vector3 &default_value);
} // namespace kraken

#endif
