#include "Engine.h"
#include <iostream>
#include "World/ECS/Components.h"

#include "Render/Graphics/VulkanGrahicsAPI.h"
#define VULKAN

Engine::Engine() :
    m_WorldController(WorldController{}), m_AssetManager(AssetManager{})
{
    glfwInit();
#ifdef VULKAN
    m_graphicsAPI = std::make_shared<VulkanGraphicsAPI>();
#elif defined(OPENGL)
#endif
    m_AssetManager.SetAssetRoot("C:/Projects/Spellgame");
    m_AssetManager.SetGraphicsAPI(m_graphicsAPI);
}

void Engine::SetUp(std::string /*setUpFilePath*/)
{
}

void Engine::SetUp()
{
}

void Engine::Tick()
{
    m_WorldController.Update();
    for (auto& [id, window] : m_windows)
    {
        window->Update();
        window->Render();
    }
}

void Engine::BeginFrame()
{
}

void Engine::EndFrame()
{
}

void Engine::Shutdown()
{
}

Window& Engine::CreateWindow(const std::string& title, int width, int height)
{
    int windowId = static_cast<int>(m_windows.size());

    auto window = std::make_unique<Window>(
        m_graphicsAPI.get(),
        title,
        width,
        height
    );

    m_windows[windowId] = std::move(window);
    return *m_windows[windowId];
}

Scene& Engine::CreateScene(uint32_t graphicsWindowId, int width, int height)
{
    SceneCreationInfo info{};
    info.graphicsWindowId = graphicsWindowId;
    info.width = static_cast<uint32_t>(width);
    info.height = static_cast<uint32_t>(height);
    info.posx = 0;
    info.posy = 0;
    info.zIndex = 1;

    const uint32_t graphicsSceneId = m_graphicsAPI->CreateScene(info.graphicsWindowId);
    info.graphicsSceneId = graphicsSceneId;

    auto scene = std::make_unique<Scene>(
        m_graphicsAPI.get(),
        &m_AssetManager,
        info
    );

    int sceneId = m_nextSceneId;
    m_nextSceneId++;

    m_scenes[sceneId] = std::move(scene);

    return *m_scenes[sceneId];
}