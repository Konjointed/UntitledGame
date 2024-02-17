#version 410 core

layout(location = 0) in vec3 aPos;

uniform mat4 model;
uniform float time; // You'll need to pass the time uniform to this shader as well

void main()
{
    float windStrength = 0.1; // Adjust to match your main shader
    float windFrequency = 2.0; // Adjust to match your main shader

    // Calculate the wind effect
    float windEffect = sin(aPos.x * windFrequency + time) * windStrength;

    // Apply the wind effect by modifying the vertex position
    vec3 modifiedPos = aPos;
    modifiedPos.x += windEffect; // Apply the wind effect on the x-axis

    gl_Position = model * vec4(modifiedPos, 1.0);
}
