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
    SceneType& CreateNewScene(uint8_t vulkanSurfaceID, SceneCreationInfo info, Args&&... args)
    {
        static_assert(std::is_base_of_v<Scene, SceneType>,
            "SceneType must derive from Scene");
        auto scene = std::make_unique<SceneType>(info, std::forward<Args>(args)...);
        auto [it, inserted] = m_Scenes.insert({ vulkanSurfaceID, std::move(scene) });
        return static_cast<SceneType&>(*it->second);
    }

    void DeleteScene(SceneID sceneID);

    Scene& GetScene(SceneID sceneID)
    {
        return *m_Scenes.at(sceneID).get();
    }

private:
    uint8_t m_ActiveScene = 0;

    // VulkanSceneID mapped to Scene Type
    std::map<SceneID, std::unique_ptr<Scene>> m_Scenes;

    // Non-owning observer of the Window's EventController
    EventController* m_EventController = nullptr;
};