#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 frag_uv;

layout(location = 0) out vec4 out_color;

layout(binding = 0) uniform sampler2D noise_texture;

void main() {
  vec4 texture_color = texture(noise_texture, frag_uv);
  out_color = vec4(texture_color.r, texture_color.r, texture_color.r, 1.0);
}