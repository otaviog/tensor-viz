#version 420

in vec3 frag_color;

uniform float Transparency;

out vec4 out_frag_color;

void main() {
  out_frag_color = vec4(frag_color, Transparency);
}
