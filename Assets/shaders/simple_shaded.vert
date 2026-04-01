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
    mat4 model;
} pc;

// -------------------------
// Output to Fragment Shader
// -------------------------
layout(location = 0) out vec2  fragUV;
layout(location = 1) out vec3  fragNormalWorld;
layout(location = 2) out vec3  fragPosWorld;

void main()
{
    vec4 worldPos    = pc.model * vec4(inPosition, 1.0);
    gl_Position      = pc.mvp * vec4(inPosition, 1.0);
    fragUV           = inUV;
    fragPosWorld     = worldPos.xyz;
    // Normal matrix: inverse-transpose of the upper-left 3x3 of model
    fragNormalWorld  = normalize(mat3(transpose(inverse(pc.model))) * inNormal);
}
