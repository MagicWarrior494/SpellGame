#pragma once
#include <stdexcept>
#include <memory>
#include <vector>
#include <string>
#include <map>
#include <iostream>

#include "Window/RenderSurface.h"
#include "Context/VulkanContext.h"
#include "BufferManager.h"
#include "World/ECS/Registry.h"
#include "Window/Window.h"

class RenderingController
{
public:
	RenderingController();
	~RenderingController();

public:
	void Update();
	void SetUp();
	void Render(Registry& reg);

	//Creates a new window, does NOT create a render surface, that is done separately when creating a scene
	uint8_t CreateNewWindow(std::string title, uint32_t width, uint32_t height, int posx = 0, int posy = 0);

	//Deletes render Surface, this should NOT be called directly, only when deleting a scene will a render surface be deleted
	void DeleteRenderSurface(int renderSurfaceID);

	uint8_t GetNextRenderSurfaceID() { return nextRenderSurfaceID++; }

	RenderSurface& GetRenderSurface(uint8_t renderSurfaceID);

	Window& GetWindow(uint8_t windowID);

	std::map<uint8_t, std::unique_ptr<Window>>& GetAllWindows();

	int GetWindowCount()
	{
		return static_cast<int>(windows.size());
	}

private:
	uint8_t nextRenderSurfaceID = 1;

	std::map<uint8_t, std::unique_ptr<RenderSurface>> renderSurfaces;
	std::map<uint8_t, std::unique_ptr<Window>> windows;
	std::shared_ptr<Vulkan::VulkanContext> vulkanContext;

	std::unique_ptr<StorageBufferManager> storageBufferManager;
};