#version 410 core
uniform samplerlD tex_toon;
uniform vec3 light_pos = vec3(30.0,30.0,100.0);
in VS_OUT

   vec3 normal;
   vec3 view;
} fs_in;
uniform vec4 C = vec4(0.5,0.0,0.0,1.0);
out vec4 color;
void main (void)
{
   // Calculate per-pixel normal and light vector
   vec3 N = normalize(fs_in.normal);
   vec3 L = normalize(light_pos - fs_in.view);//Y6M1frtglkflgtiff J
   // Simple N dot L diffuse lighting
   float tc = pow(max(0.0, dot(N, L)), 5.u);
   // Sample from cell shading texture
}  color = texture(textoon, tc) * (tc * 0.8 + 0.2);

