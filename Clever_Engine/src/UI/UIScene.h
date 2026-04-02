#pragma once
#include <cstdint>
#include <vector>
#include <memory>

#include "IRenderer.h"
#include "World/AssetManager.h"
#include "Render/Window/Scene.h"
#include "Render/Window/ISceneLayer.h"
#include "UI/UIWidget.h"

#ifndef CLEVER_ENGINE_API
    #ifdef _WIN32
        #define CLEVER_ENGINE_API __declspec(dllexport)
    #else
        #define CLEVER_ENGINE_API
    #endif
#endif

class Window;

// ---------------------------------------------------------------
// UIScene — a Scene subtype that renders a flat panel of UI
// widgets. It uses its own pipeline (no vertex buffers, no depth
// test, alpha blending on) and draws each widget as a colored
// quad via push constants only.
//
// Usage:
//   UIScene& ui = engine.CreateUIScene(window, 400, 600, 880, 60);
//   ButtonWidget& btn = ui.AddButton({ {20,20}, {160,40}, {0.2,0.5,0.9,1} });
// ---------------------------------------------------------------
class CLEVER_ENGINE_API UIScene : public IInputLayer, public ISceneLayer
{
public:
    UIScene(GraphicsCore::IRenderer* renderer,
            AssetManager*            assetManager,
            Window*                  window,
            const SceneDesc&         desc);
    ~UIScene();

    // Scene interface used by Window
    void Update()  override;
    void Render()  override;
    void Resize(uint32_t width, uint32_t height) override;
    void AttachToWindow(Window& window);

    GraphicsCore::ITexture* GetColorTarget() const override { return m_colorTarget; }
    const SceneDesc&        GetDesc()        const override { return m_desc; }
    EventController&        GetEventController()   { return *m_eventController; }

    // IInputLayer
    void OnInput(InputEvent& event) override;
    int  GetZIndex() const override { return m_desc.zIndex; }
    void ResetInputState() override;
    void SetPosition(int x, int y) override { m_desc.posX = x; m_desc.posY = y; }
    void SetWindow(Window* window)  override { m_window = window; }

    // ---------------------------------------------------------------
    // Widget registration — returns a reference into the internal
    // list so the caller can update the widget each frame.
    // ---------------------------------------------------------------
    ButtonWidget& AddButton(const ButtonWidget& widget = {});

    // Direct access to all buttons for application-side iteration
    std::vector<ButtonWidget>& GetButtons() { return m_buttons; }

private:
    void BuildPipeline();
    void DestroyPipeline();
    void RenderWidget(const UIWidget& widget, const glm::vec4& color);

    SceneDesc m_desc;

    GraphicsCore::IRenderer*    m_renderer    = nullptr;
    AssetManager*               m_assetManager = nullptr;
    Window*                     m_window       = nullptr;

    GraphicsCore::ITexture*     m_colorTarget  = nullptr;
    GraphicsCore::ICommandList* m_commandList  = nullptr;

    // UI pipeline — no vertex buffers, no descriptor sets
    GraphicsCore::IShader*   m_vertShader = nullptr;
    GraphicsCore::IShader*   m_fragShader = nullptr;
    GraphicsCore::IPipeline* m_pipeline   = nullptr;

    std::unique_ptr<EventController> m_eventController;

    std::vector<ButtonWidget> m_buttons;

    // Last known cursor position in scene-local pixel space
    double m_cursorX = -1.0;
    double m_cursorY = -1.0;
};
