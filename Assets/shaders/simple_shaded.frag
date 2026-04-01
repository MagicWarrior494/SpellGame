#version 450

layout(set = 0, binding = 0) uniform sampler2D texSampler;

// -------------------------
// Input from Vertex Shader
// -------------------------
layout(location = 0) in vec2  fragUV;
layout(location = 1) in vec3  fragNormalWorld;
layout(location = 2) in vec3  fragPosWorld;

// -------------------------
// Output
// -------------------------
layout(location = 0) out vec4 outColor;

// -------------------------
// Sun Light Constants
// -------------------------
const vec3  SUN_DIR       = normalize(vec3(0.3, 1.0, 0.3)); // coming from above + slightly angled
const vec3  SUN_COLOR     = vec3(1.0, 0.95, 0.8);           // warm white
const float SUN_INTENSITY = 1.0;

const float AMBIENT       = 0.15;

const vec3  CAM_POS       = vec3(0.0, 3.0, 30.0);           // matches Application.cpp
const float SHININESS     = 32.0;
const float SPECULAR_STR  = 0.4;

void main()
{
    vec3 normal   = normalize(fragNormalWorld);
    vec4 texColor = texture(texSampler, fragUV);

    // Ambient
    vec3 ambient = AMBIENT * texColor.rgb;

    // Diffuse (Lambert)
    float diff   = max(dot(normal, SUN_DIR), 0.0);
    vec3 diffuse = diff * SUN_COLOR * SUN_INTENSITY * texColor.rgb;

    // Specular (Blinn-Phong)
    vec3 viewDir    = normalize(CAM_POS - fragPosWorld);
    vec3 halfwayDir = normalize(SUN_DIR + viewDir);
    float spec      = pow(max(dot(normal, halfwayDir), 0.0), SHININESS);
    vec3 specular   = SPECULAR_STR * spec * SUN_COLOR;

    vec3 result = ambient + diffuse + specular;
    outColor    = vec4(result, texColor.a);
}
