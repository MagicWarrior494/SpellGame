#pragma once
#include <glm.hpp>

#include "World/WorldController.h"
#include "Render/RenderingController.h"

struct SceneCreationInfo {
	uint8_t windowID;
	uint32_t width;
	uint32_t height;
	int posx;
	int posy;
};

class Scene
{
public:
	Scene(RenderingController& renderingController, SceneCreationInfo& creationInfo)
		: creationInfo(creationInfo)
	{
		renderSurfaceID = renderingController.CreateNewRenderSurface(creationInfo.windowID, creationInfo.width, creationInfo.height, creationInfo.posx, creationInfo.posy);
	}
	virtual ~Scene() = default;

	virtual void Update(float dt, WorldController& world) = 0;

	uint8_t GetRenderSurfaceID() const
	{
		return renderSurfaceID;
	}

protected:
	uint8_t renderSurfaceID = 0;
	SceneCreationInfo creationInfo;
};


class CameraScene : public Scene
{
public:
	CameraScene(RenderingController& renderingController, SceneCreationInfo& creationInfo, uint32_t entityID)
		: Scene(renderingController,creationInfo), cameraEntityID(entityID)
	{
	}

	void Update(float dt, WorldController& world) override
	{
	}
private:
	uint32_t cameraEntityID;
};

class UIScene : public Scene
{
public:
	UIScene(RenderingController& renderingController, SceneCreationInfo& creationInfo)
		: Scene(renderingController, creationInfo)
	{
	}

	void Update(float dt, WorldController& world) override
	{
	}
};