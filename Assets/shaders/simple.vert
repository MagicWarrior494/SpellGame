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
layout(location = 0) out vec2 fragUV;

void main()
{
    gl_Position = pc.mvp * vec4(inPosition, 1.0);
    fragUV = inUV;
}
