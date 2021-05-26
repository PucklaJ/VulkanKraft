#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 in_position;

layout(binding = 0) uniform MatrixData {
  mat4 model;
  mat4 view;
  mat4 proj;
}
ubo0;

void main() {
  gl_Position = ubo0.proj * ubo0.view * ubo0.model * vec4(in_position, 1.0);
}
