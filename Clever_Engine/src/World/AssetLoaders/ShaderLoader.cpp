#include "ShaderLoader.h"
#include "World/Assets/Shader.h"
#include "World/Assets/Vertex.h"
#include <map>

// Declared in Shader.cpp
ShaderReflection ReflectCombinedShaders(const std::vector<uint32_t>& vertCode,
    const std::vector<uint32_t>& fragCode);

static std::vector<uint32_t> ReadSpirvFile(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Cannot open shader file: " + path.string());

    const size_t fileSize = static_cast<size_t>(file.tellg());
    if (fileSize % sizeof(uint32_t) != 0)
        throw std::runtime_error("SPIR-V file size is not a multiple of 4: " + path.string());

    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(fileSize));
    return buffer;
}

// Maps a reflected ShaderResourceType to the GraphicsCore ResourceType used by IResourceLayout.
static GraphicsCore::ResourceType ToGCResourceType(ShaderResourceType type)
{
    switch (type)
    {
    case ShaderResourceType::StorageBuffer:
    case ShaderResourceType::StorageBufferDynamic: return GraphicsCore::ResourceType::StorageBuffer;
    case ShaderResourceType::CombinedImageSampler:
    case ShaderResourceType::SampledImage:         return GraphicsCore::ResourceType::Texture;
    case ShaderResourceType::Sampler:              return GraphicsCore::ResourceType::Sampler;
    default:                                       return GraphicsCore::ResourceType::UniformBuffer;
    }
}

// Maps a reflected ShaderStage (engine enum from ShaderReflected.h) to GraphicsCore::ShaderStage.
static GraphicsCore::ShaderStage ToGCStage(ShaderStage stage)
{
    GraphicsCore::ShaderStage result = GraphicsCore::ShaderStage::Vertex;
    if ((stage & ShaderStage::Fragment) != ShaderStage::None)
        result = GraphicsCore::ShaderStage::Fragment;
    if ((stage & ShaderStage::Vertex) != ShaderStage::None)
        result = GraphicsCore::ShaderStage::Vertex;
    // Both vertex and fragment — default to fragment (most bindings are fragment-side)
    if ((stage & ShaderStage::Vertex)   != ShaderStage::None &&
        (stage & ShaderStage::Fragment) != ShaderStage::None)
        result = GraphicsCore::ShaderStage::Fragment;
    return result;
}

std::shared_ptr<Shader> ShaderLoader::LoadFromFile(
    GraphicsCore::IRenderer* renderer,
    const std::filesystem::path& filePath)
{
    if (!renderer)
        throw std::runtime_error("ShaderLoader: renderer is null");

    std::filesystem::path vertPath = filePath;
    vertPath.replace_extension(".vert.spv");
    std::filesystem::path fragPath = filePath;
    fragPath.replace_extension(".frag.spv");

    const std::vector<uint32_t> vertCode = ReadSpirvFile(vertPath);
    const std::vector<uint32_t> fragCode = ReadSpirvFile(fragPath);

    GraphicsCore::ShaderDesc vertDesc{};
    vertDesc.stage        = GraphicsCore::ShaderStage::Vertex;
    vertDesc.bytecode     = vertCode.data();
    vertDesc.bytecodeSize = vertCode.size() * sizeof(uint32_t);
    vertDesc.entryPoint   = "main";

    GraphicsCore::ShaderDesc fragDesc{};
    fragDesc.stage        = GraphicsCore::ShaderStage::Fragment;
    fragDesc.bytecode     = fragCode.data();
    fragDesc.bytecodeSize = fragCode.size() * sizeof(uint32_t);
    fragDesc.entryPoint   = "main";

    auto shader = std::make_shared<Shader>();
    shader->vertexShader   = renderer->CreateShader(vertDesc);
    shader->fragmentShader = renderer->CreateShader(fragDesc);
    shader->reflection     = ReflectCombinedShaders(vertCode, fragCode);

    // ---------------------------------------------------------------
    // Build descriptor set layouts from reflection.
    // Group all reflected resources by their set index, then create
    // one IResourceLayout per set. The layouts vector is indexed by
    // set number so set 0 ? layouts[0], set 1 ? layouts[1], etc.
    // ---------------------------------------------------------------

    // Deduplicate resources that appear in both stages (same set+binding).
    // We merge their stage flags so one layout entry covers both stages.
    std::map<uint32_t /*set*/, std::map<uint32_t /*binding*/, ShaderResource>> setMap;

    for (const ShaderResource& res : shader->reflection.resources)
    {
        auto& existing = setMap[res.set][res.binding];
        if (existing.name.empty())
            existing = res;
        else
            existing.stages |= res.stages;
    }

    // Build the name ? resource lookup AFTER deduplication is complete,
    // so merged stage flags and correct sizes are captured.
    for (const auto& [set, bindingMap] : setMap)
        for (const auto& [binding, res] : bindingMap)
            shader->bindingsByName[res.name] = res;

    // Find the highest set index so we can size the layouts vector correctly.
    uint32_t maxSet = 0;
    for (const auto& [set, _] : setMap)
        maxSet = set > maxSet ? set : maxSet;

    shader->layouts.resize(maxSet + 1, nullptr);

    for (const auto& [set, bindingMap] : setMap)
    {
        GraphicsCore::ResourceLayoutDesc layoutDesc{};
        for (const auto& [binding, res] : bindingMap)
        {
            GraphicsCore::ResourceBinding rb{};
            rb.binding = res.binding;
            rb.type    = ToGCResourceType(res.type);
            rb.stage   = ToGCStage(res.stages);
            layoutDesc.bindings.push_back(rb);
        }
        shader->layouts[set] = renderer->CreateResourceLayout(layoutDesc);
    }

    // ---------------------------------------------------------------
    // Build the graphics pipeline
    // ---------------------------------------------------------------
    GraphicsCore::PipelineDesc pipelineDesc{};
    pipelineDesc.vertexShader   = shader->vertexShader;
    pipelineDesc.fragmentShader = shader->fragmentShader;

    GraphicsCore::VertexBinding vertexBinding{};
    vertexBinding.binding           = 0;
    vertexBinding.stride            = sizeof(Vertex);
    vertexBinding.inputRateInstance = false;
    pipelineDesc.vertexBindings.push_back(vertexBinding);

    GraphicsCore::VertexAttribute posAttr{};
    posAttr.location = 0;
    posAttr.binding  = 0;
    posAttr.format   = GraphicsCore::TextureFormat::RGB32F;
    posAttr.offset   = offsetof(Vertex, position);
    pipelineDesc.vertexAttributes.push_back(posAttr);

    GraphicsCore::VertexAttribute normAttr{};
    normAttr.location = 1;
    normAttr.binding  = 0;
    normAttr.format   = GraphicsCore::TextureFormat::RGB32F;
    normAttr.offset   = offsetof(Vertex, normal);
    pipelineDesc.vertexAttributes.push_back(normAttr);

    GraphicsCore::VertexAttribute uvAttr{};
    uvAttr.location = 2;
    uvAttr.binding  = 0;
    uvAttr.format   = GraphicsCore::TextureFormat::RG32F;
    uvAttr.offset   = offsetof(Vertex, uv);
    pipelineDesc.vertexAttributes.push_back(uvAttr);

    pipelineDesc.topology = GraphicsCore::PrimitiveTopology::TriangleList;

    pipelineDesc.rasterizerState.cullMode              = GraphicsCore::CullMode::None;
    pipelineDesc.rasterizerState.frontCounterClockwise = true;
    pipelineDesc.rasterizerState.depthClipEnable       = true;

    pipelineDesc.depthStencilState.depthTestEnable   = true;
    pipelineDesc.depthStencilState.depthWriteEnable  = true;
    pipelineDesc.depthStencilState.depthCompareOp    = GraphicsCore::CompareOp::Less;
    pipelineDesc.depthStencilState.stencilTestEnable = false;

    pipelineDesc.blendState.blendEnable = false;

    pipelineDesc.colorAttachmentCount      = 1;
    pipelineDesc.colorAttachmentFormats[0] = GraphicsCore::TextureFormat::RGBA8;
    pipelineDesc.depthStencilFormat        = GraphicsCore::TextureFormat::Depth32F;

    for (auto* layout : shader->layouts)
    {
        if (layout)
            pipelineDesc.layouts.push_back(layout);
    }

    shader->pipeline = renderer->CreatePipeline(pipelineDesc);

    // ---------------------------------------------------------------
    // Default sampler shared by all texture bindings on this shader
    // ---------------------------------------------------------------
    GraphicsCore::SamplerDesc samplerDesc{};
    samplerDesc.minFilter        = GraphicsCore::FilterMode::Linear;
    samplerDesc.magFilter        = GraphicsCore::FilterMode::Linear;
    samplerDesc.mipFilter        = GraphicsCore::FilterMode::Linear;
    samplerDesc.wrapU            = GraphicsCore::WrapMode::Repeat;
    samplerDesc.wrapV            = GraphicsCore::WrapMode::Repeat;
    samplerDesc.wrapW            = GraphicsCore::WrapMode::Repeat;
    samplerDesc.mipLodBias       = 0.0f;
    samplerDesc.minLod           = 0.0f;
    samplerDesc.maxLod           = 1.0f;
    samplerDesc.anisotropyEnable = false;
    samplerDesc.maxAnisotropy    = 1.0f;
    samplerDesc.compareEnable    = false;
    samplerDesc.compareOp        = GraphicsCore::CompareOp::Always;

    shader->sampler = renderer->CreateSampler(samplerDesc);

    return shader;
}