#version 420

uniform vec3 Color;

out vec4 out_color;

void main() {
  out_color = vec4(Color, 1);
}
