#include "Window.h"
#include <iostream>

Window::Window(std::shared_ptr<Vulkan::VulkanContext> vulkanContext, std::string title,
    int width, int height, int posx, int posy)
    : m_Title(title), m_Width(width), m_Height(height), m_PosX(posx), m_PosY(posy), m_VulkanContext(vulkanContext)
{
    m_EventController = std::make_unique<EventController>();
    m_EventController->AttachLayer(this);
    m_SceneController = std::make_unique<SceneController>(m_EventController.get());

    Vulkan::SurfaceFlags flags = m_DefaultVulkanWindowFlags;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Crucial for Vulkan
    glfwWindowHint(GLFW_RESIZABLE, (flags & Vulkan::SurfaceFlags::Resizeable) != Vulkan::SurfaceFlags::None ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, (flags & Vulkan::SurfaceFlags::Fullscreenable) != Vulkan::SurfaceFlags::None ? GLFW_TRUE : GLFW_FALSE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    m_pGLFWWindow = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);
    if (!m_pGLFWWindow) {
        throw std::runtime_error("Failed to create GLFW window for WindowID: " + std::to_string(m_WindowID));
    }

    if ((flags & Vulkan::SurfaceFlags::Fullscreen) != Vulkan::SurfaceFlags::None) {
        glfwSetWindowMonitor(m_pGLFWWindow, monitor, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
    }
    else {
        if (m_PosX == 0 && m_PosY == 0) {
            m_PosX = mode->width / 4;
            m_PosY = mode->height / 4;
        }
        glfwSetWindowMonitor(m_pGLFWWindow, nullptr, m_PosX, m_PosY, m_Width, m_Height, GLFW_DONT_CARE);
    }

    glfwShowWindow(m_pGLFWWindow);

    m_WindowID = m_VulkanContext->CreateNewWindow(m_DefaultVulkanWindowFlags);
    m_VulkanWindow = m_VulkanContext->GetWindow(m_WindowID);

    glfwSetWindowUserPointer(m_pGLFWWindow, this);
    InitCallbacks();
}

void Window::OnInput(InputEvent& event)
{
    if(event.action == Input::Action::PRESS && event.type == InputEvent::Type::Key)
    {
        if(event.code == Input::Keyboard::KEY_ESCAPE)
        {
            m_EventController->PostWindowCloseEvent(m_pGLFWWindow);
            event.Consume();
        }
	}
    else if(event.type == InputEvent::Type::MouseMove)
    {
		std::cout << "Mouse Move Event at (" << event.x << ", " << event.y << ")" << std::endl;
    }
    else if (event.type == InputEvent::Type::MouseButton)
    {
        if(event.action == Input::Action::PRESS && event.code == Input::BUTTON_1)
        {
            ResizeScene(0, event.x, event.y);
        }
        else if(event.action == Input::Action::RELEASE && event.code == Input::BUTTON_1)
        {
            std::cout << "Mouse Button " << event.code << " Released" << std::endl;
		}
    }
}

int Window::GetZIndex() const
{
    return 0;
};

void Window::InitCallbacks() {
    // Resize Callback
    glfwSetFramebufferSizeCallback(m_pGLFWWindow, [](GLFWwindow* w, int width, int height) {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(w));
        if (self) {
            self->m_VulkanWindow->needsToBeRecreated = true;
            self->m_Width = width;
            self->m_Height = height;
            // Notify EventController (The Z-layers might need to move UI)
            self->m_EventController->PostResizeEvent(self->m_WindowID, width, height);
        }
        });

    // Close Callback
    glfwSetWindowCloseCallback(m_pGLFWWindow, [](GLFWwindow* w) {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(w));
        if (self) {
            self->m_EventController->PostWindowCloseEvent(w);
        }
    });

    // Input Callbacks
    glfwSetKeyCallback(m_pGLFWWindow, [](GLFWwindow* w, int key, int scancode, int action, int mods) {
        static_cast<Window*>(glfwGetWindowUserPointer(w))->m_EventController->PostKeyEvent(key, scancode, action, mods);
        });

    glfwSetMouseButtonCallback(m_pGLFWWindow, [](GLFWwindow* w, int button, int action, int mods) {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(w));

        // 1. Get current mouse position from GLFW
        double xpos, ypos;
        glfwGetCursorPos(w, &xpos, &ypos);

        // 2. Post the event with the coordinates included
        self->m_EventController->PostMouseButtonEvent(button, action, xpos, ypos, mods);
        });

    glfwSetCursorPosCallback(m_pGLFWWindow, [](GLFWwindow* w, double xpos, double ypos) {
        static_cast<Window*>(glfwGetWindowUserPointer(w))->m_EventController->PostMouseMoveEvent(xpos, ypos);
        });

    glfwSetScrollCallback(m_pGLFWWindow, [](GLFWwindow* w, double xoffset, double yoffset) {
        static_cast<Window*>(glfwGetWindowUserPointer(w))->m_EventController->PostMouseScrollEvent(xoffset, yoffset);
        });
}

void Window::Update() {
    if (IsWindowStillValid()) {
        glfwPollEvents();
        m_SceneController->Update(); // Update logic for all scenes in this window
    }
}

void Window::Render() {
    if (IsWindowStillValid()) {
        m_VulkanWindow->RenderScenes();
    }
}

bool Window::IsWindowStillValid() {
    return m_VulkanWindow != nullptr && m_pGLFWWindow != nullptr;
}

void Window::InitWindow() {
    if (IsWindowStillValid()) {
        m_VulkanWindow->InitWindow(m_pGLFWWindow);
    }
}

void Window::CloseWindow() {
    if (m_VulkanWindow) {
        m_VulkanWindow->CloseWindow();
    }
    if (m_pGLFWWindow) {
        glfwDestroyWindow(m_pGLFWWindow);
        m_pGLFWWindow = nullptr;
    }
}

uint8_t Window::CreateNewScene(uint32_t width, uint32_t height, int posx, int posy) {
    uint8_t vulkanSceneID = m_VulkanWindow->CreateNewScene(width, height, posx, posy);
    SceneCreationInfo info{ m_WindowID, vulkanSceneID, width, height, posx, posy, GetRenderSurfaceCount() + 1};
    m_SceneController->CreateNewScene<CameraScene>(vulkanSceneID, info, m_VulkanContext, 1);
    m_ChildrenRenderSurfaces.push_back(vulkanSceneID);
	m_EventController->AttachLayer(&m_SceneController->GetScene(vulkanSceneID));

    return vulkanSceneID;
}

void Window::MoveScene(SceneID sceneID, int newX, int newY)
{
	m_VulkanWindow->MoveScene(sceneID, newX, newY);
}

void Window::ResizeScene(SceneID sceneID, int newX, int newY)
{
	m_VulkanWindow->ResizeScene(sceneID, newX, newY);
}

void Window::AddChildRenderSurface(uint8_t renderSurfaceID) {
    m_ChildrenRenderSurfaces.push_back(renderSurfaceID);
}