#pragma once
#include <cstdint>
#include <iostream>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp> // Required for glm::lookAt and glm::perspective

#include "World/WorldController.h"
#include "Event/EventController.h"

#include "Render/Graphics/GraphicsAPI.h"
#include "World/ECS/Registry.h"
#include "Scene/RenderStage.h"

#include "Render/Window/WindowControls.h"

struct SceneCreationInfo {
    uint8_t windowID;
	uint8_t sceneID;
    uint32_t width;
    uint32_t height;
    int posx;
    int posy;
    int zIndex = 1; // Default Z-Index for input priority
};

class Scene : public IInputLayer
{
public:
    Scene(GraphicsAPI* graphicsAPI, SceneCreationInfo info)
        : m_GraphicsAPI(graphicsAPI)
    {
        //graphicsAPI.CreateScene();
	}

    void Update();

	void Render();

	Registry& GetRegistry() { return *m_Registry; }

    virtual void OnInput(InputEvent& event) override
    {
        if (event.type == InputEvent::Type::Key &&
            (event.action == Input::Action::PRESS || event.action == Input::Action::REPEAT))
        {}
    }

private:
	SceneCreationInfo m_SceneInfo;
    
	std::vector<RenderStage> m_RenderStages;//This should be rendered starting from the front

	std::unique_ptr<Registry> m_Registry = nullptr;
	GraphicsAPI* m_GraphicsAPI = nullptr;
};