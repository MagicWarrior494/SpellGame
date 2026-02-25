#version 450

// -------------------------
// Vertex Input
// -------------------------
layout(location = 0) in vec3 inPosition;

// -------------------------
// Uniform Buffer
// -------------------------
layout(set = 0, binding = 0) uniform ColorBlock
{
    vec4 color;
} ubo;

// -------------------------
// Output to Fragment Shader
// -------------------------
layout(location = 0) out vec4 fragColor;

void main()
{
    // Simple pass-through position
    gl_Position = vec4(inPosition, 1.0);

    // Pass uniform color to fragment
    fragColor = ubo.color;
}