#version 430 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texture_coordinate;


uniform mat4 model_matrix;
uniform mat4 u_model;
uniform mat4 proj_matrix;
uniform sampler2D height_map_image;

out V_OUT
{
   vec3 position;
   vec3 normal;
   vec2 texture_coordinate;
} v_out;

void main()
{
    vec3 height_map = position;
    height_map.y = height_map.y + texture(height_map_image,texture_coordinate).r;
    gl_Position = proj_matrix * model_matrix * vec4(height_map, 1.0f);
    v_out.position = height_map;
    vec3 forward = position;
    forward.x +=1;
    vec3 right = position;
    right.z +=1;
    forward = forward-position;
    right = right-position;
    v_out.normal = normalize(right*forward);
    v_out.texture_coordinate = vec2(texture_coordinate.x, 1.0f - texture_coordinate.y);

}