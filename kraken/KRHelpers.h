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
  void SetUniform(GLint location, const Vector2 &v);
  void SetUniform(GLint location, const Vector3 &v);
  void SetUniform(GLint location, const Vector4 &v);
  void SetUniform(GLint location, const Matrix4 &v);

  void setXMLAttribute(const std::string &base_name, ::tinyxml2::XMLElement *e, const Vector3 &value, const Vector3 &default_value);
  const Vector3 getXMLAttribute(const std::string &base_name, ::tinyxml2::XMLElement *e, const Vector3 &default_value);
} // namespace kraken

#endif
