#pragma once
#include "Asset.h"

#include "Render/Graphics/GraphicsAPI.h"
#include "Helper/spirv_reflect.h"
#include "Resource.h"

#include <stdexcept>

class Shader : public Asset
{
public:
    ShaderHandle vertexHandle;
    ShaderHandle fragmentHandle;

    ShaderMetadata metaData;

    void UploadToGPU(GraphicsAPI& api) override
    {

	}
};

ShaderMetadata ReflectCombinedShaders(const std::vector<uint32_t>& vertCode, const std::vector<uint32_t>& fragCode);