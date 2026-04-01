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

    GraphicsCore::IShader*         vertexShader   = nullptr;
    GraphicsCore::IShader*         fragmentShader = nullptr;
    GraphicsCore::IPipeline*       pipeline       = nullptr;
    GraphicsCore::IResourceLayout* textureLayout  = nullptr;
    GraphicsCore::ISampler*        sampler        = nullptr;

    ShaderReflection reflection;

    void DeleteGPUResources(GraphicsCore::IRenderer& renderer)
    {
        if (sampler)        { renderer.DestroySampler(sampler);               sampler        = nullptr; }
        if (textureLayout)  { renderer.DestroyResourceLayout(textureLayout);  textureLayout  = nullptr; }
        if (pipeline)       { renderer.DestroyPipeline(pipeline);             pipeline       = nullptr; }
        if (fragmentShader) { renderer.DestroyShader(fragmentShader);         fragmentShader = nullptr; }
        if (vertexShader)   { renderer.DestroyShader(vertexShader);           vertexShader   = nullptr; }
    }
};