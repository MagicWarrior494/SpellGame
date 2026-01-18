#pragma once
#include <string>
#include <memory>
#include <vector>
#include <cstdint>
#include <GLFW/glfw3.h>

#include "Context/VulkanContext.h"
#include "Event/EventController.h"
#include "Scene/SceneController.h" // Assuming this is your SceneController path

// Engine Type Definitions
using SceneID = uint8_t;
using WindowID = uint8_t;

class Window : public IInputLayer{
public:
    Window(std::shared_ptr<Vulkan::VulkanContext> vulkanContext,
        std::string title, int width, int height, int posx, int posy);
    ~Window() = default;

    // Standard Logic
    bool IsWindowStillValid();
    void InitWindow();
    void CloseWindow();
    void Update();
    void Render();

    void OnInput(InputEvent& event);
    int GetZIndex() const;

    // Render Surface Management
    uint8_t CreateNewScene(uint32_t width, uint32_t height, int posx = 0, int posy = 0);
	void MoveScene(SceneID sceneID, int newX, int newY);
    void ResizeScene(SceneID sceneID, int newX, int newY);
    void AddChildRenderSurface(uint8_t renderSurfaceID);
    int GetRenderSurfaceCount() const { return static_cast<int>(m_ChildrenRenderSurfaces.size()); }

    // --- Getters ---
    WindowID GetWindowID() const { return m_WindowID; }
    GLFWwindow* GetGLFWWindowPtr() const { return m_pGLFWWindow; }
    std::shared_ptr<Vulkan::Window> GetVulkanWindow() { return m_VulkanWindow; }
    std::vector<uint8_t>& GetChildrenRenderSurfaces() { return m_ChildrenRenderSurfaces; }
	glm::vec2 GetWindowSize() const { return glm::vec2(static_cast<float>(m_Width), static_cast<float>(m_Height)); }
	glm::vec2 GetWindowPosition() const { return glm::vec2(static_cast<float>(m_PosX), static_cast<float>(m_PosY)); }

    // Controller Accessors
    EventController& GetEventController() { return *m_EventController; }
    SceneController& GetSceneController() { return *m_SceneController; }

    // Callbacks
    void OnResize(int width, int height);

private:
    void InitCallbacks();

    WindowID m_WindowID = 0;
    std::string m_Title;
    int m_Width;
    int m_Height;
    int m_PosX;
    int m_PosY;

    GLFWwindow* m_pGLFWWindow = nullptr;
    std::shared_ptr<Vulkan::VulkanContext> m_VulkanContext;
    std::shared_ptr<Vulkan::Window> m_VulkanWindow;

    std::vector<uint8_t> m_ChildrenRenderSurfaces{};
    Vulkan::SurfaceFlags m_DefaultVulkanWindowFlags = Vulkan::SurfaceFlags::Resizeable | Vulkan::SurfaceFlags::Fullscreenable;

    // Controllers Owned by the Window
    std::unique_ptr<EventController> m_EventController;
    std::unique_ptr<SceneController> m_SceneController;
};