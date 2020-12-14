#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;
uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 proj_matrix;


void main()
{
    TexCoords = aTexCoords;
    gl_Position =proj_matrix * view_matrix * model_matrix * vec4(aPos.x, aPos.y, 0.0, 1.0); 
}  