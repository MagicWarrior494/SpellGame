#version 450
layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D sceneImages[16];
//16 current max number of scenes

layout(push_constant) uniform PushConstants {
    int sceneIndex;
} pc;

void main() {
    //outColor = vec4(float(), 0.5, 0.0, 1.0);
    outColor = texture(sceneImages[pc.sceneIndex], fragUV);
}