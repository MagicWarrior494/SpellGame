#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "IRenderer.h"
#include "Render/Window/Window.h"
#include "Render/Window/Scene.h"
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

    Window& CreateWindow(const std::string& title, int width, int height);
    Scene&  CreateScene(Window& window, int width, int height, int xpos = 0, int ypos = 0);

    AssetManager& GetAssetManager() { return m_assetManager; }

private:
    std::unique_ptr<GraphicsCore::IRenderer>        m_renderer;

    std::map<int, std::unique_ptr<Window>>           m_windows;
    std::map<int, std::unique_ptr<Scene>>            m_scenes;
    std::map<int, std::vector<Scene*>>               m_windowScenes; // window id -> scenes

    AssetManager    m_assetManager;
    WorldController m_worldController;

    int  m_nextWindowId = 0;
    int  m_nextSceneId  = 0;
    bool m_running      = true;
};