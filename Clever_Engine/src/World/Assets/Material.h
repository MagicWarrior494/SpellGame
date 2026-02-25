#pragma once

#include "Asset.h"
#include "Shader.h"
#include "Resource.h"
#include <map>
#include <string>
#include <memory>
#include <variant>
#include <glm.hpp>
#include <stdexcept>

// Material parameter types
struct MaterialParameter
{
    enum class Type {
        Float,
        Vector2,
        Vector3,
        Vector4,
        Texture2D,
        Color,
        Int
    };

    Type type;
    std::variant<float, glm::vec2, glm::vec3, glm::vec4, TextureHandle, int> value;
};

class Material : public Asset
{
public:
    Material(std::shared_ptr<Shader> shader)
        : m_shader(shader)
    {
        if (!shader) {
            throw std::runtime_error("Cannot create Material with null Shader");
        }
        
        // Initialize resource bindings based on shader metadata
        InitializeFromShader();
    }

    // Material parameter setters
    void SetFloat(const std::string& name, float value);
    void SetVector(const std::string& name, const glm::vec3& value);
    void SetVector4(const std::string& name, const glm::vec4& value);
    void SetColor(const std::string& name, const glm::vec4& color);
    void SetTexture(const std::string& name, TextureHandle texture);
    void SetInt(const std::string& name, int value);

    // Get parameters
    const MaterialParameter* GetParameter(const std::string& name) const;

    // Bind material resources to GPU (called before drawing)
    void Bind(GraphicsAPI& api, uint32_t descriptorSet = 1);

    // Upload material data to GPU buffers
    void UploadToGPU(GraphicsAPI& api) override;
    
    // Update GPU resources if parameters changed
    void UpdateGPUResources(GraphicsAPI& api);

    std::shared_ptr<Shader> GetShader() const { return m_shader; }

    // Check if material needs GPU update
    bool IsDirty() const { return m_isDirty; }

private:
    void InitializeFromShader();
    void CreateMaterialBuffer(GraphicsAPI& api);
    void UpdateMaterialBuffer(GraphicsAPI& api);

private:
    std::shared_ptr<Shader> m_shader;
    
    // Material parameters (CPU-side)
    std::map<std::string, MaterialParameter> m_parameters;
    
    // GPU resources bound to this material (textures, buffers)
    std::map<std::string, Resource> m_boundResources;
    
    // Material properties buffer (uniform buffer for material data)
    BufferHandle m_materialBuffer;
    
    bool m_isDirty = true;
    bool m_gpuResourcesCreated = false;
};