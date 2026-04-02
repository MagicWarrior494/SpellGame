#include "Window.h"
#include "Scene.h"
#include "UI/UIScene.h"
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <algorithm>


Window::Window(GraphicsCore::IRenderer* renderer,
    const std::string& title,
    uint32_t                 width,
    uint32_t                 height,
    int                      posX,
    int                      posY)
    : m_renderer(renderer)
    , m_title(title)
    , m_width(width)
    , m_height(height)
{
    (void)posX;
    (void)posY;

    m_eventController = std::make_unique<EventController>();
    m_eventController->AttachLayer(this);

    GraphicsCore::WindowDesc desc{};
    desc.width = width;
    desc.height = height;
    desc.title = m_title.c_str();
    desc.fullscreen = false;
    desc.vsync = true;

    m_iWindow = m_renderer->CreateWindow(desc);

    if (!m_iWindow)
        throw std::runtime_error("Failed to create window: " + title);

    m_iWindow->SetEventSink(this);
    m_blitCommandList = m_renderer->CreateCommandList();
}

Window::~Window()
{
    if (m_renderer)
    {
        // Ensure the GPU has finished all in-flight work for this window
        // before destroying its command list and swap chain resources.
        m_renderer->WaitIdle();
        if (m_blitCommandList) m_renderer->DestroyCommandList(m_blitCommandList);
        if (m_iWindow)         m_renderer->DestroyWindow(m_iWindow);
    }
}

bool Window::IsAlive()
{
    if (!m_iWindow)
        return false;

    GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(m_iWindow->GetPlatformHandle());
    if (!glfwWindow)
        return false;

    return glfwWindowShouldClose(glfwWindow) == 0;
}

void Window::Update()
{
}

void Window::Render(const std::vector<ISceneLayer*>& scenes)
{
    if (!m_renderer || !m_iWindow || !m_blitCommandList)
        return;

    // Don't render if the window is closing
    GLFWwindow* glfwWin = static_cast<GLFWwindow*>(m_iWindow->GetPlatformHandle());
    if (glfwWin && glfwWindowShouldClose(glfwWin))
        return;

    // FPS counter: update title once per second
    ++m_fpsFrameCount;
    auto fpsNow     = std::chrono::steady_clock::now();
    float fpsElapsed = std::chrono::duration<float>(fpsNow - m_fpsLastTime).count();
    if (fpsElapsed >= 1.0f)
    {
        int fps = static_cast<int>(m_fpsFrameCount / fpsElapsed);
        std::string fpsTitle = m_title + "  |  FPS: " + std::to_string(fps);
        m_iWindow->SetTitle(fpsTitle.c_str());
        m_fpsFrameCount = 0;
        m_fpsLastTime   = fpsNow;
    }

    // Resize scenes BEFORE BeginFrame
    const auto& windowDesc = m_iWindow->GetDesc();
    uint32_t curW = windowDesc.width;
    uint32_t curH = windowDesc.height;
    if (curW != m_width || curH != m_height)
    {
        m_width  = curW;
        m_height = curH;
        for (ISceneLayer* layer : scenes)
        {
            if (!layer) continue;
            const SceneDesc& sd = layer->GetDesc();
            if (sd.posX == 0 && sd.posY == 0 && sd.width == m_width && sd.height == m_height)
                layer->Resize(curW, curH);
        }
    }

    m_iWindow->BeginFrame();
    if (!m_iWindow->IsFrameReady())
        return;

    for (ISceneLayer* layer : scenes)
        if (layer) layer->Render();

    GraphicsCore::ITexture* backbuffer = m_iWindow->GetCurrentBackbuffer();
    if (!backbuffer)
        return;

    m_blitCommandList->Begin();
    m_blitCommandList->TextureBarrier(backbuffer,
        GraphicsCore::TextureUsage_RenderTarget,
        GraphicsCore::TextureUsage_TransferDst);

    // Clear the entire backbuffer to black first so areas not covered by any
    // scene layer don't show stale data from previous frames.
    m_blitCommandList->ClearTexture(backbuffer, 0.0f, 0.0f, 0.0f, 1.0f);

    for (ISceneLayer* layer : scenes)
    {
        if (!layer) continue;
        GraphicsCore::ITexture* colorTarget = layer->GetColorTarget();
        if (!colorTarget) continue;
        const SceneDesc& sd = layer->GetDesc();
        m_blitCommandList->BlitTexture(colorTarget, backbuffer, sd.posX, sd.posY);
    }

    m_blitCommandList->TextureBarrier(backbuffer,
        GraphicsCore::TextureUsage_TransferDst,
        GraphicsCore::TextureUsage_RenderTarget);
    m_blitCommandList->End();

    m_renderer->SubmitBlit(m_blitCommandList, m_iWindow);
    m_renderer->Present(m_iWindow);
}

void Window::OnResize(uint32_t width, uint32_t height)
{
    m_width = width;
    m_height = height;
}

void Window::GetScreenPosition(int& x, int& y) const
{
    if (m_iWindow) m_iWindow->GetPosition(x, y);
    else           x = y = 0;
}

void Window::SetScreenPosition(int x, int y)
{
    if (m_iWindow) m_iWindow->SetPosition(x, y);
}

void Window::SetSize(uint32_t width, uint32_t height)
{
    if (m_iWindow) m_iWindow->SetSize(width, height);
}

void Window::RegisterScene(ISceneLayer* scene)
{
    if (!scene) return;
    if (std::find(m_scenes.begin(), m_scenes.end(), scene) == m_scenes.end())
        m_scenes.push_back(scene);
}

void Window::UnregisterScene(ISceneLayer* scene)
{
    m_scenes.erase(std::remove(m_scenes.begin(), m_scenes.end(), scene), m_scenes.end());
    if (m_focusedScene == dynamic_cast<Scene*>(scene))
        m_focusedScene = nullptr;
}

Scene* Window::SceneAtPoint(double x, double y) const
{
    for (int i = static_cast<int>(m_scenes.size()) - 1; i >= 0; --i)
    {
        ISceneLayer* layer = m_scenes[i];
        if (!layer) continue;
        Scene* s = dynamic_cast<Scene*>(layer);
        if (!s) continue;  // UIScene handles its own input; skip for focus routing
        const SceneDesc& sd = s->GetDesc();
        if (x >= sd.posX && x < sd.posX + static_cast<int>(sd.width) &&
            y >= sd.posY && y < sd.posY + static_cast<int>(sd.height))
            return s;
    }
    return nullptr;
}

bool Window::UISceneAtPoint(double x, double y) const
{
    for (ISceneLayer* layer : m_scenes)
    {
        if (!layer) continue;
        UIScene* ui = dynamic_cast<UIScene*>(layer);
        if (!ui) continue;
        const SceneDesc& sd = ui->GetDesc();
        if (x >= sd.posX && x < sd.posX + static_cast<int>(sd.width) &&
            y >= sd.posY && y < sd.posY + static_cast<int>(sd.height))
            return true;
    }
    return false;
}

ISceneLayer* Window::LayerAtDragHandle(double x, double y) const
{
    // Iterate in reverse so the topmost (highest zIndex) layer wins.
    // The drag handle is a full-width strip along the top of each layer,
    // k_dragHandleSize pixels tall — like a title bar.
    for (int i = static_cast<int>(m_scenes.size()) - 1; i >= 0; --i)
    {
        ISceneLayer* layer = m_scenes[i];
        if (!layer) continue;
        const SceneDesc& sd = layer->GetDesc();
        if (x >= sd.posX && x < sd.posX + static_cast<int>(sd.width) &&
            y >= sd.posY && y <  sd.posY + k_dragHandleSize)
            return layer;
    }
    return nullptr;
}

ISceneLayer* Window::LayerAtResizeEdge(double x, double y, int& edgeFlags) const
{
    edgeFlags = Edge_None;
    for (int i = static_cast<int>(m_scenes.size()) - 1; i >= 0; --i)
    {
        ISceneLayer* layer = m_scenes[i];
        if (!layer) continue;
        const SceneDesc& sd = layer->GetDesc();

        int L = sd.posX;
        int T = sd.posY;
        int R = sd.posX + static_cast<int>(sd.width);
        int B = sd.posY + static_cast<int>(sd.height);

        // Cursor must be within the layer's bounding box (expanded by handle size)
        bool inX = (x >= L - k_resizeHandleSize && x <= R + k_resizeHandleSize);
        bool inY = (y >= T - k_resizeHandleSize && y <= B + k_resizeHandleSize);
        if (!inX || !inY) continue;

        // Cursor must actually be near at least one edge, not just anywhere inside
        bool nearL = (x >= L - k_resizeHandleSize && x <= L + k_resizeHandleSize);
        bool nearR = (x >= R - k_resizeHandleSize && x <= R + k_resizeHandleSize);
        bool nearT = (y >= T - k_resizeHandleSize && y <= T + k_resizeHandleSize);
        bool nearB = (y >= B - k_resizeHandleSize && y <= B + k_resizeHandleSize);

        // Cursor must be within the layer in the perpendicular axis to count as an edge hit
        int flags = Edge_None;
        if (nearL && y >= T && y <= B) flags |= Edge_Left;
        if (nearR && y >= T && y <= B) flags |= Edge_Right;
        if (nearT && x >= L && x <= R) flags |= Edge_Top;
        if (nearB && x >= L && x <= R) flags |= Edge_Bottom;

        if (flags != Edge_None)
        {
            edgeFlags = flags;
            return layer;
        }
    }
    return nullptr;
}

void Window::ApplySnap(ISceneLayer* layer, int edgeFlags,
                        int& left, int& top, int& right, int& bottom) const
{
    int winW = static_cast<int>(m_width);
    int winH = static_cast<int>(m_height);

    // Snap active edges to window borders
    if ((edgeFlags & Edge_Left)   && left   <= k_snapThreshold)              left   = 0;
    if ((edgeFlags & Edge_Top)    && top    <= k_snapThreshold)              top    = 0;
    if ((edgeFlags & Edge_Right)  && right  >= winW - k_snapThreshold)       right  = winW;
    if ((edgeFlags & Edge_Bottom) && bottom >= winH - k_snapThreshold)       bottom = winH;

    // Snap active edges to other layers' opposing edges
    for (ISceneLayer* other : m_scenes)
    {
        if (!other || other == layer) continue;
        const SceneDesc& od = other->GetDesc();
        int oL = od.posX;
        int oT = od.posY;
        int oR = od.posX + static_cast<int>(od.width);
        int oB = od.posY + static_cast<int>(od.height);

        if ((edgeFlags & Edge_Left)   && std::abs(left   - oR) <= k_snapThreshold) left   = oR;
        if ((edgeFlags & Edge_Right)  && std::abs(right  - oL) <= k_snapThreshold) right  = oL;
        if ((edgeFlags & Edge_Top)    && std::abs(top    - oB) <= k_snapThreshold) top    = oB;
        if ((edgeFlags & Edge_Bottom) && std::abs(bottom - oT) <= k_snapThreshold) bottom = oT;
    }
}

void Window::OnKey(int glfwKey, int scancode, int action, int mods)
{
    // Key events go only to the focused scene; if no scene is focused, keys go nowhere
    if (m_focusedScene)
        m_focusedScene->GetEventController().PostKeyEvent(glfwKey, scancode, action, mods);
}

void Window::OnMouseButton(int button, int action, double x, double y, int mods)
{
    // Left button: check for drag-handle grab first, then resize edge
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            // Drag handle (top-left corner) takes priority
            ISceneLayer* hit = LayerAtDragHandle(x, y);
            if (hit)
            {
                // Single-layer window: drag the OS window itself rather than
                // moving the scene inside it (it fills the window anyway).
                if (m_scenes.size() == 1)
                {
                    m_windowDragMode = true;
                    int winSX = 0, winSY = 0;
                    GetScreenPosition(winSX, winSY);
                    // cursor screen position = window screen position + local cursor position
                    m_windowDragOffsetX = static_cast<int>(x); // local cursor X at grab
                    m_windowDragOffsetY = static_cast<int>(y); // local cursor Y at grab
                    return;
                }

                m_draggedLayer = hit;
                const SceneDesc& sd = hit->GetDesc();
                m_dragOffsetX = x - sd.posX;
                m_dragOffsetY = y - sd.posY;
                return;
            }

            // Resize edge check
            int edgeFlags = Edge_None;
            ISceneLayer* resizeHit = LayerAtResizeEdge(x, y, edgeFlags);
            if (resizeHit)
            {
                m_resizedLayer = resizeHit;
                m_resizeEdges  = edgeFlags;
                const SceneDesc& sd = resizeHit->GetDesc();
                m_resizeStartL = sd.posX;
                m_resizeStartT = sd.posY;
                m_resizeStartR = sd.posX + static_cast<int>(sd.width);
                m_resizeStartB = sd.posY + static_cast<int>(sd.height);
                m_resizeMouseX = x;
                m_resizeMouseY = y;
                return;
            }
        }
        else if (action == GLFW_RELEASE)
        {
            if (m_windowDragMode)
            {
                m_windowDragMode = false;
                // Fire the dock callback so Engine can merge this window into
                // another if the cursor landed over one.
                if (m_windowDockCallback && !m_scenes.empty())
                {
                    int winSX = 0, winSY = 0;
                    GetScreenPosition(winSX, winSY);
                    int screenCX = winSX + static_cast<int>(x);
                    int screenCY = winSY + static_cast<int>(y);
                    m_windowDockCallback(m_scenes[0], screenCX, screenCY);
                }
                return;
            }
            if (m_draggedLayer)
            {
                m_draggedLayer = nullptr;
                return;
            }
            if (m_resizedLayer)
            {
                m_resizedLayer = nullptr;
                m_resizeEdges  = Edge_None;
                return;
            }
        }
    }

    // Always broadcast mouse button events to all UIScene layers
    for (ISceneLayer* layer : m_scenes)
    {
        UIScene* ui = dynamic_cast<UIScene*>(layer);
        if (ui) ui->GetEventController().PostMouseButtonEvent(button, action, x, y, mods);
    }

    // If the click landed on a UIScene, do not propagate to 3D scenes
    if (UISceneAtPoint(x, y))
        return;

    // Focus routing for 3D scenes
    Scene* hit = SceneAtPoint(x, y);
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        if (m_focusedScene && m_focusedScene != hit)
            m_focusedScene->ResetInputState();
        m_focusedScene = hit;
    }

    Scene* target = m_focusedScene ? m_focusedScene : hit;
    if (target)
        target->GetEventController().PostMouseButtonEvent(button, action, x, y, mods);
}

void Window::OnMouseMove(double x, double y)
{
    // Window-drag mode: the sole scene's drag handle is held, so we move the
    // OS window with the cursor instead of moving the scene inside it.
    if (m_windowDragMode)
    {
        int winSX = 0, winSY = 0;
        GetScreenPosition(winSX, winSY);
        // Compute new window screen position so the grab point stays under the cursor.
        // m_windowDragOffsetX/Y is the local cursor position at the moment of grab.
        // The screen cursor is: winSX + x  (GLFW gives local coords even while dragging).
        int newWinSX = (winSX + static_cast<int>(x)) - m_windowDragOffsetX;
        int newWinSY = (winSY + static_cast<int>(y)) - m_windowDragOffsetY;
        SetScreenPosition(newWinSX, newWinSY);
        return;
    }

    // If a layer is being dragged, move it and consume the event
    if (m_draggedLayer)
    {
        // Check if cursor has left this window's client area
        if (m_layerDetachCallback &&
            (x < 0 || y < 0 ||
             x >= static_cast<double>(m_width) ||
             y >= static_cast<double>(m_height)))
        {
            // Convert window-local cursor to screen space
            int winScreenX = 0, winScreenY = 0;
            GetScreenPosition(winScreenX, winScreenY);
            int screenCX = winScreenX + static_cast<int>(x);
            int screenCY = winScreenY + static_cast<int>(y);

            ISceneLayer* layer = m_draggedLayer;
            m_draggedLayer = nullptr; // clear before callback so re-entrancy is safe
            m_layerDetachCallback(layer, screenCX, screenCY);
            return;
        }

        const SceneDesc& sd = m_draggedLayer->GetDesc();
        int newX = static_cast<int>(x - m_dragOffsetX);
        int newY = static_cast<int>(y - m_dragOffsetY);

        // Clamp so the scene cannot leave the window
        newX = std::max(0, std::min(newX, static_cast<int>(m_width)  - static_cast<int>(sd.width)));
        newY = std::max(0, std::min(newY, static_cast<int>(m_height) - static_cast<int>(sd.height)));

        // Snap all four edges — the whole rect moves rigidly so pass all edges active
        int left   = newX;
        int top    = newY;
        int right  = newX + static_cast<int>(sd.width);
        int bottom = newY + static_cast<int>(sd.height);
        ApplySnap(m_draggedLayer, Edge_Left | Edge_Right | Edge_Top | Edge_Bottom,
                  left, top, right, bottom);
        // ApplySnap may pull opposite edges independently; keep size fixed by
        // preferring whichever edge snapped and re-deriving the other.
        if (left != newX)        right  = left  + static_cast<int>(sd.width);
        else if (right != newX + static_cast<int>(sd.width))  left = right  - static_cast<int>(sd.width);
        if (top  != newY)        bottom = top   + static_cast<int>(sd.height);
        else if (bottom != newY + static_cast<int>(sd.height)) top  = bottom - static_cast<int>(sd.height);

        m_draggedLayer->SetPosition(left, top);
        return;
    }

    // If a layer is being resized, compute new rect and apply snap
    if (m_resizedLayer)
    {
        int dx = static_cast<int>(x - m_resizeMouseX);
        int dy = static_cast<int>(y - m_resizeMouseY);

        int left   = m_resizeStartL;
        int top    = m_resizeStartT;
        int right  = m_resizeStartR;
        int bottom = m_resizeStartB;

        if (m_resizeEdges & Edge_Left)   left   = std::min(m_resizeStartL + dx, right  - k_minLayerSize);
        if (m_resizeEdges & Edge_Right)  right  = std::max(m_resizeStartR + dx, left   + k_minLayerSize);
        if (m_resizeEdges & Edge_Top)    top    = std::min(m_resizeStartT + dy, bottom - k_minLayerSize);
        if (m_resizeEdges & Edge_Bottom) bottom = std::max(m_resizeStartB + dy, top    + k_minLayerSize);

        // Clamp to window bounds
        left   = std::max(left,   0);
        top    = std::max(top,    0);
        right  = std::min(right,  static_cast<int>(m_width));
        bottom = std::min(bottom, static_cast<int>(m_height));

        // Snap edges to window borders and other layers
        ApplySnap(m_resizedLayer, m_resizeEdges, left, top, right, bottom);

        // Enforce minimum size after snap
        if (right  - left < k_minLayerSize) right  = left + k_minLayerSize;
        if (bottom - top  < k_minLayerSize) bottom = top  + k_minLayerSize;

        uint32_t newW = static_cast<uint32_t>(right  - left);
        uint32_t newH = static_cast<uint32_t>(bottom - top);

        m_resizedLayer->SetPosition(left, top);
        m_resizedLayer->Resize(newW, newH);
        return;
    }

    // Broadcast cursor position to all UIScene layers for hover detection
    for (ISceneLayer* layer : m_scenes)
    {
        UIScene* ui = dynamic_cast<UIScene*>(layer);
        if (ui) ui->GetEventController().PostMouseMoveEvent(x, y);
    }

    // Mouse move goes only to the focused 3D scene (it may have the cursor locked)
    if (m_focusedScene)
        m_focusedScene->GetEventController().PostMouseMoveEvent(x, y);
}

void Window::OnMouseScroll(double xoffset, double yoffset)
{
    m_eventController->PostMouseScrollEvent(xoffset, yoffset);
}

void Window::OnFocus(bool focused)
{
    if (!focused)
    {
        // Cancel any in-progress drag or resize — the OS may have stolen the mouse
        // (e.g. the user grabbed the window resize border) without delivering
        // a GLFW_RELEASE for the left button, leaving state set forever.
        m_draggedLayer   = nullptr;
        m_resizedLayer   = nullptr;
        m_resizeEdges    = Edge_None;
        m_windowDragMode = false;

        for (ISceneLayer* layer : m_scenes)
        {
            if (layer) layer->ResetInputState();
        }
    }
}

void Window::OnClose()
{
    if (m_closeCallback)
        m_closeCallback();
}