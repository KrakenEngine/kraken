#version 450
#extension GL_GOOGLE_include_directive : enable

#include "vulkan_test_include.glsl"

// Temporary test shader for vulkan bringup...
// See https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Shader_modules

layout(location = 0) out vec3 fragColor;
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec3 vertex_tangent;
layout(constant_id = 0) const int QUALITY_LEVEL = 64; // Specialization constant test

layout( push_constant ) uniform constants
{
	vec3 fade_color;
	mat4 model_matrix;
} PushConstants;

void main() {
    gl_Position = vec4(vertex_position * 0.5, 1.0);
    fragColor = vertex_normal * 0.25 + vec3(0.5, 0.5, 0.5) * VULKAN_TEST_BRIGHTNESS;
}
