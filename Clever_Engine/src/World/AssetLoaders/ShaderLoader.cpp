#include "ShaderLoader.h"
#include "World/Assets/Shader.h"
#include "World/Assets/Vertex.h"

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
    vertDesc.stage = GraphicsCore::ShaderStage::Vertex;
    vertDesc.bytecode = vertCode.data();
    vertDesc.bytecodeSize = vertCode.size() * sizeof(uint32_t);
    vertDesc.entryPoint = "main";

    GraphicsCore::ShaderDesc fragDesc{};
    fragDesc.stage = GraphicsCore::ShaderStage::Fragment;
    fragDesc.bytecode = fragCode.data();
    fragDesc.bytecodeSize = fragCode.size() * sizeof(uint32_t);
    fragDesc.entryPoint = "main";

    auto shader = std::make_shared<Shader>();
    shader->vertexShader   = renderer->CreateShader(vertDesc);
    shader->fragmentShader = renderer->CreateShader(fragDesc);
    shader->reflection     = ReflectCombinedShaders(vertCode, fragCode);

    // Build the graphics pipeline for this shader
    GraphicsCore::PipelineDesc pipelineDesc{};
    pipelineDesc.vertexShader   = shader->vertexShader;
    pipelineDesc.fragmentShader = shader->fragmentShader;

    // Vertex layout: binding 0, stride = sizeof(Vertex), per-vertex
    GraphicsCore::VertexBinding vertexBinding{};
    vertexBinding.binding          = 0;
    vertexBinding.stride           = sizeof(Vertex);
    vertexBinding.inputRateInstance = false;
    pipelineDesc.vertexBindings.push_back(vertexBinding);

    // Vertex attributes: position (vec3), normal (vec3), uv (vec2)
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

    pipelineDesc.depthStencilState.depthTestEnable  = true;
    pipelineDesc.depthStencilState.depthWriteEnable = true;
    pipelineDesc.depthStencilState.depthCompareOp   = GraphicsCore::CompareOp::Less;
    pipelineDesc.depthStencilState.stencilTestEnable = false;

    pipelineDesc.blendState.blendEnable = false;

    pipelineDesc.colorAttachmentCount      = 1;
    pipelineDesc.colorAttachmentFormats[0] = GraphicsCore::TextureFormat::RGBA8;
    pipelineDesc.depthStencilFormat        = GraphicsCore::TextureFormat::Depth32F;

    // Descriptor set layout: set 0, binding 0 = combined image sampler (fragment)
    GraphicsCore::ResourceBinding texBinding{};
    texBinding.binding = 0;
    texBinding.type    = GraphicsCore::ResourceType::Texture;
    texBinding.stage   = GraphicsCore::ShaderStage::Fragment;

    GraphicsCore::ResourceLayoutDesc layoutDesc{};
    layoutDesc.bindings.push_back(texBinding);

    shader->textureLayout = renderer->CreateResourceLayout(layoutDesc);
    pipelineDesc.layouts.push_back(shader->textureLayout);

    shader->pipeline = renderer->CreatePipeline(pipelineDesc);

    // Default linear sampler shared by all draw calls using this shader
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