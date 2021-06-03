#version 450
#extension GL_ARB_separate_shader_objects : enable
#define ALPHA_DISCARD 0.05

layout(location = 0) in vec2 frag_uv;

layout(location = 0) out vec4 out_color;

layout(binding = 1, set = 1) uniform sampler2D text_texture;

void main() {
  vec4 texture_color = texture(text_texture, frag_uv);
  if (texture_color.r < ALPHA_DISCARD) {
    discard;
  }
  out_color = vec4(texture_color.r);
}