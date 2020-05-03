#version 420

in vec4 position;

uniform mat4 ProjModelview;

void main() {
  gl_Position = ProjModelview * position;  
}
  

