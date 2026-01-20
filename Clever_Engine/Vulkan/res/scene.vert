#version 450

// Per-Scene Data (Camera)
layout(set = 0, binding = 0) uniform SceneBuffer {
    mat4 view;
    mat4 proj;
} scene;

// Per-Object Data (Instancing)
layout(std430, set = 0, binding = 1) readonly buffer MatrixBuffer {
    mat4 modelMatrices[];
} matrixBuffer;

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 fragColor;

void main() {
    mat4 modelMatrix = matrixBuffer.modelMatrices[gl_InstanceIndex];
    
    // Standard Transformation Pipeline: Proj * View * Model * Position
    gl_Position = scene.proj * scene.view * modelMatrix * vec4(inPosition, 1.0);
    
    // Map position to color. 
    // We use abs() to ensure we don't get negative colors if the model is centered at 0,0,0
    // We add 0.2 to make it a bit brighter.
    fragColor = abs(inPosition) + vec3(0.2, 0.2, 0.2); 
}