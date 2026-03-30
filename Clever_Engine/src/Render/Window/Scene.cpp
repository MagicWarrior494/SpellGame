#include "Scene.h"
#include "Window.h"
#include "World/Assets/Mesh.h"
#include "World/Assets/Shader.h"
#include "World/ECS/Components.h"
#include <chrono>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/quaternion.hpp>
#include <GLFW/glfw3.h>

Scene::Scene(GraphicsCore::IRenderer* renderer,
         AssetManager*            assetManager,
         Window*                  window,
         const SceneDesc&         desc)
: m_renderer(renderer)
, m_assetManager(assetManager)
, m_window(window)
, m_desc(desc)
{
m_registry = std::make_unique<Registry>();

    // --- Offscreen color target ---
    GraphicsCore::TextureDesc colorDesc{};
    colorDesc.width       = desc.width;
    colorDesc.height      = desc.height;
    colorDesc.depth       = 1;
    colorDesc.mipLevels   = 1;
    colorDesc.arrayLayers = 1;
    colorDesc.type        = GraphicsCore::TextureType::Texture2D;
    colorDesc.format      = GraphicsCore::TextureFormat::RGBA8;
    colorDesc.usage       = GraphicsCore::TextureUsage_RenderTarget
                          | GraphicsCore::TextureUsage_TransferSrc
                          | GraphicsCore::TextureUsage_ShaderResource;

    m_colorTarget = m_renderer->CreateTexture(colorDesc);

    // --- Offscreen depth target ---
    GraphicsCore::TextureDesc depthDesc{};
    depthDesc.width       = desc.width;
    depthDesc.height      = desc.height;
    depthDesc.depth       = 1;
    depthDesc.mipLevels   = 1;
    depthDesc.arrayLayers = 1;
    depthDesc.type        = GraphicsCore::TextureType::Texture2D;
    depthDesc.format      = GraphicsCore::TextureFormat::Depth32F;
    depthDesc.usage       = GraphicsCore::TextureUsage_DepthStencil;

    m_depthTarget = m_renderer->CreateTexture(depthDesc);

    m_commandList = m_renderer->CreateCommandList();
}

Scene::~Scene()
{
    if (m_renderer)
    {
        m_renderer->WaitIdle();

        if (m_commandList) m_renderer->DestroyCommandList(m_commandList);
        if (m_colorTarget) m_renderer->DestroyTexture(m_colorTarget);
        if (m_depthTarget) m_renderer->DestroyTexture(m_depthTarget);
    }
}

void Scene::AttachToWindow(Window& window)
{
    window.GetEventController().AttachLayer(this);
}

void Scene::OnInput(InputEvent& event)
{
    if (event.type == InputEvent::Type::MouseButton)
    {
        if (event.code == Input::Mouse::BUTTON_2) // right-click toggles mouse lock
        {
            if (event.action == Input::Action::PRESS)
            {
                m_mouseLocked = true;
                GLFWwindow* glfwWin = static_cast<GLFWwindow*>(m_window->GetIWindow()->GetPlatformHandle());
                glfwSetInputMode(glfwWin, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
            else if (event.action == Input::Action::RELEASE)
            {
                m_mouseLocked = false;
                GLFWwindow* glfwWin = static_cast<GLFWwindow*>(m_window->GetIWindow()->GetPlatformHandle());
                glfwSetInputMode(glfwWin, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            event.Consume();
        }
        return;
    }

    if (event.type == InputEvent::Type::Key)
    {
        bool pressed = (event.action == Input::Action::PRESS || event.action == Input::Action::REPEAT);
        switch (event.code)
        {
        case Input::KEY_W: m_keysHeld[0] = pressed; break;
        case Input::KEY_S: m_keysHeld[1] = pressed; break;
        case Input::KEY_A: m_keysHeld[2] = pressed; break;
        case Input::KEY_D: m_keysHeld[3] = pressed; break;
        case Input::KEY_Q: m_keysHeld[4] = pressed; break;
        case Input::KEY_E: m_keysHeld[5] = pressed; break;
        default: break;
        }
        return;
    }

    if (event.type == InputEvent::Type::MouseMove && m_mouseLocked)
    {
        auto& cameras = m_registry->GetAllComponents<CameraComponent>();
        for (auto& [entity, cam] : cameras)
        {
            cam.yaw   -= static_cast<float>(event.deltaX) * cam.sensitivity;
            cam.pitch -= static_cast<float>(event.deltaY) * cam.sensitivity;
            cam.pitch  = glm::clamp(cam.pitch, -89.0f, 89.0f);

            glm::quat qYaw   = glm::angleAxis(glm::radians(cam.yaw),   glm::vec3(0, 1, 0));
            glm::quat qPitch = glm::angleAxis(glm::radians(cam.pitch), glm::vec3(1, 0, 0));
            cam.rotation = qYaw * qPitch;
        }
        event.Consume();
    }
}

void Scene::Update()
{
    auto now = std::chrono::steady_clock::now();
    if (m_firstFrame)
    {
        m_lastFrameTime = now;
        m_firstFrame = false;
        return;
    }

    float dt = std::chrono::duration<float>(now - m_lastFrameTime).count();
    m_lastFrameTime = now;

    auto& cameras = m_registry->GetAllComponents<CameraComponent>();
    for (auto& [entity, cam] : cameras)
    {
        glm::vec3 move(0.0f);
        if (m_keysHeld[0]) move += cam.GetForward();
        if (m_keysHeld[1]) move -= cam.GetForward();
        if (m_keysHeld[2]) move -= cam.GetRight();
        if (m_keysHeld[3]) move += cam.GetRight();
        if (m_keysHeld[4]) move -= cam.GetUp();
        if (m_keysHeld[5]) move += cam.GetUp();

        if (glm::length(move) > 0.0f)
            cam.position += glm::normalize(move) * cam.moveSpeed * dt;
    }
}

void Scene::Render()
{
    if (!m_renderer || !m_registry)
        return;

    GraphicsCore::IWindow* iWindowCheck = m_window ? m_window->GetIWindow() : nullptr;
    if (iWindowCheck && !iWindowCheck->IsFrameReady())
        return;

    auto& meshComponents   = m_registry->GetAllComponents<MeshComponent>();
    auto& shaderComponents = m_registry->GetAllComponents<ShaderComponent>();

    m_commandList->Begin();

    // Transition color target: undefined -> color attachment
    m_commandList->TextureBarrier(m_colorTarget,
        GraphicsCore::TextureUsage_ShaderResource,
        GraphicsCore::TextureUsage_RenderTarget);

    // Transition depth target: undefined -> depth attachment
    m_commandList->TextureBarrier(m_depthTarget,
        GraphicsCore::TextureUsage_ShaderResource,
        GraphicsCore::TextureUsage_DepthStencil);

    GraphicsCore::ColorAttachment colorAttachment{};
    colorAttachment.texture       = m_colorTarget;
    colorAttachment.loadOp        = GraphicsCore::AttachmentLoadOp::Clear;
    colorAttachment.storeOp       = GraphicsCore::AttachmentStoreOp::Store;
    colorAttachment.clearColor[0] = 0.1f;
    colorAttachment.clearColor[1] = 0.1f;
    colorAttachment.clearColor[2] = 0.1f;
    colorAttachment.clearColor[3] = 1.0f;

    GraphicsCore::DepthStencilAttachment depthAttachment{};
    depthAttachment.texture    = m_depthTarget;
    depthAttachment.loadOp     = GraphicsCore::AttachmentLoadOp::Clear;
    depthAttachment.storeOp    = GraphicsCore::AttachmentStoreOp::DontCare;
    depthAttachment.clearDepth = 1.0f;

    m_commandList->BeginRendering(1, &colorAttachment, &depthAttachment);

    GraphicsCore::Viewport vp{};
    vp.x        = 0.0f;
    vp.y        = 0.0f;
    vp.width    = static_cast<float>(m_desc.width);
    vp.height   = static_cast<float>(m_desc.height);
    vp.minDepth = 0.0f;
    vp.maxDepth = 1.0f;
    m_commandList->SetViewport(vp);

    GraphicsCore::Scissor scissor{};
    scissor.x      = 0;
    scissor.y      = 0;
    scissor.width  = m_desc.width;
    scissor.height = m_desc.height;
    m_commandList->SetScissor(scissor);

    // Build view/projection from CameraComponent if present, otherwise use defaults
    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 3.0f, 8.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));
    float fov    = 60.0f;
    float aspect = static_cast<float>(m_desc.width) / static_cast<float>(m_desc.height);
    float nearP  = 0.1f;
    float farP   = 1000.0f;

    auto& cameras = m_registry->GetAllComponents<CameraComponent>();
    if (!cameras.empty())
    {
        const CameraComponent& cam = cameras.begin()->second;
        glm::vec3 eye    = cam.position;
        glm::vec3 target = cam.position + cam.GetForward();
        glm::vec3 up     = cam.GetUp();
        view   = glm::lookAt(eye, target, up);
        fov    = cam.fov;
        aspect = cam.aspectRatio;
        nearP  = cam.nearPlane;
        farP   = cam.farPlane;
    }

    glm::mat4 proj = glm::perspective(glm::radians(fov), aspect, nearP, farP);
    proj[1][1] *= -1.0f; // Vulkan Y flip

    auto& transformComponents = m_registry->GetAllComponents<Transform>();

    // Draw each entity that has both a mesh and a shader
    int drawCount = 0;
    for (auto& [entity, meshComp] : meshComponents)
    {
        auto shaderIt = shaderComponents.find(entity);
        if (shaderIt == shaderComponents.end())
            continue;

        const Shader* shader = shaderIt->second.shader.get();
        const Mesh*   mesh   = meshComp.mesh.get();

        if (!shader || !shader->pipeline || !mesh)
            continue;

        if (!mesh->vertexBuffer || !mesh->indexBuffer)
            continue;

        glm::mat4 model = glm::mat4(1.0f);
        auto transformIt = transformComponents.find(entity);
        if (transformIt != transformComponents.end())
            model = transformIt->second.GetModelMatrix();

        glm::mat4 mvp = proj * view * model;

        m_commandList->BindPipeline(shader->pipeline);
        m_commandList->PushConstants(shader->vertexShader, 0, sizeof(glm::mat4), &mvp);
        m_commandList->BindVertexBuffer(0, mesh->vertexBuffer, 0);
        m_commandList->BindIndexBuffer(mesh->indexBuffer, 0, false);
        m_commandList->DrawIndexed(
            static_cast<uint32_t>(mesh->indices.size()), 1, 0, 0, 0);
        ++drawCount;
    }

    m_commandList->EndRendering();

    // Transition color target back: color attachment -> transfer src (window will blit it)
    m_commandList->TextureBarrier(m_colorTarget,
        GraphicsCore::TextureUsage_RenderTarget,
        GraphicsCore::TextureUsage_TransferSrc);

    m_commandList->End();

    GraphicsCore::IWindow* iWindow = m_window ? m_window->GetIWindow() : nullptr;
    m_renderer->Submit(m_commandList, iWindow);
}