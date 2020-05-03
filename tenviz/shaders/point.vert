#version 420

in vec4 in_position;
in vec3 in_color;

uniform mat4 ProjModelview;

out vec3 frag_color;

void main() {
  gl_Position = ProjModelview*in_position;
  frag_color = in_color;
}
