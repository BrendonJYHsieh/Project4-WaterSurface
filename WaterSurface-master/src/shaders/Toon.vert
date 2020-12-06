#version 410 core
uniform mat4 model_matrix;
uniform mat4 proj_matrix;
layout (location = 0) in vec4 positon;
layout (location = 1) in vec3 normal;

out VS_OUT{
    vec3 normal;
    vec3 view;
}vs_out;


void main()
{
    vec4 pos_vs = model_matrix*positon;
    vs_out.normal = mat3(model_matrix) * normal;
    vs_out.view = pos_vs.xyz;
    gl_Position = proj_matrix * pos_vs;
}