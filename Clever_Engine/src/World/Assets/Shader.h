#pragma once
#include "Asset.h"
#include "ShaderReflected.h"
#include "IRenderer.h"

class Shader : public Asset
{
public:
    ~Shader()
    {
        // GPU resources destroyed via IRenderer by AssetManager before shutdown
    }

    GraphicsCore::IShader*   vertexShader   = nullptr;
    GraphicsCore::IShader*   fragmentShader = nullptr;
    GraphicsCore::IPipeline* pipeline       = nullptr;

    ShaderReflection reflection;
};