#pragma once
#include <memory>
#include <map>

#include "Render/RenderingController.h"
#include "Scene.h"

class SceneController
{
public:
	SceneController() = default;
	~SceneController() = default;
public:
	void Update();
	//Creates a new scene
	template<typename SceneType, typename... Args>
	SceneType& CreateNewScene(RenderingController& renderingController, SceneCreationInfo info, Args&&... args)
	{
		static_assert(std::is_base_of_v<Scene, SceneType>,
			"SceneType must derive from Scene");

		auto scene = std::make_unique<SceneType>(renderingController, info, std::forward<Args>(args)...);

		SceneType& ref = *scene;
		scenes.insert({ ref.GetRenderSurfaceID(), std::move(scene) });
		return ref;
	}

	//Deletes a scene and will NOT delete the window even if it is the last scene in that window
	void DeleteScene(int sceneID);

	Scene& GetScene(int sceneID)
	{
		return *scenes.at(sceneID);
	}

private:
	//RenderSurface ID mapped to Scene Type
	std::map<uint8_t, std::unique_ptr<Scene>> scenes;
};