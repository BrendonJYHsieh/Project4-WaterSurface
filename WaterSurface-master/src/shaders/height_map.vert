#version 430 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texture_coordinate;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 proj_matrix;
uniform float amplitude;
uniform float frequency;
uniform sampler2D texture_d;

out V_OUT
{
   vec3 position;
   vec3 normal;
   vec2 texture_coordinate;
} v_out;

void main()
{
    vec3 height_map = position;
    height_map.y = height_map.y + (texture(texture_d,texture_coordinate).r) * amplitude;
    gl_Position = proj_matrix * view_matrix * model_matrix * vec4(height_map, 1.0f);
    v_out.position = (model_matrix*vec4(height_map, 1.0f)).xyz;
    v_out.normal = mat3(transpose(inverse(model_matrix)))*normal;
    v_out.texture_coordinate = texture_coordinate;

}