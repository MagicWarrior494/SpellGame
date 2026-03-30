#include "Engine.h"
#include "VulkanRenderer.h"
#include <filesystem>

Engine::Engine()
    : m_worldController(WorldController{})
    , m_assetManager(AssetManager{})
{
    m_renderer = std::make_unique<GraphicsCore::VulkanRenderer>();

    // Resolve Assets/ relative to the executable directory
    std::filesystem::path exeDir = std::filesystem::current_path();
    std::filesystem::path assetRoot = exeDir / "Assets";
    if (!std::filesystem::exists(assetRoot))
        assetRoot = exeDir / "../Assets"; // fallback one level up

    m_assetManager.SetAssetRoot(std::filesystem::weakly_canonical(assetRoot));
    m_assetManager.SetRenderer(m_renderer.get());
}

Engine::~Engine()
{
    Shutdown();
}

void Engine::SetUp()
{
}

void Engine::Tick()
{
    m_worldController.Update();
    for (auto& [id, window] : m_windows)
    {
        window->Update();

        auto it = m_windowScenes.find(id);
        if (it != m_windowScenes.end())
            window->Render(it->second);
        else
            window->Render({});
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
    if (!m_renderer)
        return;

    m_renderer->WaitIdle();

    m_scenes.clear();
    m_windows.clear();
    m_renderer.reset();
}

Window& Engine::CreateWindow(const std::string& title, int width, int height)
{
    const int id = m_nextWindowId++;

    auto window = std::make_unique<Window>(
        m_renderer.get(),
        title,
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height));

    m_windows[id] = std::move(window);
    m_windowScenes[id] = {};
    return *m_windows[id];
}

Scene& Engine::CreateScene(Window& window, int width, int height)
{
    const int sceneId = m_nextSceneId++;

    int windowId = -1;
    for (auto& [id, w] : m_windows)
    {
        if (w.get() == &window)
        {
            windowId = id;
            break;
        }
    }

    SceneDesc desc{};
    desc.width = static_cast<uint32_t>(width);
    desc.height = static_cast<uint32_t>(height);
    desc.posX = 0;
    desc.posY = 0;
    desc.zIndex = static_cast<int>(m_windowScenes[windowId].size()) + 1;

    auto scene = std::make_unique<Scene>(m_renderer.get(), &m_assetManager, &window, desc);
    Scene* scenePtr = scene.get();

    m_scenes[sceneId] = std::move(scene);

    if (windowId >= 0)
        m_windowScenes[windowId].push_back(scenePtr);

    return *scenePtr;
}