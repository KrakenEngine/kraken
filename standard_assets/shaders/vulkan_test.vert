#version 450

// Temporary test shader for vulkan bringup...
// See https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Shader_modules

layout(location = 0) out vec3 fragColor;
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_uv;
layout(constant_id = 0) const int QUALITY_LEVEL = 64; // Specialization constant test

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main() {
    gl_Position = vec4(vertex_position * 0.5, 1.0);
    fragColor = colors[gl_VertexIndex];
}
