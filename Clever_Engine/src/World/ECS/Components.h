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

struct MaterialInstanceComponent
{
    std::shared_ptr<Material> material;
};