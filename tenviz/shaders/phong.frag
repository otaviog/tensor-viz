#version 420

uniform vec4 Lightpos;
uniform mat4 Modelview;

uniform vec4 AmbientColor;
uniform vec4 DiffuseColor;
uniform vec4 SpecularColor;
uniform float SpecularExp;
uniform sampler2D Tex;

in vec3 frag_pos;
in vec3 frag_normal;
in vec2 frag_texcoord;

out vec4 out_frag_color;

void main() {
  // Begin
  vec3 v_normal = normalize(frag_normal);
  vec3 v_light = normalize((Modelview * Lightpos).xyz - frag_pos);

  vec3 v_view = normalize(-frag_pos);
  vec3 v_ref = 2 * dot(v_normal, v_light) * v_normal - v_light;

  out_frag_color = AmbientColor * texture2D(Tex, frag_texcoord);

  // Diffuse component
  out_frag_color += max(dot(v_normal, v_light), 0) * DiffuseColor;

  // Specular component
  out_frag_color +=
      pow(max(dot(v_view, v_ref), 0), SpecularExp) * SpecularColor;

  out_frag_color =
      clamp(out_frag_color, vec4(0.0, 0.0, 0.0, 0.0), vec4(1.0, 1.0, 1.0, 1.0));
}
