#include "Material.h"
#include <cstring>
#include <vector>

void Material::SetFloat(const std::string& name, float value)
{
    m_parameters[name] = { MaterialParameter::Type::Float, value };
    m_isDirty = true;
}

void Material::SetVector3(const std::string& name, const glm::vec3& value)
{
    m_parameters[name] = { MaterialParameter::Type::Vector3, value };
    m_isDirty = true;
}

void Material::SetVector4(const std::string& name, const glm::vec4& value)
{
    m_parameters[name] = { MaterialParameter::Type::Vector4, value };
    m_isDirty = true;
}

void Material::SetInt(const std::string& name, int value)
{
    m_parameters[name] = { MaterialParameter::Type::Int, value };
    m_isDirty = true;
}

const MaterialParameter* Material::GetParameter(const std::string& name) const
{
    auto it = m_parameters.find(name);
    return it != m_parameters.end() ? &it->second : nullptr;
}

void Material::BuildFromShaderReflection(GraphicsCore::IRenderer* renderer,
                                          const ShaderReflection&  reflection)
{
    m_reflection = reflection;
    m_uniformBlocks.clear();

    for (const auto& resource : reflection.resources)
    {
        if (resource.type != ShaderResourceType::UniformBuffer)
            continue;

        MaterialUniformBlock block{};
        block.blockName = resource.name;
        block.set       = resource.set;
        block.binding   = resource.binding;
        block.size      = resource.size;
        block.stages    = resource.stages;
        block.members   = resource.members;

        GraphicsCore::BufferDesc desc{};
        desc.size          = block.size;
        desc.usage         = GraphicsCore::BufferUsage::Uniform;
        desc.cpuAccessible = true;
        block.buffer = renderer->CreateBuffer(desc);

        m_uniformBlocks[resource.name] = std::move(block);
    }

    m_isDirty = true;
}

void Material::UploadToGPU(GraphicsCore::IRenderer* renderer)
{
    if (!m_isDirty)
        return;

    for (auto& [blockName, block] : m_uniformBlocks)
    {
        std::vector<uint8_t> tempBuffer(block.size, 0);

        for (const auto& member : block.members)
        {
            auto it = m_parameters.find(member.name);
            if (it == m_parameters.end())
                continue;

            uint8_t* dst = tempBuffer.data() + member.offset;
            std::visit([&](auto&& value)
            {
                using T = std::decay_t<decltype(value)>;
                memcpy(dst, &value, sizeof(T));
            }, it->second.value);
        }

        void* mapped = renderer->MapBuffer(block.buffer);
        memcpy(mapped, tempBuffer.data(), block.size);
        renderer->UnmapBuffer(block.buffer);
    }

    m_isDirty = false;
}