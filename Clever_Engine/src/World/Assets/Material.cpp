#include "Material.h"

//
// ============================================================
//  Parameter Setters
// ============================================================
//

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
    if (it != m_parameters.end())
        return &it->second;

    return nullptr;
}

//
// ============================================================
//  Build Uniform Buffers From Reflection
// ============================================================
//

void Material::BuildFromShaderReflection(GraphicsAPI* api, const ShaderReflection& reflection)
{
    m_reflection = reflection;
    m_uniformBlocks.clear();

    for (const auto& resource : reflection.resources)
    {
        if (resource.type != ShaderResourceType::UniformBuffer)
            continue;

        MaterialUniformBlock block{};

        block.blockName = resource.name;
        block.set = resource.set;
        block.binding = resource.binding;
        block.size = resource.size;
        block.stages = resource.stages;
        block.members = resource.members;

        // Create GPU uniform buffer via API
		block.buffer = api->CreateBuffer(block.size, BufferUsage::Uniform);

        m_uniformBlocks[resource.name] = std::move(block);
    }

    m_isDirty = true;
}

//
// ============================================================
//  Upload CPU Params → GPU Buffer
// ============================================================
//

void Material::UploadToGPU(GraphicsAPI* api)
{
    if (!m_isDirty)
        return;

    for (auto& [blockName, block] : m_uniformBlocks)
    {
        // Allocate temporary CPU memory for this block
        std::vector<uint8_t> tempBuffer(block.size);
        memset(tempBuffer.data(), 0, block.size);

        // Fill block with material parameters
        for (const auto& member : block.members)
        {
            auto it = m_parameters.find(member.name);
            if (it == m_parameters.end())
                continue;

            uint8_t* dst =
                tempBuffer.data() + member.offset;

            std::visit([&](auto&& value)
                {
                    using T = std::decay_t<decltype(value)>;
                    *reinterpret_cast<T*>(dst) = value;

                }, it->second.value);
        }
        api->UpdateBuffer(block.buffer,
            tempBuffer.data(),
            block.size);
    }

    m_isDirty = false;
}