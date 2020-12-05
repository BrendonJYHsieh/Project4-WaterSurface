#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 proj_matrix;
uniform mat4 model_matrix;

void main()
{
    TexCoords = aPos;
    gl_Position = proj_matrix * model_matrix * vec4(aPos, 1.0);
}
