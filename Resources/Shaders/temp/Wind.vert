#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform float time; // Uniform variable to represent time

void main()
{
    float windStrength = 0.1; // Adjust the wind strength
    float windFrequency = 2.0; // Adjust the frequency of the swaying motion
    
    // Calculate the wind effect
    float windEffect = sin(aPos.x * windFrequency + time) * windStrength;
    
    // Apply the wind effect by modifying the vertex position
    vec3 modifiedPos = aPos;
    modifiedPos.x += windEffect; // Apply the wind effect on the x-axis
    
    vs_out.FragPos = vec3(model * vec4(modifiedPos, 1.0));
    vs_out.Normal = transpose(inverse(mat3(model))) * aNormal;
    vs_out.TexCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(modifiedPos, 1.0);
}
