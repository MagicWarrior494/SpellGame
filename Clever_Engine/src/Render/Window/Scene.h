#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include <chrono>
#include <array>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "Event/EventController.h"
#include "IRenderer.h"
#include "World/ECS/Registry.h"
#include "Scene/RenderStage.h"
#include "Render/Window/WindowControls.h"
#include "World/AssetManager.h"

#ifndef CLEVER_ENGINE_API
    #ifdef _WIN32
        #define CLEVER_ENGINE_API __declspec(dllexport)
    #else
        #define CLEVER_ENGINE_API
    #endif
#endif

struct CLEVER_ENGINE_API SceneDesc
{
    uint32_t width = 0;
    uint32_t height = 0;
    int      posX = 0;
    int      posY = 0;
    int      zIndex = 1;
};

class Window;


class CLEVER_ENGINE_API Scene : public IInputLayer
{
public:
    Scene(GraphicsCore::IRenderer* renderer,
        AssetManager* assetManager,
        Window* window,
        const SceneDesc& desc);

    ~Scene();

    void Update();
    void Render();
    void Resize(uint32_t width, uint32_t height);
    void AttachToWindow(Window& window);

    GraphicsCore::ITexture* GetColorTarget() const { return m_colorTarget; }
    GraphicsCore::ITexture* GetDepthTarget() const { return m_depthTarget; }

    const SceneDesc& GetDesc()     const { return m_desc; }
    Registry& GetRegistry() { return *m_registry; }
    EventController& GetEventController() { return *m_eventController; }

    void OnInput(InputEvent& event) override;
    int  GetZIndex() const override { return m_desc.zIndex; }

    void ResetInputState();

private:
    SceneDesc m_desc;

    GraphicsCore::IRenderer* m_renderer = nullptr;
    AssetManager* m_assetManager = nullptr;
    Window* m_window = nullptr;

    GraphicsCore::ITexture* m_colorTarget = nullptr;
    GraphicsCore::ITexture* m_depthTarget = nullptr;
    GraphicsCore::ICommandList* m_commandList = nullptr;

    std::unique_ptr<Registry>       m_registry;
    std::unique_ptr<EventController> m_eventController;
    std::vector<RenderStage>        m_renderStages;

    // Camera input state
    std::array<bool, 6> m_keysHeld = {};  // W S A D Q E
    bool    m_mouseLocked  = false;
    float   m_mouseDeltaX  = 0.0f;
    float   m_mouseDeltaY  = 0.0f;
    std::chrono::steady_clock::time_point m_lastFrameTime;
    bool  m_firstFrame = true;
};