#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_uv;

layout(location = 0) out vec2 frag_uv;

layout(binding = 0) uniform Global { mat4 proj; }
global;

void main() {
  gl_Position = global.proj * vec4(in_position, 0.0, 1.0);
  gl_Position.z = 0.0;
  frag_uv = in_uv;
}