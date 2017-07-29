#include "KREngine-common.h"

#include "KRHelpers.h"

namespace kraken {

void SetUniform(GLint location, const Vector2 &v)
{
  if (location != -1) GLDEBUG(glUniform2f(location, v.x, v.y));
}

void SetUniform(GLint location, const Vector3 &v)
{
  if (location != -1) GLDEBUG(glUniform3f(location, v.x, v.y, v.z));
}

void SetUniform(GLint location, const KRVector4 &v)
{
  if (location != -1) GLDEBUG(glUniform4f(location, v.x, v.y, v.z, v.w));
}

void SetUniform(GLint location, const KRMat4 &v)
{
  if (location != -1) GLDEBUG(glUniformMatrix4fv(location, 1, GL_FALSE, v.c));
}

void setXMLAttribute(const std::string &base_name, tinyxml2::XMLElement *e, const Vector3 &value, const Vector3 &default_value)
{
  // TODO - Increase number of digits after the decimal in floating point format (6 -> 12?)
  // FINDME, TODO - This needs optimization...
  if (value != default_value) {
    e->SetAttribute((base_name + "_x").c_str(), value.x);
    e->SetAttribute((base_name + "_y").c_str(), value.y);
    e->SetAttribute((base_name + "_z").c_str(), value.z);
  }
}

const Vector3 getXMLAttribute(const std::string &base_name, tinyxml2::XMLElement *e, const Vector3 &default_value)
{
  Vector3 value;
  if (e->QueryFloatAttribute((base_name + "_x").c_str(), &value.x) == tinyxml2::XML_SUCCESS
    && e->QueryFloatAttribute((base_name + "_y").c_str(), &value.y) == tinyxml2::XML_SUCCESS
    && e->QueryFloatAttribute((base_name + "_z").c_str(), &value.z) == tinyxml2::XML_SUCCESS) {
    return value;
  } else {
    return default_value;
  }
}

} // namespace kraken
