#version 450

layout(push_constant) uniform PushConstants
{
    vec2  position;
    vec2  size;
    vec4  color;
    vec2  resolution;
} pc;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = pc.color;
}
