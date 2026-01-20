#pragma once
#include <cstdint>
#include <iostream>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp> // Required for glm::lookAt and glm::perspective

#include "World/WorldController.h"
#include "Event/EventController.h"
#include "Context/VulkanContext.h"

#include "Render/BufferManager.h"
#include "Objects/Vertex.h"
#include "World/ECS/Registry.h"
#include "World/ECS/Components.h"

struct SceneCreationInfo {
    uint8_t windowID;
	uint8_t sceneID;
    uint32_t width;
    uint32_t height;
    int posx;
    int posy;
    int zIndex = 0; // Default Z-Index for input priority
};

class Scene : public IInputLayer
{
public:
    Scene(SceneCreationInfo& creationInfo, std::shared_ptr<Vulkan::VulkanContext> vulkanContext, Registry& registry)
		: m_CreationInfo(creationInfo), m_VulkanContext(vulkanContext), m_Registry(registry)
    {
       
    }

    virtual ~Scene() = default;

    // Standard engine update
    virtual void Update() = 0;

    // --- IInputLayer Interface Implementation ---
    // Derived scenes override these to handle input
    virtual void OnInput(InputEvent& event) override {}

    virtual int GetZIndex() const override { return m_CreationInfo.zIndex; }

    // --- Getters ---
    SceneCreationInfo GetCreationInfo() const { return m_CreationInfo; }


	// --- Setters ---
	void SetZIndex(int zIndex) { m_CreationInfo.zIndex = zIndex; }
	void SetPosition(int x, int y) { m_CreationInfo.posx = x; m_CreationInfo.posy = y; }
	void SetSize(uint32_t width, uint32_t height) { m_CreationInfo.width = width; m_CreationInfo.height = height; }

protected:
    SceneCreationInfo m_CreationInfo;
    std::shared_ptr<Vulkan::VulkanContext> m_VulkanContext;
    Registry& m_Registry;
};

// --- Specialized Scene Types ---

class CameraScene : public Scene
{
public:
    CameraScene(SceneCreationInfo& creationInfo, std::shared_ptr<Vulkan::VulkanContext> vulkanContext, Registry& registry, uint32_t entityID)
        : Scene(creationInfo, vulkanContext, registry), m_CameraEntityID(entityID)
    {
        Vulkan::Window& window = *m_VulkanContext->GetWindow(m_CreationInfo.windowID).get();

        Vulkan::VulkanBuffer& buffer = window.vulkanSurface.cameraBuffer;

        size_t minAligment = vulkanContext->GetPhysicalDeviceMinAlignment();

        m_UniformBufferManager = std::make_unique<UniformBufferManager>(
            buffer,
            minAligment,
            sizeof(SceneShaderData)
        );
        m_CameraSlot = m_UniformBufferManager->AllocateSlot();
        window.vulkanSurface.sceneIDToCameraBufferSlot.insert({ creationInfo.sceneID, m_CameraSlot });
    }

    virtual void Update() override
    {
        // 1. Get components from the registry
        auto& camera = m_Registry.GetComponent<Camera>(m_CameraEntityID);

        // 2. Prepare the Data Struct
        SceneShaderData gpuData{};

        // 3. Calculate View Matrix (Where the camera is looking)
        // transform.forward should be a unit vector (e.g., 0,0,-1)
        gpuData.view = glm::lookAt(
            camera.position,
            camera.position + camera.GetForward(),
            glm::vec3(0.0f, 1.0f, 0.0f) // World Up
        );

        // 4. Calculate Projection Matrix (How the camera sees)
        gpuData.proj = glm::perspective(
            glm::radians(camera.fov),
            camera.aspectRatio,
            camera.nearPlane,
            camera.farPlane
        );

        // 5. Vulkan Correction: Flip the Y-axis
        // GLM is right-handed (Y-up), Vulkan clip space is Y-down.
        gpuData.proj[1][1] *= -1;

        // 6. Update the GPU Buffer using the manager
        // m_CameraSlot should be the index you got from AllocateSlot()
        m_UniformBufferManager->UpdateSlot(m_CameraSlot, &gpuData);
    }

    // Example of handling specific input in a scene
    virtual void OnInput(InputEvent& event) override
    {
        if (event.type == InputEvent::Type::Key &&
            (event.action == Input::Action::PRESS || event.action == Input::Action::REPEAT))
        {
            auto& camera = m_Registry.GetComponent<Camera>(m_CameraEntityID);

            glm::vec3 forward = camera.GetForward();
            glm::vec3 right = camera.GetRight();

            float speed = camera.moveSpeed * m_VulkanContext->GetDeltaTime();

            //std::cout << speed << " " << m_VulkanContext->GetDeltaTime() << std::endl;

            if (event.code == Input::Keyboard::KEY_W) camera.position += forward * speed;
            if (event.code == Input::Keyboard::KEY_S) camera.position -= forward * speed;
            if (event.code == Input::Keyboard::KEY_A) camera.position -= right * speed;
            if (event.code == Input::Keyboard::KEY_D) camera.position += right * speed;

            event.Consume();
        }
        if (event.type == InputEvent::Type::MouseMove)
        {
            auto& camera = m_Registry.GetComponent<Camera>(m_CameraEntityID);

            // 1. Update Euler angles based on mouse movement
            // deltaX: positive means mouse moved right
            // deltaY: positive means mouse moved down
            camera.yaw -= event.deltaX * camera.sensitivity;
            camera.pitch -= event.deltaY * camera.sensitivity;

            // 2. Clamp the pitch to prevent the "Backflip" bug
            // Looking straight up or down causes the View Matrix to collapse (Gimbal Lock)
            camera.pitch = glm::clamp(camera.pitch, -89.0f, 89.0f);

            // 3. Rebuild the Rotation Quaternion
            // We use the same order as your successful A/D test
            glm::quat qYaw = glm::angleAxis(glm::radians(camera.yaw), glm::vec3(0, 1, 0));
            glm::quat qPitch = glm::angleAxis(glm::radians(camera.pitch), glm::vec3(1, 0, 0));

            camera.rotation = qYaw * qPitch;
            camera.rotation = glm::normalize(camera.rotation);

            event.Consume();
        }
    }

private:
    uint32_t m_CameraEntityID;

    //Slot in the UniformBuffer
    uint32_t m_CameraSlot;
    std::unique_ptr<UniformBufferManager> m_UniformBufferManager;
};

class UIScene : public Scene
{
public:
    UIScene(SceneCreationInfo& creationInfo, std::shared_ptr<Vulkan::VulkanContext> vulkanContext, Registry& registry)
        : Scene(creationInfo, vulkanContext, registry)
    {
        m_CreationInfo.zIndex = 100;
    }

    virtual void Update() override
    {
        // UI animation/logic here
    }

    virtual void OnInput(InputEvent& event) override
    {
        if (event.type == InputEvent::Type::MouseButton)
        {
            // if (isOverButton) event.Consume();
        }
    }
};