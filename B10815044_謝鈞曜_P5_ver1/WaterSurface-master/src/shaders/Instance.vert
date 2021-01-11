#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 instanceMatrix;

out vec2 TexCoords;

uniform mat4 view_matrix;
uniform mat4 proj_matrix;

void main()
{
    gl_Position = proj_matrix * view_matrix * instanceMatrix * vec4(aPos, 1.0); 
    TexCoords = aTexCoords;
}