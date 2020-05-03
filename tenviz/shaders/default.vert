#version 420

in vec4 in_position;
uniform mat4 ProjModelview;

void main() {
  gl_Position = ProjModelview*in_position;
}
