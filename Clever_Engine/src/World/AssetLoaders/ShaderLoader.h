#pragma once
#include <memory>
#include <filesystem>
#include <vector>
#include <string>
#include <fstream>

#include "Render/Graphics/GraphicsAPI.h"
#include "World/Assets/Shader.h"

class ShaderLoader
{
public:
	static std::shared_ptr<Shader> LoadFromFile(std::shared_ptr<GraphicsAPI> graphicsAPI, const std::filesystem::path& filePath);
};