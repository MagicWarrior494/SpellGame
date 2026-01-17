#version 450

layout(std430, set = 0, binding = 0) readonly buffer MatrixBuffer {
    mat4 modelMatrices[];
} matrixBuffer;

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 fragColor;

void main() {

    mat4 modelMatrix = matrixBuffer.modelMatrices[gl_InstanceIndex];

    gl_Position = modelMatrix * vec4(inPosition, 1.0);
    fragColor = vec3(inPosition.x * 0.5 + 0.5, inPosition.y * 0.5 + 0.5, 0.0);
}