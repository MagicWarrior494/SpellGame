#pragma once
#include <memory>
#include <filesystem>
#include <vector>
#include <fstream>

#include "IRenderer.h"
#include "World/Assets/Shader.h"

class ShaderLoader
{
public:
    static std::shared_ptr<Shader> LoadFromFile(
        GraphicsCore::IRenderer* renderer,
        const std::filesystem::path& filePath);
};