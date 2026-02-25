#pragma once
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "World/WorldController.h"
#include "Event/EventController.h"

#include "Render/Graphics/GraphicsAPI.h"
#include "World/ECS/Registry.h"
#include "Scene/RenderStage.h"
#include "Render/Window/WindowControls.h"
#include "World/AssetManager.h"

struct SceneCreationInfo {
    uint32_t graphicsWindowId = 0;
    uint32_t graphicsSceneId = 0;

    uint32_t width = 0;
    uint32_t height = 0;
    int posx = 0;
    int posy = 0;
    int zIndex = 1;
};

class Scene : public IInputLayer
{
public:
    Scene(GraphicsAPI* graphicsAPI, AssetManager* assetManager, SceneCreationInfo info)
        : m_GraphicsAPI(graphicsAPI)
        , m_AssetManager(assetManager)
        , m_SceneInfo(info)
        , m_GraphicsSceneId(info.graphicsSceneId)
    {
        m_Registry = std::make_unique<Registry>();
    }

    void Update();
    void Render();
    void ExecuteRenderStage(const RenderStage& stage);

    Registry& GetRegistry() { return *m_Registry; }
    const SceneCreationInfo& GetSceneInfo() const { return m_SceneInfo; }
    uint32_t GetGraphicsSceneId() const { return m_GraphicsSceneId; }

    virtual void OnInput(InputEvent& event) override
    {
        if (event.type == InputEvent::Type::Key &&
            (event.action == Input::Action::PRESS || event.action == Input::Action::REPEAT))
        {
        }
    }

    virtual int GetZIndex() const override { return m_SceneInfo.zIndex; }

private:
    SceneCreationInfo m_SceneInfo;
    uint32_t m_GraphicsSceneId = 0;

    std::vector<RenderStage> m_RenderStages;

    std::unique_ptr<Registry> m_Registry;
    GraphicsAPI* m_GraphicsAPI = nullptr;
    AssetManager* m_AssetManager = nullptr;
};