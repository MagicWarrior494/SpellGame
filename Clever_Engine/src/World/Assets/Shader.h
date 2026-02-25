#pragma once
#include "Asset.h"

#include "Render/Graphics/GraphicsAPI.h"

#include "ShaderReflected.h"

#include <stdexcept>

class Shader : public Asset
{
public:
    ShaderHandle vertexHandle;
    ShaderHandle fragmentHandle;

    ShaderReflection m_Reflection;

	PipelineLayoutHandle pipelineHandle;
};
ShaderReflection ReflectCombinedShaders(const std::vector<uint32_t>& vertCode, const std::vector<uint32_t>& fragCode);