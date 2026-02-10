#pragma once
#include <string>
#include <memory>
#include <vector>
#include <cstdint>
#include <GLFW/glfw3.h>
#include "Render/Graphics/GraphicsAPI.h"

#include "Event/EventController.h"
#include "World/ECS/Registry.h"
#include "WindowControls.h"

class Window : public IInputLayer{
public:
    Window(GraphicsAPI* graphicsAPI, std::string title, int width, int height, int posx = 0, int posy = 0);
    ~Window() = default;

    // Standard Logic
    bool ShouldWindowClose();
    void CloseWindow();
    void Update();
    void Render();

    void OnInput(InputEvent& event);
    int GetZIndex() const;

    // --- Getters ---
    int GetWindowID() const { return m_WindowID; }
    GLFWwindow* GetGLFWWindowPtr() const { return m_pGLFWWindow; }
	glm::vec2 GetWindowSize() const { return glm::vec2(static_cast<float>(m_Width), static_cast<float>(m_Height)); }
	glm::vec2 GetWindowPosition() const { return glm::vec2(static_cast<float>(m_PosX), static_cast<float>(m_PosY)); }

    // Controller Accessors
    EventController& GetEventController() { return *m_EventController; }

    // Callbacks
    void OnResize(int width, int height);

private:
    void InitCallbacks();

    int m_WindowID = 0;
    std::string m_Title;
    int m_Width;
    int m_Height;
    int m_PosX;
    int m_PosY;

    GLFWwindow* m_pGLFWWindow = nullptr;

    // Controllers Owned by the Window
    std::unique_ptr<EventController> m_EventController;

	uint32_t m_GraphicsWindowID = 0; // ID for the GraphicsAPI to reference this window
	GraphicsAPI* m_GraphicsAPI; // Assume this is set externally, or you can initialize it here
};