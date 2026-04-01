#include "Window.h"
#include "Scene.h"
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

void Window::Render(const std::vector<Scene*>& scenes)
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

    // Resize scenes BEFORE BeginFrame so that WaitIdle inside Scene::Resize
    // does not fire while a swapchain image is already acquired (which would
    // leave m_imageAvailableSemaphore in a permanently-signalled state and
    // stall the next vkAcquireNextImageKHR indefinitely).
    const auto& windowDesc = m_iWindow->GetDesc();
    uint32_t curW = windowDesc.width;
    uint32_t curH = windowDesc.height;
    if (curW != m_width || curH != m_height)
    {
        m_width  = curW;
        m_height = curH;
        for (Scene* scene : scenes)
        {
            if (!scene) continue;
            const SceneDesc& sd = scene->GetDesc();
            // Only auto-resize scenes that are explicitly filling the whole window
            if (sd.posX == 0 && sd.posY == 0 && sd.width == m_width && sd.height == m_height)
                scene->Resize(curW, curH);
        }
    }

    // Call BeginFrame to acquire the swapchain image
    m_iWindow->BeginFrame();
    
    if (!m_iWindow->IsFrameReady())
    {
        // Frame not ready (e.g. minimised, or swapchain out of date).
        // The fence was not reset in BeginFrame so nothing needs to be done here.
        return;
    }

    // Now render all scenes (they will check IsFrameReady and proceed)
    for (Scene* scene : scenes)
    {
        if (scene)
            scene->Render();
    }
    
    // Finally, blit all scene color targets to the backbuffer
    GraphicsCore::ITexture* backbuffer = m_iWindow->GetCurrentBackbuffer();
    if (!backbuffer)
        return;

    m_blitCommandList->Begin();

    // Transition backbuffer: undefined -> transfer dst
    m_blitCommandList->TextureBarrier(backbuffer,
        GraphicsCore::TextureUsage_RenderTarget,
        GraphicsCore::TextureUsage_TransferDst);

    // Blit each scene's color target onto the backbuffer at the scene's position
    for (Scene* scene : scenes)
    {
        if (!scene) continue;
        GraphicsCore::ITexture* colorTarget = scene->GetColorTarget();
        if (!colorTarget) continue;
        const SceneDesc& sd = scene->GetDesc();
        m_blitCommandList->BlitTexture(colorTarget, backbuffer, sd.posX, sd.posY);
    }

    // Transition backbuffer: transfer dst -> present
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

void Window::RegisterScene(Scene* scene)
{
    if (!scene) return;
    if (std::find(m_scenes.begin(), m_scenes.end(), scene) == m_scenes.end())
        m_scenes.push_back(scene);
}

void Window::UnregisterScene(Scene* scene)
{
    m_scenes.erase(std::remove(m_scenes.begin(), m_scenes.end(), scene), m_scenes.end());
    if (m_focusedScene == scene)
        m_focusedScene = nullptr;
}

Scene* Window::SceneAtPoint(double x, double y) const
{
    // Walk in reverse registration order so later (higher z) scenes win
    for (int i = static_cast<int>(m_scenes.size()) - 1; i >= 0; --i)
    {
        Scene* s = m_scenes[i];
        if (!s) continue;
        const SceneDesc& sd = s->GetDesc();
        if (x >= sd.posX && x < sd.posX + static_cast<int>(sd.width) &&
            y >= sd.posY && y < sd.posY + static_cast<int>(sd.height))
            return s;
    }
    return nullptr;
}

void Window::OnKey(int glfwKey, int scancode, int action, int mods)
{
    // Key events go only to the focused scene; if no scene is focused, keys go nowhere
    if (m_focusedScene)
        m_focusedScene->GetEventController().PostKeyEvent(glfwKey, scancode, action, mods);
}

void Window::OnMouseButton(int button, int action, double x, double y, int mods)
{
    // Determine which scene the click landed in
    Scene* hit = SceneAtPoint(x, y);

    // On right-click press, transfer focus to the scene under the cursor
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        if (m_focusedScene && m_focusedScene != hit)
        {
            // Reset the old scene's input so held keys / mouse lock don't stick
            m_focusedScene->ResetInputState();
        }
        m_focusedScene = hit;
    }

    // Deliver the event to the scene under the cursor (or focused scene if locked)
    Scene* target = m_focusedScene ? m_focusedScene : hit;
    if (target)
        target->GetEventController().PostMouseButtonEvent(button, action, x, y, mods);
}

void Window::OnMouseMove(double x, double y)
{
    // Mouse move goes only to the focused scene (it may have the cursor locked)
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
        for (Scene* scene : m_scenes)
        {
            if (scene)
                scene->ResetInputState();
        }
    }
}

void Window::OnClose()
{
    if (m_closeCallback)
        m_closeCallback();
}