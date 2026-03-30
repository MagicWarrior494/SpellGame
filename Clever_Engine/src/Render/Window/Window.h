#pragma once
#include <string>
#include <memory>
#include <vector>
#include <cstdint>

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

    void OnResize(uint32_t width, uint32_t height);

    GraphicsCore::IWindow* GetIWindow()        const { return m_iWindow; }
    EventController&       GetEventController()      { return *m_eventController; }
    uint32_t               GetWidth()          const { return m_width; }
    uint32_t               GetHeight()         const { return m_height; }

private:
GraphicsCore::IRenderer* m_renderer    = nullptr;
GraphicsCore::IWindow*   m_iWindow     = nullptr;

std::string m_title;
uint32_t    m_width  = 0;
uint32_t    m_height = 0;

GraphicsCore::ICommandList* m_blitCommandList = nullptr;

std::unique_ptr<EventController> m_eventController;
};