#pragma once
#include <string>
#include <memory>
#include <vector>
#include <cstdint>
#include <functional>
#include <chrono>

#include "IRenderer.h"
#include "Event/EventController.h"
#include "WindowControls.h"

#ifndef CLEVER_ENGINE_API
    #ifdef _WIN32
        #define CLEVER_ENGINE_API __declspec(dllexport)
    #else
        #define CLEVER_ENGINE_API
    #endif
#endif

class Scene;

class CLEVER_ENGINE_API Window : public IInputLayer, public GraphicsCore::IWindowEventSink
{
public:
    Window(GraphicsCore::IRenderer* renderer,
           const std::string&       title,
           uint32_t                 width,
           uint32_t                 height,
           int                      posX = 0,
           int                      posY = 0);

    ~Window();

    bool IsAlive();
    void Update();
    void Render(const std::vector<Scene*>& scenes);

    void OnInput(InputEvent& /*event*/) override {}
    int  GetZIndex() const override { return 0; }

    // IWindowEventSink
    void OnKey(int glfwKey, int scancode, int action, int mods) override;
    void OnMouseButton(int button, int action, double x, double y, int mods) override;
    void OnMouseMove(double x, double y) override;
    void OnMouseScroll(double xoffset, double yoffset) override;
    void OnFocus(bool focused) override;
    void OnClose() override;

    void OnResize(uint32_t width, uint32_t height);

    void SetCloseCallback(std::function<void()> callback) { m_closeCallback = callback; }

    GraphicsCore::IWindow* GetIWindow()        const { return m_iWindow; }
    EventController&       GetEventController()      { return *m_eventController; }
    uint32_t               GetWidth()          const { return m_width; }
    uint32_t               GetHeight()         const { return m_height; }

    // Scene focus: only the focused scene receives keyboard and mouse-move events.
    // A scene gains focus when the user right-clicks inside its bounds.
    void          RegisterScene(Scene* scene);
    void          UnregisterScene(Scene* scene);
    Scene*        GetFocusedScene() const { return m_focusedScene; }

private:
    // Returns the topmost scene whose rectangle contains (x, y), or nullptr.
    Scene* SceneAtPoint(double x, double y) const;

    GraphicsCore::IRenderer* m_renderer    = nullptr;
    GraphicsCore::IWindow*   m_iWindow     = nullptr;

    std::string m_title;
    uint32_t    m_width  = 0;
    uint32_t    m_height = 0;

    // FPS counter
    std::chrono::steady_clock::time_point m_fpsLastTime  = std::chrono::steady_clock::now();
    uint32_t                              m_fpsFrameCount = 0;

    GraphicsCore::ICommandList* m_blitCommandList = nullptr;

    std::unique_ptr<EventController> m_eventController;
    std::function<void()>            m_closeCallback;

    // Scenes registered with this window for focus routing
    std::vector<Scene*> m_scenes;
    Scene*              m_focusedScene = nullptr;
};