#version 430 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texture_coordinate;


uniform mat4 model_matrix;
uniform mat4 proj_matrix;

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
    float w = 2 * 3.1415926535 * frequency * (position.x) +t;
    vec3 temp = position;
    temp.y = amplitude * sin(w);
    vec3 tangent = normalize(vec3(1,amplitude*cos(w)*4,0));
    v_out.normal = normalize(vec3(-tangent.y, tangent.x, 0));
    gl_Position = proj_matrix * model_matrix * vec4(temp, 1.0f);
    v_out.position = temp;
    v_out.texture_coordinate = vec2(texture_coordinate.x, 1.0f - texture_coordinate.y);

}