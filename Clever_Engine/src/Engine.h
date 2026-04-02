#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "IRenderer.h"
#include "Render/Window/Window.h"
#include "Render/Window/Scene.h"
#include "Render/Window/ISceneLayer.h"
#include "UI/UIScene.h"
#include "World/WorldController.h"
#include "World/AssetManager.h"
#include "World/Assets/AssetIncluder.h"

#ifdef _WIN32
    #define CLEVER_ENGINE_API __declspec(dllexport)
#else
    #define CLEVER_ENGINE_API
#endif

class CLEVER_ENGINE_API Engine
{
public:
    Engine();
    ~Engine();

    void SetUp();
    void Tick();
    void BeginFrame();
    void EndFrame();
    void Shutdown();

    bool IsRunning()    const { return m_running; }
    void RequestClose()       { m_running = false; }

    Window&   CreateWindow(const std::string& title, int width, int height);
    Scene&    CreateScene(Window& window, int width, int height, int xpos = 0, int ypos = 0);
    UIScene&  CreateUIScene(Window& window, int width, int height, int xpos = 0, int ypos = 0);

    AssetManager& GetAssetManager() { return m_assetManager; }

private:
    // Transfer a layer from whichever window currently owns it.
    // screenCursorX/Y is where the cursor is in desktop pixels when the drag fires.
    void OnLayerDetached(ISceneLayer* layer, int screenCursorX, int screenCursorY);

    // Remove a layer from a window's scene list and unregister it.
    // Returns the window id that owned it, or -1.
    int  DetachLayerFromWindow(ISceneLayer* layer);

    // Attach a layer to a window's scene list and register it.
    void AttachLayerToWindow(int windowId, ISceneLayer* layer);

    // Register the detach callback on every window so Engine stays notified.
    void BindDetachCallback(int windowId);

    // Register the dock callback on a single-layer window so Engine can merge
    // it into another window when released over one (without destroying the window
    // when no target is found, unlike the full detach path).
    void BindWindowDockCallback(int windowId);

    // Fired when a single-layer window finishes a window-drag: dock the layer
    // into the window under the cursor if one exists, otherwise do nothing.
    void OnWindowDockAttempt(ISceneLayer* layer, int screenCursorX, int screenCursorY);

private:
    std::unique_ptr<GraphicsCore::IRenderer>        m_renderer;

    std::map<int, std::unique_ptr<Window>>           m_windows;
    std::map<int, std::unique_ptr<Scene>>            m_scenes;
    std::map<int, std::unique_ptr<UIScene>>          m_uiScenes;
    std::map<int, std::vector<ISceneLayer*>>         m_windowScenes; // window id -> layers

    AssetManager    m_assetManager;
    WorldController m_worldController;

    int  m_nextWindowId = 0;
    int  m_nextSceneId  = 0;
    bool m_running      = true;
};