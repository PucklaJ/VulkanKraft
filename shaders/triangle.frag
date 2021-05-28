#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 frag_tex_coord;

layout(location = 0) out vec4 out_color;

layout(binding = 1) uniform sampler2D tex_sampler;

void main() {
  out_color = texture(tex_sampler, frag_tex_coord);
  out_color.rgb *= out_color.a;
}
