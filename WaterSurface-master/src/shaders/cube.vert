#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 Normal;
out vec3 Position;

uniform mat4 model_matrix;
uniform mat4 proj_matrix;

void main()
{
    Normal =  aNormal;
    Position = vec3(vec4(aPos, 1.0));
    gl_Position = proj_matrix * model_matrix * vec4(aPos, 1.0);
}