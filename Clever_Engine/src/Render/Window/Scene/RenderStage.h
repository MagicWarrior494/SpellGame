#pragma once
#include <string>
#include <vector>


enum class StageSource {
    ECS_COMPONENTS, // Filtered by a tag (Stars, Elements, Players)
    SINGLE_ENTITY,  // A specific entity (The Sun, The Terrain)
    FULL_SCREEN,    // Post-processing / Backgrounds
    COMPUTE         // No rendering, just data manipulation (Chemical reactions)
};

enum class StageOutput {
    SCENE_COLOR,    // Standard color attachment
    OFFSCREEN_IMAGE, // Render to a specific bitmap/texture for later use
    COMPUTE_BUFFER  // Output is just raw data (for your Material System)
};

enum class CullMode {
    NONE,
    FRONT,
    BACK
};

struct RenderStage {
    // --- Identity ---
    std::string name;           // "TerrainPass", "GlowEffect"
    StageSource sourceType;
    StageOutput outputType;

    // --- Shaders ---
    // If empty, the engine can use a "Fallback" error shader
    std::string shaderName;     // Base name (engine finds .vert and .frag)

    // --- Filtering ---
    // Used if sourceType is ECS_COMPONENTS or SINGLE_ENTITY
    std::string componentTag;

    // --- Resource Dependencies (The "Future-Proof" Part) ---
    // Tells the system: "This stage needs the output image from 'TerrainPass'"
    // The engine uses this to bind the correct textures to 'u_InputTexture'
    std::vector<std::string> inputDependencies;

    // --- State Overrides ---
    // Allows you to change how this specific stage behaves without a new class
    bool depthTest = true;
    bool depthWrite = true;
    bool alphaBlending = false;
    
	CullMode cullMode = CullMode::NONE;

    // --- Offscreen Settings ---
    // Only used if outputType is OFFSCREEN_IMAGE
    uint32_t outputWidth = 0;  // 0 means "Match Scene Width"
    uint32_t outputHeight = 0; // 0 means "Match Scene Height"
};