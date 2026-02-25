#include "Material.h"
#include <cstring>

void Material::InitializeFromShader()
{
    if (!m_shader) return;

    // Set default values for shader bindings
    for (const auto& binding : m_shader->metaData.bindings) {
        switch (binding.type) {
            case BindingType::UniformBuffer:
                // Material uniform buffer will be created in UploadToGPU
                break;
            
            case BindingType::CombinedImageSampler:
            case BindingType::SampledImage:
                // Initialize with default white texture if needed
                // You can set defaults here or require explicit SetTexture calls
                break;
            
            default:
                break;
        }
    }
}

void Material::SetFloat(const std::string& name, float value)
{
    MaterialParameter param;
    param.type = MaterialParameter::Type::Float;
    param.value = value;
    m_parameters[name] = param;
    m_isDirty = true;
}

void Material::SetVector(const std::string& name, const glm::vec3& value)
{
    MaterialParameter param;
    param.type = MaterialParameter::Type::Vector3;
    param.value = value;
    m_parameters[name] = param;
    m_isDirty = true;
}

void Material::SetVector4(const std::string& name, const glm::vec4& value)
{
    MaterialParameter param;
    param.type = MaterialParameter::Type::Vector4;
    param.value = value;
    m_parameters[name] = param;
    m_isDirty = true;
}

void Material::SetColor(const std::string& name, const glm::vec4& color)
{
    SetVector4(name, color);
}

void Material::SetTexture(const std::string& name, TextureHandle texture)
{
    MaterialParameter param;
    param.type = MaterialParameter::Type::Texture2D;
    param.value = texture;
    
    m_parameters[name] = param;
    
    // Also bind as a resource
    Resource res;
    res.data = texture;
    m_boundResources[name] = res;
    
    m_isDirty = true;
}

void Material::SetInt(const std::string& name, int value)
{
    MaterialParameter param;
    param.type = MaterialParameter::Type::Int;
    param.value = value;
    m_parameters[name] = param;
    m_isDirty = true;
}

const MaterialParameter* Material::GetParameter(const std::string& name) const
{
    auto it = m_parameters.find(name);
    if (it != m_parameters.end()) {
        return &it->second;
    }
    return nullptr;
}

void Material::UploadToGPU(GraphicsAPI& api)
{
    if (!m_gpuResourcesCreated) {
        CreateMaterialBuffer(api);
        m_gpuResourcesCreated = true;
    }
    
    UpdateMaterialBuffer(api);
    m_isDirty = false;
}

void Material::UpdateGPUResources(GraphicsAPI& api)
{
    if (m_isDirty && m_gpuResourcesCreated) {
        UpdateMaterialBuffer(api);
        m_isDirty = false;
    }
}

void Material::CreateMaterialBuffer(GraphicsAPI& api)
{
    // Find the material properties uniform buffer in shader metadata
    for (const auto& binding : m_shader->metaData.bindings) {
        if (binding.type == BindingType::UniformBuffer && 
            binding.name == "MaterialProperties") {
            
            // Calculate required buffer size from parameters
            size_t bufferSize = 256; // Default size, adjust based on your needs
            
            m_materialBuffer = api.CreateBuffer(bufferSize, BufferUsage::Uniform);
            
            Resource res;
            res.data = m_materialBuffer;
            m_boundResources["MaterialProperties"] = res;
            break;
        }
    }
}

void Material::UpdateMaterialBuffer(GraphicsAPI& api)
{
    if (m_materialBuffer.value == 0) return;
    
    // Pack material parameters into a buffer
    // This is simplified - in a real implementation, you'd need proper layout
    struct MaterialData {
        alignas(16) glm::vec4 baseColor = glm::vec4(1.0f);
        alignas(16) glm::vec4 emissiveColor = glm::vec4(0.0f);
        alignas(4)  float metallic = 0.0f;
        alignas(4)  float roughness = 0.5f;
        alignas(4)  float ao = 1.0f;
        alignas(4)  int useTexture = 0;
    };
    
    MaterialData data;
    
    // Fill from parameters
    if (auto* param = GetParameter("baseColor")) {
        if (std::holds_alternative<glm::vec4>(param->value)) {
            data.baseColor = std::get<glm::vec4>(param->value);
        }
    }
    
    if (auto* param = GetParameter("metallic")) {
        if (std::holds_alternative<float>(param->value)) {
            data.metallic = std::get<float>(param->value);
        }
    }
    
    if (auto* param = GetParameter("roughness")) {
        if (std::holds_alternative<float>(param->value)) {
            data.roughness = std::get<float>(param->value);
        }
    }
    
    api.UpdateBuffer(m_materialBuffer, &data, sizeof(MaterialData));
}

void Material::Bind(GraphicsAPI& api, uint32_t /*descriptorSet*/)
{
    for (const auto& [name, resource] : m_boundResources) {
        const auto* binding = m_shader->metaData.FindBinding(name);
        if (!binding) continue;

        if (resource.isBuffer()) {
            api.AssignBufferToShader(m_shader->fragmentHandle, name, resource.getBuffer());
        } else if (resource.isTexture()) {
            api.AssignTextureToShader(m_shader->fragmentHandle, name, resource.getTexture());
        } else if (resource.isSampler()) {
            api.AssignSamplerToShader(m_shader->fragmentHandle, name, resource.getSampler());
        }
    }
}