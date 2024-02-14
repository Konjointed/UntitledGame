#version 330 core

// Affects how the object reacts to light
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 

struct Light {
    vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
  
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 viewPos = vec3(0.0f, 0.0f, 3.0f);
uniform sampler2D objectTexture;
uniform Material material;
uniform Light light;

void main() {
    // Sample the texture
    vec4 texColor = texture(objectTexture, TexCoord);

    // Apply the texture color to ambient and diffuse components
    vec3 ambient = light.ambient * (material.ambient * texColor.rgb);
  	
    // Diffuse 
    vec3 norm = normalize(Normal);
    //vec3 lightDir = normalize(light.direction);
    vec3 lightDir = normalize(-vec3(-0.2f, -1.0f, -0.3f));
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.diffuse * texColor.rgb);
    
    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);  
    
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}