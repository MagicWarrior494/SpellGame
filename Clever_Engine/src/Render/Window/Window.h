#pragma once
#include <string>
#include <memory>
#include <vector>
#include <cstdint>
#include <functional>
#include <chrono>

#include "IRenderer.h"
#include "Event/EventController.h"
#include "Render/Window/ISceneLayer.h"
#include "WindowControls.h"

#ifndef CLEVER_ENGINE_API
    #ifdef _WIN32
        #define CLEVER_ENGINE_API __declspec(dllexport)
    #else
        #define CLEVER_ENGINE_API
    #endif
#endif

class Scene;
class UIScene;

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
    void Render(const std::vector<ISceneLayer*>& scenes);

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

    // Screen-space position of the OS window (in pixels from top-left of desktop)
    void GetScreenPosition(int& x, int& y) const;
    void SetScreenPosition(int x, int y);
    void SetSize(uint32_t width, uint32_t height);

    // Called by Engine so it can intercept a layer being dragged outside this window.
    // Signature: (layer, screenCursorX, screenCursorY)
    using LayerDetachCallback = std::function<void(ISceneLayer*, int, int)>;
    void SetLayerDetachCallback(LayerDetachCallback cb) { m_layerDetachCallback = std::move(cb); }

    // Called by Engine when this window (single-layer) is released after a window-drag,
    // so Engine can dock the layer into another window if the cursor is over one.
    // Signature: (layer, screenCursorX, screenCursorY)
    using WindowDockCallback = std::function<void(ISceneLayer*, int, int)>;
    void SetWindowDockCallback(WindowDockCallback cb) { m_windowDockCallback = std::move(cb); }

    GraphicsCore::IWindow* GetIWindow()        const { return m_iWindow; }
    EventController&       GetEventController()      { return *m_eventController; }
    uint32_t               GetWidth()          const { return m_width; }
    uint32_t               GetHeight()         const { return m_height; }

    void          RegisterScene(ISceneLayer* scene);
    void          UnregisterScene(ISceneLayer* scene);
    Scene*        GetFocusedScene() const { return m_focusedScene; }

private:
// Returns the topmost Scene whose rectangle contains (x, y), or nullptr.
Scene* SceneAtPoint(double x, double y) const;
// Returns true if (x, y) falls inside any registered UIScene rectangle.
bool UISceneAtPoint(double x, double y) const;
// Returns the topmost layer whose drag-handle corner (top-left 7px) contains (x, y).
ISceneLayer* LayerAtDragHandle(double x, double y) const;
// Returns the topmost layer whose edge (within k_resizeHandleSize px) contains (x,y).
// Writes which edges are hot into edgeFlags (bitmask of EdgeFlag values).
ISceneLayer* LayerAtResizeEdge(double x, double y, int& edgeFlags) const;
// Apply snap: clamp each active resize edge to the nearest window border or
// opposing layer edge within k_snapThreshold pixels.
void ApplySnap(ISceneLayer* layer, int edgeFlags, int& left, int& top, int& right, int& bottom) const;

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
    LayerDetachCallback              m_layerDetachCallback;
    WindowDockCallback               m_windowDockCallback;

    // Layers registered with this window for focus routing
    std::vector<ISceneLayer*> m_scenes;
    Scene*                    m_focusedScene = nullptr;

    // Scene drag state
    static constexpr int      k_dragHandleSize = 11;
    ISceneLayer*              m_draggedLayer   = nullptr;
    double                    m_dragOffsetX    = 0.0;
    double                    m_dragOffsetY    = 0.0;

    // Window-drag mode: active when the sole scene's drag handle is grabbed.
    // The OS window moves with the cursor instead of the layer inside it.
    bool   m_windowDragMode    = false;
    int    m_windowDragOffsetX = 0;  // cursor screen X minus window screen X at drag start
    int    m_windowDragOffsetY = 0;  // cursor screen Y minus window screen Y at drag start

    // Scene resize state
    static constexpr int  k_resizeHandleSize = 5;   // px from edge that starts a resize
    static constexpr int  k_snapThreshold    = 10;  // px — snap to window edge or layer edge
    static constexpr int  k_minLayerSize     = 32;  // minimum width or height after resize

    // Edge bitmask
    enum EdgeFlag { Edge_None = 0, Edge_Left = 1, Edge_Right = 2, Edge_Top = 4, Edge_Bottom = 8 };

    ISceneLayer* m_resizedLayer  = nullptr;
    int          m_resizeEdges   = Edge_None; // bitmask of active EdgeFlag values
    // Rect of the layer at the moment the resize started (in window pixels)
    int          m_resizeStartL  = 0;  // posX
    int          m_resizeStartT  = 0;  // posY
    int          m_resizeStartR  = 0;  // posX + width
    int          m_resizeStartB  = 0;  // posY + height
    double       m_resizeMouseX  = 0.0;
    double       m_resizeMouseY  = 0.0;
};