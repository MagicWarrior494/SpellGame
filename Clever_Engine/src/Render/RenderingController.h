#pragma once
#include <stdexcept>
#include <memory>
#include <vector>
#include <string>
#include <map>
#include <iostream>

#include "Window/RenderSurface.h"
#include "Window/Window.h"
#include "Context/VulkanContext.h"

#include "Scene/SceneController.h"
#include "World/WorldController.h"

class RenderingController
{
public:
	RenderingController() = default;
	~RenderingController() = default;

public:
	void Update();
	void SetUp();

	void Render(const SceneController& sceneController, const WorldController& worldController);

	int CreateNewRenderSurface(uint8_t windowID, uint32_t width, uint32_t height, int posx = 0, int posy = 0);
	int CreateNewWindow(std::string title, uint32_t width, uint32_t height, int posx = 0, int posy = 0);
	void DeleteRenderSurface(int renderSurfaceID);

	void RenderAll();

	uint8_t GetNextRenderSurfaceID() { return nextRenderSurfaceID++; }

private:
	uint8_t nextRenderSurfaceID = 1;

	std::map<uint8_t, std::unique_ptr<RenderSurface>> renderSurfaces;
	std::map<uint8_t, std::unique_ptr<Window>> windows;
	std::shared_ptr<Vulkan::VulkanContext> vulkanContext;
};