#version 450
#extension GL_ARB_separate_shader_objects : enable
#define ALPHA_DISCARD 0.005

layout(location = 0) in vec2 frag_uv;

layout(location = 0) out vec4 out_color;

layout(binding = 1, set = 1) uniform sampler2D texture_sampler;

void main() {
  out_color = texture(texture_sampler, frag_uv);
  if (out_color.a < ALPHA_DISCARD) {
    discard;
  }
}