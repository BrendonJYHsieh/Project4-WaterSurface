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
uniform vec3 light_pos ={30.0,70.0,100};

void main()
{   

    
    //vec3 R = refract(I, normalize(f_in.normal), ratio);
    // properties
    vec3 result={0.0,0.0,0.0};
    vec3 viewDir = normalize(viewPos - f_in.position);

    float ratio = 1.00 / 1.52;
    vec3 I = normalize(f_in.position - viewPos);

    

    if(reflect_enable && refract_enable){
        vec3 ReflectColor = vec3(textureCube(skybox, reflect(I, normalize(f_in.normal))));
        vec3 RefractColor = vec3(textureCube(skybox, refract(I, normalize(f_in.normal), ratio)));
        f_color = vec4(mix(RefractColor, ReflectColor, 0.5),1.0f); 
    }
    else if(reflect_enable){
        result += reflect(I, normalize(f_in.normal));
        f_color = vec4(texture(skybox, result).rgb, 1.0);
    }
    else if(refract_enable){
        result += refract(I, normalize(f_in.normal), ratio);
        f_color = vec4(texture(skybox, result).rgb, 1.0);
    }
    else{
        f_color = vec4(texture(skybox, result).rgb, 1.0);
    }
    
    
    
 
    /*
    if(direct_enable) {
        float intensity;
    vec4 color;
 
    vec3 n = normalize(f_in.normal);

    intensity = dot(-I,n);

    if (intensity > 0.95)
        color = vec4(1.0,0.5,0.5,1.0);
    else if (intensity > 0.5)
        color = vec4(0.6,0.3,0.3,1.0);
    else if (intensity > 0.25)
        color = vec4(0.4,0.2,0.2,1.0);
    else
        color = vec4(0.2,0.1,0.1,1.0);
        f_color = color;
    }
    else{
        f_color = vec4(texture(skybox, result).rgb, 1.0);
    }*/
    //if(point_enable) result += CalcPointLight(pointLights, f_in.normal,f_in.position, viewDir);
    //if(spot_enable) result += CalcSpotLight(spotLight, f_in.normal, f_in.position, viewDir);

    
 
    
    
    /*
    f_color += vec4(texture(texture_diffuse1,f_in.texture_coordinate));
    vec3 light_direction = normalize(light_position - f_in.position);
    vec3 normal = normalize(f_in.normal);
    vec3 half_vector = normalize(normalize(light_direction)+normalize(eyeDirection));
    float diffuse = max(0.0,dot(normal,light_direction));
    float specular = pow(max(0.0,dot(f_in.normal,half_vector)),shininess);
    f_color += min(Color * color_ambient,vec4(1.0)) +diffuse *color_diffuse+specular*color_specular;*/

    
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
