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
    if(event.type == InputEvent::Type::Key)
    {
        if (event.code == Input::Keyboard::KEY_ESCAPE)
        {
                UnlockMouse(m_pGLFWWindow);
                m_EventController->ResetLayers();
                m_EventController->AttachLayer(this);
        }
	}
    else if (event.type == InputEvent::Type::MouseButton)
    {
        
        if(event.action == Input::Action::PRESS && event.code == Input::BUTTON_1)
        {
            for (auto sceneID : m_ChildrenRenderSurfaces)
            {
                auto& scene = m_SceneController->GetScene(sceneID);
                auto info = scene.GetCreationInfo();
                if (event.x > info.posx &&
                    event.x < info.posx + info.width &&
                    event.y > info.posy &&
                    event.y < info.posy + info.height)
                {
                    m_EventController->ResetLayers();
                    m_EventController->AttachLayer(this);
                    m_EventController->AttachLayer(&scene);
                    if (scene.GetSceneType() == SceneType::CameraScene)
                    {
                        LockMouse(m_pGLFWWindow, (info.posx + info.width)/2, (info.posy + info.height)/2);
                    }
                }
            }
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

    Vulkan::Window& window = *m_VulkanContext->GetWindow(m_WindowID);

    Vulkan::VulkanBuffer& buffer = window.vulkanSurface.cameraBuffer;

    size_t minAligment = m_VulkanContext->GetPhysicalDeviceMinAlignment();

    sharedCameraSceneData = std::make_unique<SharedCameraSceneData>
        (
            buffer,
            minAligment,
            sizeof(SceneShaderData)
        );
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