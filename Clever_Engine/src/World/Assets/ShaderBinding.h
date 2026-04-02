#pragma once
#include <string>
#include "IRenderer.h"

// A named buffer binding — maps a GLSL binding name (e.g. "light_positions") to a GPU buffer.
// The user creates the buffer and holds onto it; the renderer will bind it each frame.
struct BufferBinding
{
    std::string                name;    // Must match the block/variable name in the GLSL source
    GraphicsCore::IBuffer*     buffer  = nullptr;
    size_t                     offset  = 0;
    size_t                     range   = 0;     // 0 = use full buffer size
};

// A named texture binding — maps a GLSL sampler name (e.g. "albedo") to a GPU texture.
struct TextureBinding
{
    std::string                name;    // Must match the uniform sampler name in the GLSL source
    GraphicsCore::ITexture*    texture = nullptr;
    GraphicsCore::ISampler*    sampler = nullptr; // nullptr = use the shader's default sampler
};
