#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

uniform bool reflect_enable;
uniform bool refract_enable;
uniform vec3 viewPos;
uniform samplerCube skybox;

void main()
{    

    float ratio = 1.0 / 1.52;
    vec3 I = normalize(Position - viewPos);
    vec3 R={0.0,0.0,0.0};
    if(reflect_enable)
    R += reflect(I, normalize(Normal));
    if(refract_enable)
    R += refract(I, normalize(Normal), ratio);
    FragColor = vec4(texture(skybox, R).rgb, 1.0);

}