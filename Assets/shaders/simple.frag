#version 450

// -------------------------
// Input From Vertex Shader
// -------------------------
layout(location = 0) in vec4 fragColor;

// -------------------------
// Output
// -------------------------
layout(location = 0) out vec4 outColor;

void main()
{
    outColor = fragColor;
}