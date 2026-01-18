#pragma once
#include <glm.hpp>
#include <cstdint>
#include <iostream>

#include "World/WorldController.h"
#include "Event/EventController.h"
#include "Context/VulkanContext.h"

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
    Scene(SceneCreationInfo& creationInfo, std::shared_ptr<Vulkan::VulkanContext> vulkanContext)
		: m_CreationInfo(creationInfo), m_VulkanContext(vulkanContext)
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
};

// --- Specialized Scene Types ---

class CameraScene : public Scene
{
public:
    CameraScene(SceneCreationInfo& creationInfo, std::shared_ptr<Vulkan::VulkanContext> vulkanContext, uint32_t entityID)
        : Scene(creationInfo, vulkanContext), m_CameraEntityID(entityID)
    {
    }

    virtual void Update() override
    {
        // Camera logic here
    }

    // Example of handling specific input in a scene
    virtual void OnInput(InputEvent& event) override
    {
        
    }

private:
    uint32_t m_CameraEntityID;
};

class UIScene : public Scene
{
public:
    UIScene(SceneCreationInfo& creationInfo, std::shared_ptr<Vulkan::VulkanContext> vulkanContext)
        : Scene(creationInfo, vulkanContext)
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