#version 420

in vec4 in_position;
in vec3 in_normal;
in vec2 in_texcoord;

uniform mat4 Modelview;
uniform mat3 NormalModelview;
uniform mat4 ProjModelview;

out vec3 frag_pos;
out vec3 frag_normal;
out vec2 frag_texcoord;

void main() {
  gl_Position = ProjModelview * in_position;
  frag_pos = (Modelview * in_position).xyz;
  frag_normal = NormalModelview * in_normal;
  frag_texcoord = in_texcoord;
}
