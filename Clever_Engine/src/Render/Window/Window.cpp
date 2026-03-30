#include "Window.h"
#include "Scene.h"
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <chrono>
#include <thread>


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
    // Just poll events - do NOT call BeginFrame here
    // BeginFrame has a fence wait that can block, preventing responsive input
    glfwPollEvents();
}

void Window::Render(const std::vector<Scene*>& scenes)
{
    if (!m_renderer || !m_iWindow || !m_blitCommandList)
        return;

    // Call BeginFrame FIRST to acquire the swapchain image
    m_iWindow->BeginFrame();
    
    if (!m_iWindow->IsFrameReady())
        return;
    
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

    // Blit each scene's color target onto the backbuffer
    for (Scene* scene : scenes)
    {
        if (!scene) continue;
        GraphicsCore::ITexture* colorTarget = scene->GetColorTarget();
        if (!colorTarget) continue;
        m_blitCommandList->BlitTexture(colorTarget, backbuffer);
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

void Window::OnKey(int glfwKey, int scancode, int action, int mods)
{
    m_eventController->PostKeyEvent(glfwKey, scancode, action, mods);
}

void Window::OnMouseButton(int button, int action, double x, double y, int mods)
{
    m_eventController->PostMouseButtonEvent(button, action, x, y, mods);
}

void Window::OnMouseMove(double x, double y)
{
    m_eventController->PostMouseMoveEvent(x, y);
}

void Window::OnMouseScroll(double xoffset, double yoffset)
{
    m_eventController->PostMouseScrollEvent(xoffset, yoffset);
}