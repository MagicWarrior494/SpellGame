#pragma once

#include "Asset.h"
#include "ShaderReflected.h"
#include "IRenderer.h"

#include <unordered_map>
#include <string>
#include <variant>
#include <vector>
#include <glm.hpp>

struct MaterialParameter
{
    enum class Type { Float, Vector2, Vector3, Vector4, Color, Int };
    Type type;
    std::variant<float, glm::vec2, glm::vec3, glm::vec4, int> value;
};

struct MaterialUniformBlock
{
    std::string blockName;
    uint32_t    set     = 0;
    uint32_t    binding = 0;
    size_t      size    = 0;
    ShaderStage stages  = ShaderStage::None;

    GraphicsCore::IBuffer* buffer = nullptr;

    std::vector<ReflectedMember> members;
};

class Material : public Asset
{
public:
    void SetFloat(const std::string& name, float value);
    void SetVector3(const std::string& name, const glm::vec3& value);
    void SetVector4(const std::string& name, const glm::vec4& value);
    void SetInt(const std::string& name, int value);

    const MaterialParameter* GetParameter(const std::string& name) const;

    void BuildFromShaderReflection(GraphicsCore::IRenderer* renderer,
                                   const ShaderReflection&  reflection);
    void UploadToGPU(GraphicsCore::IRenderer* renderer);

    bool IsDirty() const { return m_isDirty; }

private:
    std::unordered_map<std::string, MaterialParameter>    m_parameters;
    std::unordered_map<std::string, MaterialUniformBlock> m_uniformBlocks;
    ShaderReflection                                       m_reflection;
    bool                                                   m_isDirty = true;
};