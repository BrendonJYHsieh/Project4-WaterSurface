#version 430 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texture_coordinate;


uniform mat4 view_matrix;
uniform mat4 proj_matrix;
uniform mat4 model_matrix;
uniform float amplitude;
uniform float frequency;
uniform float t;

out V_OUT
{
   vec3 position;
   vec3 normal;
   vec2 texture_coordinate;
} v_out;

void main()
{
   // float k = 2 * 3.1415926535 * frequency;
   // float w =  k * (position.x)+t;

   vec3 p = position;
   // p.y = amplitude * sin(w);
   // vec3 tangent = normalize(vec3(1,k*amplitude*cos(w),0));
   // v_out.normal =  mat3(transpose(inverse(model_matrix)))*normalize(vec3(-tangent.y, tangent.x, 0));

    gl_Position = proj_matrix * view_matrix * model_matrix * vec4(p, 1.0f);
    v_out.position = vec3(model_matrix * vec4(p, 1.0));
    v_out.texture_coordinate = texture_coordinate;

}