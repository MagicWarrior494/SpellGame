#pragma once
#include <memory>
#include <filesystem>

#include "IRenderer.h"
#include "World/Assets/Texture.h"

class TextureLoader
{
public:
    static std::shared_ptr<Texture> LoadFromFile(
        GraphicsCore::IRenderer* renderer,
        const std::filesystem::path& filePath);
};
