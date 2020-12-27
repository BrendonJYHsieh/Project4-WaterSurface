#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 bPos;

out vec2 TexCoords;
out vec4 ParticleColor;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 proj_matrix;

uniform vec3 offset;
uniform vec4 color;
uniform float scale;

void main()
{
    TexCoords =aPos.xy;
    ParticleColor = color;
    gl_Position = proj_matrix * view_matrix * model_matrix * vec4(aPos, 1.0);
}