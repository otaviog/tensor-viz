#version 420

layout(location = 0) out vec4 out_rgba8;
layout(location = 1) out vec4 out_rgba32f;
layout(location = 2) out vec4 out_rgba32i;
layout(location = 3) out int out_ri;

void main() {
  out_rgba8 = vec4(255, 0, 0, 1);
  out_rgba32f = vec4(0, 1.0, 0.0, 1);
  out_rgba32i = vec4(0, 0, 2000, 1);
  out_ri = 100;
}
