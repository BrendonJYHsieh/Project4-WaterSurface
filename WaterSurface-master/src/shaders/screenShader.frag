#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float vx_offset=0.55;
uniform float rt_w; 
uniform float rt_h; 
uniform float t;
uniform float pixel_w = 7; // 15.0
uniform float pixel_h = 5; 
uniform bool pixel_enable;
uniform bool offset_enable;
uniform bool other_enable;

void main()
{
        if(pixel_enable){
            vec2 uv = TexCoords.xy;
            vec3 tc = vec3(1.0, 0.0, 0.0);
            if (uv.x < (vx_offset-0.005))
            {
            float dx = pixel_w*(1./rt_w);
            float dy = pixel_h*(1./rt_h);
            vec2 coord = vec2(dx*floor(uv.x/dx),
                                dy*floor(uv.y/dy));
            tc = texture2D(screenTexture, coord).rgb;
            }
            else if (uv.x>=(vx_offset+0.005))
            {
            tc = texture2D(screenTexture, uv).rgb;
            }
            FragColor = vec4(tc, 1.0);
        }
        else if(offset_enable){
            vec2 uv = TexCoords.xy;
            vec3 col = texture( screenTexture, uv + 0.005*vec2( sin(t+1024.0*uv.x),cos(t+768.0*uv.y)) ).xyz;
            FragColor = vec4(col, 1.0);
        }
        else if(other_enable){
            vec3 col = vec3(1.0 - texture(screenTexture, TexCoords));
            FragColor = vec4(col, 1.0);
        }
        else{
            vec3 col = texture(screenTexture, TexCoords).rgb;
            FragColor = vec4(col, 1.0);
        }
}   