#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;

layout(location = 0) out vec3 frag_color;

layout(binding = 0) uniform Global { mat4 proj; }
global;
layout(push_constant) uniform PushConstants { mat4 m; }
model;

void main() {
  gl_Position = global.proj * model.m * vec4(in_position, 1.0);
  frag_color = in_color;
}