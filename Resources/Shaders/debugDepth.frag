#version 410 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2DArray shadowMap;
uniform int layer; // The cascade layer to visualize

float LinearizeDepth(float depth, float near, float far)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main()
{   
    float depthValue = texture(shadowMap, vec3(TexCoords, layer)).r;
    depthValue = LinearizeDepth(depthValue, 0.1, 100.0); // Adjust near and far plane accordingly
    FragColor = vec4(vec3(depthValue), 1.0);
}