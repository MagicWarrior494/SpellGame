#include "Engine.h"
#include "VulkanRenderer.h"
#include "World/Assets/StorageBufferComponent.h"
#include <filesystem>
#include <algorithm>

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
    m_renderer->PollEvents();

    // ---------------------------------------------------------------
    // Single-layer rule: when a window has exactly one layer, that
    // layer always fills the window completely (position 0,0 and
    // matching dimensions). This keeps things tidy after a tear-off.
    // ---------------------------------------------------------------
    for (auto& [id, window] : m_windows)
    {
        auto it = m_windowScenes.find(id);
        if (it == m_windowScenes.end()) continue;
        std::vector<ISceneLayer*>& layers = it->second;
        if (layers.size() == 1 && layers[0])
        {
            ISceneLayer* layer = layers[0];
            uint32_t wW = window->GetWidth();
            uint32_t wH = window->GetHeight();
            const SceneDesc& sd = layer->GetDesc();
            if (sd.posX != 0 || sd.posY != 0)
                layer->SetPosition(0, 0);
            if (sd.width != wW || sd.height != wH)
                layer->Resize(wW, wH);
        }
    }

    // ---------------------------------------------------------------
    // Destroy windows that have no layers left.
    // Collect ids first so we don't mutate while iterating.
    // ---------------------------------------------------------------
    std::vector<int> deadWindows;
    for (auto& [id, window] : m_windows)
    {
        auto it = m_windowScenes.find(id);
        if (it != m_windowScenes.end() && it->second.empty())
            deadWindows.push_back(id);
    }
    if (!deadWindows.empty())
        m_renderer->WaitIdle();
    for (int id : deadWindows)
    {
        m_windowScenes.erase(id);
        m_windows.erase(id);
    }

    // ---------------------------------------------------------------
    // Multi-layer window-shrink rule: if a window has multiple layers
    // and the window has become smaller than some layers, first push any
    // layer that overhangs back into the visible area, then shrink the
    // widest/tallest offender until everything fits.
    // ---------------------------------------------------------------
    for (auto& [id, window] : m_windows)
    {
        auto it = m_windowScenes.find(id);
        if (it == m_windowScenes.end()) continue;
        std::vector<ISceneLayer*>& layers = it->second;
        if (layers.size() <= 1) continue; // single-layer rule already handles this

        int winW = static_cast<int>(window->GetWidth());
        int winH = static_cast<int>(window->GetHeight());

        // Pass 1 — clamp position: move any layer whose origin has been pushed
        // outside the window back to the nearest in-bounds corner.
        for (ISceneLayer* layer : layers)
        {
            if (!layer) continue;
            const SceneDesc& sd = layer->GetDesc();
            int newX = sd.posX;
            int newY = sd.posY;
            // Clamp so the top-left corner stays inside the window.
            newX = std::max(0, std::min(newX, winW - 1));
            newY = std::max(0, std::min(newY, winH - 1));
            if (newX != sd.posX || newY != sd.posY)
                layer->SetPosition(newX, newY);
        }

        // Pass 2 — shrink: repeatedly shrink the worst offender until all
        // layers fit within the window bounds.
        bool changed = true;
        while (changed)
        {
            changed = false;
            ISceneLayer* worstW = nullptr; int maxOverflowW = 0;
            ISceneLayer* worstH = nullptr; int maxOverflowH = 0;
            for (ISceneLayer* layer : layers)
            {
                if (!layer) continue;
                const SceneDesc& sd = layer->GetDesc();
                int overflowW = sd.posX + static_cast<int>(sd.width)  - winW;
                int overflowH = sd.posY + static_cast<int>(sd.height) - winH;
                if (overflowW > maxOverflowW) { maxOverflowW = overflowW; worstW = layer; }
                if (overflowH > maxOverflowH) { maxOverflowH = overflowH; worstH = layer; }
            }
            if (worstW)
            {
                const SceneDesc& sd = worstW->GetDesc();
                uint32_t newW = static_cast<uint32_t>(std::max(1, static_cast<int>(sd.width) - maxOverflowW));
                worstW->Resize(newW, sd.height);
                changed = true;
            }
            if (worstH)
            {
                const SceneDesc& sd = worstH->GetDesc();
                uint32_t newH = static_cast<uint32_t>(std::max(1, static_cast<int>(sd.height) - maxOverflowH));
                worstH->Resize(sd.width, newH);
                changed = true;
            }
        }
    }

    m_worldController.Update();
    for (auto& [id, window] : m_windows)
    {
        auto it = m_windowScenes.find(id);
        if (it != m_windowScenes.end())
        {
            for (ISceneLayer* layer : it->second)
                if (layer) layer->Update();
            window->Render(it->second);
        }
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

    // Destroy StorageBufferComponent GPU buffers for every entity in every scene.
    // These raw IBuffer* pointers are owned by the component, not the AssetManager,
    // so they must be explicitly freed before the VMA allocator is torn down.
    for (auto& [id, scene] : m_scenes)
    {
        if (!scene) continue;
        auto& ssboMap = scene->GetRegistry().GetAllComponents<StorageBufferComponent>();
        for (auto& [entity, ssbo] : ssboMap)
            ssbo.Destroy(*m_renderer);
    }

    m_uiScenes.clear();
    m_scenes.clear();
    m_windows.clear();
    m_assetManager.ReleaseAllGPUResources();
    m_renderer.reset();
}

// ---------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------

void Engine::BindDetachCallback(int windowId)
{
    auto it = m_windows.find(windowId);
    if (it == m_windows.end()) return;
    Window* win = it->second.get();
    win->SetLayerDetachCallback([this](ISceneLayer* layer, int sx, int sy)
    {
        OnLayerDetached(layer, sx, sy);
    });
}

void Engine::BindWindowDockCallback(int windowId)
{
    auto it = m_windows.find(windowId);
    if (it == m_windows.end()) return;
    Window* win = it->second.get();
    win->SetWindowDockCallback([this](ISceneLayer* layer, int sx, int sy)
    {
        OnWindowDockAttempt(layer, sx, sy);
    });
}

void Engine::OnWindowDockAttempt(ISceneLayer* layer, int screenCursorX, int screenCursorY)
{
    // Find which window currently owns this layer (the single-layer source window).
    int sourceWindowId = -1;
    for (auto& [id, layers] : m_windowScenes)
    {
        auto it = std::find(layers.begin(), layers.end(), layer);
        if (it != layers.end()) { sourceWindowId = id; break; }
    }

    // Check if the cursor is inside a *different* existing window's client area.
    for (auto& [id, window] : m_windows)
    {
        if (!window) continue;
        if (id == sourceWindowId) continue; // skip own window
        int wx = 0, wy = 0;
        window->GetScreenPosition(wx, wy);
        int wW = static_cast<int>(window->GetWidth());
        int wH = static_cast<int>(window->GetHeight());
        if (screenCursorX >= wx && screenCursorX < wx + wW &&
            screenCursorY >= wy && screenCursorY < wy + wH)
        {
            // Detach from source and attach to target window.
            DetachLayerFromWindow(layer);
            layer->SetPosition(screenCursorX - wx, screenCursorY - wy);
            AttachLayerToWindow(id, layer);
            // The source window is now empty and will be cleaned up by the
            // dead-window pass in Tick().
            return;
        }
    }
    // No target window found — leave the layer in its current window, do nothing.
}

int Engine::DetachLayerFromWindow(ISceneLayer* layer)
{
    for (auto& [id, layers] : m_windowScenes)
    {
        auto layerIt = std::find(layers.begin(), layers.end(), layer);
        if (layerIt != layers.end())
        {
            layers.erase(layerIt);
            // Unregister from the Window object so it stops routing events to it
            auto winIt = m_windows.find(id);
            if (winIt != m_windows.end())
                winIt->second->UnregisterScene(layer);
            return id;
        }
    }
    return -1;
}

void Engine::AttachLayerToWindow(int windowId, ISceneLayer* layer)
{
    auto winIt = m_windows.find(windowId);
    if (winIt == m_windows.end()) return;
    m_windowScenes[windowId].push_back(layer);
    Window* win = winIt->second.get();
    layer->SetWindow(win);
    win->RegisterScene(layer);
}

void Engine::OnLayerDetached(ISceneLayer* layer, int screenCursorX, int screenCursorY)
{
    // Remove from current window and remember which window it came from
    // so we don't immediately re-attach to the source window.
    int sourceWindowId = DetachLayerFromWindow(layer);

    // Check if the cursor is inside an existing window's client area
    for (auto& [id, window] : m_windows)
    {
        if (!window) continue;
        if (id == sourceWindowId) continue; // never drop back onto the source
        int wx = 0, wy = 0;
        window->GetScreenPosition(wx, wy);
        int wW = static_cast<int>(window->GetWidth());
        int wH = static_cast<int>(window->GetHeight());
        if (screenCursorX >= wx && screenCursorX < wx + wW &&
            screenCursorY >= wy && screenCursorY < wy + wH)
        {
            // Drop into this existing window, position relative to it
            layer->SetPosition(screenCursorX - wx, screenCursorY - wy);
            AttachLayerToWindow(id, layer);
            return;
        }
    }

    // No existing window hit — create a new one sized to the layer
    const SceneDesc& sd = layer->GetDesc();
    int newWinId = m_nextWindowId++;

    auto newWindow = std::make_unique<Window>(
        m_renderer.get(),
        "SpellGame",
        sd.width,
        sd.height);

    // Position the new OS window so the layer appears under the cursor
    // Offset by the layer's own position within itself (0,0 after tear-off)
    newWindow->SetScreenPosition(screenCursorX, screenCursorY);
    newWindow->SetCloseCallback([this]() { RequestClose(); });

    m_windows[newWinId] = std::move(newWindow);
    m_windowScenes[newWinId] = {};

    // Layer position is (0,0) inside the new window
    layer->SetPosition(0, 0);
    AttachLayerToWindow(newWinId, layer);
    BindDetachCallback(newWinId);
    BindWindowDockCallback(newWinId);
}

// ---------------------------------------------------------------
// Public factory methods
// ---------------------------------------------------------------

Window& Engine::CreateWindow(const std::string& title, int width, int height)
{
    const int id = m_nextWindowId++;

    auto window = std::make_unique<Window>(
        m_renderer.get(),
        title,
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height));

    window->SetCloseCallback([this]() { RequestClose(); });

    m_windows[id] = std::move(window);
    m_windowScenes[id] = {};

    BindDetachCallback(id);
    BindWindowDockCallback(id);

    return *m_windows[id];
}

Scene& Engine::CreateScene(Window& window, int width, int height, int xpos, int ypos)
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
    desc.posX = xpos;
    desc.posY = ypos;
    desc.zIndex = static_cast<int>(m_windowScenes[windowId].size()) + 1;

    auto scene = std::make_unique<Scene>(m_renderer.get(), &m_assetManager, &window, desc);
    Scene* scenePtr = scene.get();
    scenePtr->AttachToWindow(window);

    m_scenes[sceneId] = std::move(scene);

    if (windowId >= 0)
        m_windowScenes[windowId].push_back(scenePtr);

    return *scenePtr;
}

UIScene& Engine::CreateUIScene(Window& window, int width, int height, int xpos, int ypos)
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
    desc.width  = static_cast<uint32_t>(width);
    desc.height = static_cast<uint32_t>(height);
    desc.posX   = xpos;
    desc.posY   = ypos;
    desc.zIndex = static_cast<int>(m_windowScenes[windowId].size()) + 1;

    auto uiScene = std::make_unique<UIScene>(m_renderer.get(), &m_assetManager, &window, desc);
    UIScene* uiPtr = uiScene.get();
    uiPtr->AttachToWindow(window);

    m_uiScenes[sceneId] = std::move(uiScene);

    if (windowId >= 0)
        m_windowScenes[windowId].push_back(uiPtr);

    return *uiPtr;
}