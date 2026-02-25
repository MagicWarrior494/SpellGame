#version 450

// Input from vertex buffer
layout(location = 0) in vec3 inPosition;

// Uniform: model matrix
layout(set = 0, binding = 0) uniform ModelBuffer {
    mat4 model;
} uModel;

// Push constant: additional position offset
layout(push_constant) uniform PushConstants {
    vec3 offset;
} pc;

void main()
{
    // Apply model matrix, then add push constant offset
    gl_Position = uModel.model * vec4(inPosition + pc.offset, 1.0);
}