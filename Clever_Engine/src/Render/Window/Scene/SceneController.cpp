#include "SceneController.h"
#include <algorithm>

/**
 * @brief Construct a new Scene Controller
 * @param eventController Pointer to the Window's event system for input layer registration
 */
SceneController::SceneController(EventController* eventController)
    : m_EventController(eventController)
{
}

/**
 * @brief Updates all active scenes. This should be called once per frame by the Window.
 */
void SceneController::Update()
{
    // We use a manual iterator or a copy if scenes might be deleted DURING an update
    for (auto const& [id, scene] : m_Scenes)
    {
        if (scene)
        {
            scene->Update();
        }
    }
}

/**
 * @brief Deletes a specific scene by its ID.
 * @param sceneID The uint8_t ID (RenderSurfaceID) of the scene to remove.
 */
void SceneController::DeleteScene(SceneID sceneID)
{
    auto it = m_Scenes.find(sceneID);

    if (it != m_Scenes.end())
    {
        m_EventController->DetachLayer(it->second.get());
        m_Scenes.erase(it);
    }
}

/**
 * Note: CreateNewScene is implemented in the header because it is a template function.
 * C++ requires template definitions to be visible to the compiler at the call site.
 */