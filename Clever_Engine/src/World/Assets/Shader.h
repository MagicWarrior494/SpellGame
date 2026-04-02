#pragma once
#include "Asset.h"
#include "ShaderReflected.h"
#include "IRenderer.h"
#include <vector>
#include <unordered_map>

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
    GraphicsCore::ISampler*        sampler        = nullptr;

    // One layout per descriptor set index (set 0, set 1, ...).
    // Built automatically from SPIR-V reflection by ShaderLoader.
    std::vector<GraphicsCore::IResourceLayout*> layouts;

    // Reflected binding lookup: GLSL name ? ShaderResource (set, binding, type, size).
    // Used by the renderer to resolve ShaderDataComponent entries.
    std::unordered_map<std::string, ShaderResource> bindingsByName;

    ShaderReflection reflection;

    void DeleteGPUResources(GraphicsCore::IRenderer& renderer)
    {
        if (sampler) { renderer.DestroySampler(sampler); sampler = nullptr; }
        for (auto* layout : layouts)
        {
            if (layout) renderer.DestroyResourceLayout(layout);
        }
        layouts.clear();
        bindingsByName.clear();
        if (pipeline)       { renderer.DestroyPipeline(pipeline);     pipeline       = nullptr; }
        if (fragmentShader) { renderer.DestroyShader(fragmentShader); fragmentShader = nullptr; }
        if (vertexShader)   { renderer.DestroyShader(vertexShader);   vertexShader   = nullptr; }
    }
};