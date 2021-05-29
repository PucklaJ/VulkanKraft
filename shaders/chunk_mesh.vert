#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 in_position;

layout(location = 0) out vec3 frag_position;

layout(binding = 0) uniform Global { mat4 proj_view; }
global;

void main() {
  gl_Position = global.proj_view * vec4(in_position, 1.0);
  frag_position = in_position;
}
