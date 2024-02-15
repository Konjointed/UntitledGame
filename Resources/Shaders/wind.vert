#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal; // Normal can be used for more advanced effects
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time; // Pass the elapsed time since the start of the application

void main()
{
    float windStrength = 0.1;
    float windFrequency = 2.0;
    
    // Calculate wind effect based on position and time to create a moving effect
    float windEffect = sin(aPos.x * windFrequency + time) * cos(aPos.z * windFrequency + time) * windStrength;
    
    // Apply the wind effect by modifying the vertex position along the Y-axis
    vec3 position = aPos + vec3(0.0, windEffect, 0.0);
    
    gl_Position = projection * view * model * vec4(position, 1.0);
    TexCoords = aTexCoords;
}
