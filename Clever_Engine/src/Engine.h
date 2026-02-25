#pragma once

#include <GLFW/glfw3.h>
#include "Render/Window/Window.h"
#include "Render/Window/Scene.h"
#include "Render/Graphics/GraphicsAPI.h"

#include "World/WorldController.h"
#include "World/AssetManager.h"

#include "World/Assets/AssetIncluder.h"

#include <stdexcept>
#include <map>
#include <memory>
#include <string>

class Engine
{
public:
    Engine();

    void SetUp(std::string setUpFilePath);

    //Default Setup
    void SetUp();
    void Tick();
    void BeginFrame();
    void EndFrame();
    void Shutdown();

    Window& CreateWindow(const std::string& title, int width, int height);
    Scene& CreateScene(uint32_t graphicsWindowId, int width, int height);

    AssetManager& GetAssetManager() { return m_AssetManager; }

private:
    std::map<int, std::unique_ptr<Window>> m_windows;
    std::map<int, std::unique_ptr<Scene>> m_scenes;

    std::shared_ptr<GraphicsAPI> m_graphicsAPI;

    AssetManager m_AssetManager;
    WorldController m_WorldController;
    std::string m_SetUpFilePath;
    
    // Add scene ID counter
    int m_nextSceneId = 0;
};