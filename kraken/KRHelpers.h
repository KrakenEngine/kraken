#ifndef KRHELPERS_H
#define KRHELPERS_H

#if defined(_WIN32) || defined(_WIN64)
#include <GL/glew.h>
#include "../3rdparty/tinyxml2/tinyxml2.h"
#endif

#include "KREngine-common.h"

#define KRMIN(x,y) ((x) < (y) ? (x) : (y))
#define KRMAX(x,y) ((x) > (y) ? (x) : (y))
#define KRCLAMP(x, min, max) (KRMAX(KRMIN(x, max), min))
#define KRALIGN(x) ((x + 3) & ~0x03)

float const PI = 3.141592653589793f;
float const D2R = PI * 2 / 360;

namespace kraken {
  void SetUniform(GLint location, const Vector2 &v);
  void SetUniform(GLint location, const KRVector3 &v);
  void SetUniform(GLint location, const KRVector4 &v);
  void SetUniform(GLint location, const KRMat4 &v);

  void setXMLAttribute(const std::string &base_name, ::tinyxml2::XMLElement *e, const KRVector3 &value, const KRVector3 &default_value);
  const KRVector3 getXMLAttribute(const std::string &base_name, ::tinyxml2::XMLElement *e, const KRVector3 &default_value);
} // namespace kraken

#endif