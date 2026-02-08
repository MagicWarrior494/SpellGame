#pragma once
#include <string>
#include <memory>
#include <vector>
#include <cstdint>
#include <GLFW/glfw3.h>

#include "Event/EventController.h"
#include "Scene/SceneController.h" // Assuming this is your SceneController path
#include "World/ECS/Registry.h"
#include "WindowControls.h"

class Window : public IInputLayer{
public:
    Window(std::string title, int width, int height, int posx = 0, int posy = 0);
    ~Window() = default;

    // Standard Logic
    bool IsWindowStillValid();
    void CloseWindow();
    void Update();
    void Render();

    void OnInput(InputEvent& event);
    int GetZIndex() const;

    Scene& CreateNewScene(uint32_t width, uint32_t height, int posx = 0, int posy = 0);

    int GetSceneCount() const { return m_Scenes.size(); };

    // --- Getters ---
    int GetWindowID() const { return m_WindowID; }
    GLFWwindow* GetGLFWWindowPtr() const { return m_pGLFWWindow; }
    std::vector<std::unique_ptr<Scene>>& getScenes() { return m_Scenes; }
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
    std::vector<std::unique_ptr<Scene>> m_Scenes{};

    // Controllers Owned by the Window
    std::unique_ptr<EventController> m_EventController;
};