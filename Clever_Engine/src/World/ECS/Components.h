#pragma once
#include <string>
#include <glm.hpp>
#include <memory>

#define GLM_ENABLE_EXPERIMENTAL
#include <gtc/matrix_transform.hpp>
#include <gtx/quaternion.hpp>

#include "World/Assets/Shader.h"
#include "World/Assets/Material.h"
#include "World/Assets/Mesh.h"
#include "World/Assets/Texture.h"
#include "World/Assets/ShaderBinding.h"
#include "World/Assets/StorageBufferComponent.h"
#include <vector>


class Material;

struct CameraComponent
{
    glm::vec3 position{ 0.0f, 0.0f, 50.0f };
    glm::quat rotation = glm::identity<glm::quat>();
    CameraComponent()
    {
        glm::quat qYaw = glm::angleAxis(glm::radians(yaw), glm::vec3(0, 1, 0));
        glm::quat qPitch = glm::angleAxis(glm::radians(pitch), glm::vec3(1, 0, 0));
        rotation = qYaw * qPitch;
    };
    float yaw = 0.0f;
    float pitch = 0.0f;
    float moveSpeed = 150.0f;
    float sensitivity = 0.4f;

    float fov = 90.0f;
    float aspectRatio = (float)960 / (float)540;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;

    bool isPerspective = true;
    float orthoSize = 10.0f;

    glm::vec3 GetForward() const { return rotation * glm::vec3(0.0f, 0.0f, -1.0f); }
    glm::vec3 GetUp() const { return rotation * glm::vec3(0.0f, 1.0f, 0.0f); }
    glm::vec3 GetRight() const { return rotation * glm::vec3(1.0f, 0.0f, 0.0f); }
};

struct Transform
{
    glm::vec3 position{ 0.0f, 0.0f, 0.0f };
    glm::quat rotation = glm::identity<glm::quat>();
    glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
    glm::mat4 GetModelMatrix() const
    {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
        model *= glm::toMat4(rotation);
        model = glm::scale(model, scale);
        return model;
    }
};

struct MeshComponent
{
	std::shared_ptr<Mesh> mesh;
};

struct MaterialComponent
{
    std::shared_ptr<Material> material;
};

struct ShaderComponent
{
    std::shared_ptr<Shader> shader;
};

struct TextureComponent
{
    std::shared_ptr<Texture> texture;
};

// Flexible per-entity shader data. Declare buffer and texture bindings by their
// GLSL name; the renderer resolves them against the shader's reflection at draw time.
// Standard data (mvp, model matrix) is always provided automatically.
// Set instanceCount > 1 to use instanced rendering via DrawIndexed instanceCount.
struct ShaderDataComponent
{
    std::vector<BufferBinding>  buffers;
    std::vector<TextureBinding> textures;
    uint32_t                    instanceCount = 1;

    ShaderDataComponent& BindBuffer(const std::string& name, GraphicsCore::IBuffer* buffer,
                                    size_t offset = 0, size_t range = 0)
    {
        buffers.push_back({ name, buffer, offset, range });
        return *this;
    }

    ShaderDataComponent& BindTexture(const std::string& name, GraphicsCore::ITexture* texture,
                                     GraphicsCore::ISampler* sampler = nullptr)
    {
        textures.push_back({ name, texture, sampler });
        return *this;
    }

    ShaderDataComponent& SetInstanceCount(uint32_t count)
    {
        instanceCount = count;
        return *this;
    }
};

struct MaterialInstanceComponent
{
    std::shared_ptr<Material> material;
};