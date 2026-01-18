#pragma once
#include <memory>
#include <map>
#include <type_traits>

#include "Event/EventController.h"
#include "Scene.h"

// SceneID was defined as uint8_t in Window.h
using SceneID = uint8_t;

class SceneController
{
public:
    // Pass the EventController pointer from the Window that owns this controller
    SceneController(EventController* eventController);
    ~SceneController() = default;

    void Update();

    /**
     * Creates a new scene, stores it, and automatically attaches it
     * to the EventController's layer stack.
     */
    template<typename SceneType, typename... Args>
    void CreateNewScene(uint8_t vulkanSurfaceID, SceneCreationInfo info, Args&&... args)
    {
        static_assert(std::is_base_of_v<Scene, SceneType>,
            "SceneType must derive from Scene");

        // 1. Create the scene instance
        auto scene = std::make_unique<SceneType>(info, std::forward<Args>(args)...);

        // 2. Register with Event System if the scene implements IInputLayer
        // This allows the scene to receive mouse/key events based on its Z-index
        if (m_EventController) {
            m_EventController->AttachLayer(scene.get());
        }

        // 3. Store and return
        m_Scenes.insert({ vulkanSurfaceID, std::move(scene) });
    }

    void DeleteScene(SceneID sceneID);

    Scene& GetScene(SceneID sceneID)
    {
        return *m_Scenes.at(sceneID).get();
    }

private:
    // VulkanSceneID mapped to Scene Type
    std::map<SceneID, std::unique_ptr<Scene>> m_Scenes;

    // Non-owning observer of the Window's EventController
    EventController* m_EventController = nullptr;
};