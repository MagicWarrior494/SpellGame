#version 450

// -------------------------
// Vertex Input
// -------------------------
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

// -------------------------
// Push Constants
// -------------------------
layout(push_constant) uniform PushConstants
{
    mat4 mvp;
} pc;

// -------------------------
// Output to Fragment Shader
// -------------------------
layout(location = 0) out vec4 fragColor;

void main()
{
    gl_Position = pc.mvp * vec4(inPosition, 1.0);
    fragColor = vec4(inNormal * 0.5 + 0.5, 1.0);
}
