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
// SSBO: per-instance world positions
// Must match the name used in ShaderDataComponent::BindBuffer("instancePositions", ...)
// -------------------------
layout(set = 0, binding = 1) readonly buffer InstancePositions
{
    vec4 positions[];
} instancePositions;

// -------------------------
// Output to Fragment Shader
// -------------------------
layout(location = 0) out vec2  fragUV;
layout(location = 1) out vec3  fragNormalWorld;
layout(location = 2) out vec3  fragPosWorld;

void main()
{
    vec3 instanceOffset = instancePositions.positions[gl_InstanceIndex].xyz;

    // Build a simple translation model matrix for this instance
    mat4 instanceModel = mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        instanceOffset.x, instanceOffset.y, instanceOffset.z, 1.0
    );

    vec4 worldPos   = instanceModel * vec4(inPosition, 1.0);
    gl_Position     = pc.mvp * worldPos;

    fragUV          = inUV;
    fragPosWorld    = worldPos.xyz;
    fragNormalWorld = normalize(mat3(transpose(inverse(instanceModel))) * inNormal);
}
