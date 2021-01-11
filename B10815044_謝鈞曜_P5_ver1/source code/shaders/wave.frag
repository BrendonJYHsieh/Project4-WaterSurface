#version 430 core
out vec4 f_color;

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};

struct PointLight {    
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  

struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};


in V_OUT
{
   vec3 position;
   vec3 normal;
   vec2 texture_coordinate;
   vec4 screenCoord;
} f_in;
 


uniform vec3 viewPos;


uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;

uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLights;
uniform SpotLight spotLight;

uniform vec4 color_ambient = vec4(0.1,0.2,0.5,1.0);
uniform vec4 color_diffuse = vec4(0.2,0.3,0.6,1.0);
uniform vec4 color_specular = vec4(1.0,1.0,1.0,1.0);
uniform vec4 Color =vec4(0.1,0.1,0.5,1.0);
uniform float shininess = 77.0;
uniform vec3 light_position = vec3(50.0,32.0,560.0);
uniform vec3 eyeDirection = vec3(0.0,0.0,1.0);

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);  
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

//float dot(vec3 v1,vec3 v2);
//float max(float a,float b);
//float min(vec4 a,vec4 b);
uniform bool direct_enable;
uniform bool point_enable;
uniform bool spot_enable;
uniform samplerCube skybox;
uniform bool reflect_enable;
uniform bool refract_enable;

uniform sampler2D reflect_texture;
uniform sampler2D refract_texture;
uniform sampler2D height_map_texture;
uniform vec3 light_pos ={30.0,70.0,100};
uniform int wave_mode;

void main()
{   
    vec3 norm;
    if(wave_mode == 3||wave_mode == 1){
        norm=-normalize(cross(dFdy(f_in.position),dFdx(f_in.position)));
    }
    else{
        norm = f_in.normal; 
    }


    vec3 result={0.0,0.0,0.0};
    vec3 viewDir = normalize(viewPos - f_in.position);

    float ratio = 1.00 / 1.52;
    vec3 I = normalize(f_in.position - viewPos);

    

    if(reflect_enable&&wave_mode!=1){
        result += reflect(I, normalize(norm));
        f_color = vec4(texture(skybox, result).rgb, 1.0);
    }
    else if(refract_enable&&wave_mode!=1){
        result += refract(I, normalize(norm), ratio);
        f_color = vec4(texture(skybox, result).rgb, 1.0);
    }
    else{
        float FresnelBase = 0.0;
		float FresnelScale = 10.0;
		float FresnelPower = 6.0;
        vec4 reflectColor = texture(reflect_texture, f_in.screenCoord.xy/f_in.screenCoord.w+(f_in.position.y/40));
        vec4 refractColor = texture(refract_texture, f_in.screenCoord.xy/f_in.screenCoord.w+(f_in.position.y/40));
        float fresnel = 0.0;
        if((-viewDir).y<0)
			{				
				fresnel = clamp( FresnelBase + FresnelScale * pow(1 - dot(f_in.normal, viewDir), FresnelPower), 0.0, 1.0);
			}
			else
			{				
			    fresnel = clamp( FresnelBase + FresnelScale * pow(1 - dot(-f_in.normal, viewDir), FresnelPower), 0.0, 1.0);
			}
        //
        vec4 base = {0.0,0.0,0.2,1.0};
        if(reflect_enable&&refract_enable){
           f_color = refractColor*(1-fresnel)+reflectColor*fresnel;
        }
        else if(reflect_enable){
           f_color = reflectColor*0.8+base*0.2;
        }
        else if(refract_enable){
           f_color = refractColor*0.8+base*0.2;
        }
        else{
            f_color = refractColor*(1-fresnel)+reflectColor*fresnel;
        }
    }
  
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
     //diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, f_in.texture_coordinate));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, f_in.texture_coordinate));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, f_in.texture_coordinate));
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
     //diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
     //specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
     //attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));    
     //combine results
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, f_in.texture_coordinate));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, f_in.texture_coordinate));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, f_in.texture_coordinate));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
     //spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, f_in.texture_coordinate));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, f_in.texture_coordinate));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, f_in.texture_coordinate));
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}
