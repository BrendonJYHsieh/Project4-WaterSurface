#version 430 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texture_coordinate;


uniform mat4 view_matrix;
uniform mat4 proj_matrix;
uniform mat4 model_matrix;
uniform float amplitude;
uniform float frequency;
uniform float t;

uniform int wave_mode;
uniform sampler2D height_map_texture;

uniform vec2 uv_center;
uniform float uv_t;

uniform float PI = 3.1415926535;
float delta = 0.0001*100;


out V_OUT
{
   vec3 position;
   vec3 normal;
   vec2 texture_coordinate;
} v_out;



void main()
{ 
    if(wave_mode == 1){
        float k = 2 * PI * frequency;
        float w =  k * (position.x)+t;
        vec3 p = position;
        p.y = amplitude * sin(w);
        v_out.normal = mat3(transpose(inverse(model_matrix)))*normal;
        gl_Position = proj_matrix * view_matrix * model_matrix * vec4(p, 1.0f);
        v_out.position = vec3(model_matrix * vec4(p, 1.0));
        v_out.texture_coordinate = texture_coordinate;
    }
    else if(wave_mode == 2){
        vec3 height_map = position;
        vec3 height_map1 = position;
        float dist = distance(texture_coordinate, uv_center)*frequency*100;
        float t_c = (t-uv_t)*(2*PI);

        
        height_map.y = height_map.y + (texture2D(height_map_texture,vec2(texture_coordinate.x,texture_coordinate.y)).r) * amplitude;
        height_map.y+= amplitude * sin((dist-t_c)*clamp(0.0125*t_c,0,1))/(exp(0.1*abs(dist-t_c)+(0.05*t_c)))*1.5;

        //∫‚Normal
        float dx = delta;
        float dz = delta;
        float dy = (texture2D(height_map_texture,vec2(texture_coordinate.x+dx,texture_coordinate.y)).r-(texture2D(height_map_texture,vec2(texture_coordinate.x,texture_coordinate.y)).r))*amplitude;
        vec3 du = vec3(dx,dy,0.0);
        dy = (texture2D(height_map_texture,vec2(texture_coordinate.x,texture_coordinate.y+dz)).r-(texture2D(height_map_texture,vec2(texture_coordinate.x,texture_coordinate.y)).r))*amplitude;
        vec3 dv = vec3(0.0,dy,dz);
        
        v_out.normal = mat3(transpose(inverse(model_matrix)))*normalize(cross(dv,du));



        gl_Position = proj_matrix * view_matrix * model_matrix * vec4(height_map, 1.0f);
        v_out.position = (model_matrix*vec4(height_map, 1.0f)).xyz;
        //v_out.normal = mat3(transpose(inverse(model_matrix)))*normal;
        v_out.texture_coordinate = texture_coordinate;
    }
    else if(wave_mode ==3){
        vec3 p = position;
        float dist = distance(texture_coordinate, uv_center)*frequency*100;
        float t_c = (t-uv_t)*(2*PI);
        p.y += amplitude * sin((dist-t_c)*clamp(0.0125*t_c,0,1))/(exp(0.1*abs(dist-t_c)+(0.05*t_c)))*1.5;

        v_out.normal = mat3(transpose(inverse(model_matrix)))*normal;
        gl_Position = proj_matrix * view_matrix * model_matrix * vec4(p, 1.0f);
        v_out.position = vec3(model_matrix * vec4(p, 1.0));
        v_out.texture_coordinate = texture_coordinate;
    }
}