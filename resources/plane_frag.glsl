#version 410 core
layout (location = 0) out vec4 color;
layout (location = 1) out vec4 BrightColor;
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;
uniform vec3 campos;

uniform sampler2D tex;

void main()
{
   vec3 lightpos  = vec3(50, 100, 50);
   vec3 lightdir  = normalize(lightpos - vertex_pos);
   vec3 camdir    = normalize(campos - vertex_pos);
   vec3 frag_norm = normalize(vertex_normal);

   float diffuse_fact = clamp(dot(lightdir, frag_norm), 0, 1);

   vec3 h = normalize(camdir + lightdir);
   float spec_fact = clamp(dot(h, frag_norm), 0, 1);

   //vec4 tcol = texture(tex, vertex_tex)
   //color = tcol;

   color.rgb = vec3(1) * 0.1 + 
      vec3(0.4) * diffuse_fact + 
      vec3(1) * spec_fact;
   
   color.a=1;


   float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(color.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
