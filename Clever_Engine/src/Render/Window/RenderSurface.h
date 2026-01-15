#pragma once
#include <string>
#include <glm.hpp>
#include <memory>
#include "Context/VulkanContext.h"


class RenderSurface 
{
public:
	RenderSurface(uint8_t vulkanSceneId, uint32_t width, uint32_t height, int posx, int posy);

	void Update();
	void setParentWindowID(uint8_t id) { parentWindowID = id; }
	uint8_t getParentWindowID() const { return parentWindowID; }
private:
	uint8_t vulkanSceneId = 0;
	uint8_t parentWindowID = 0;

	uint32_t width = 0;
	uint32_t height = 0;
	int posx = 0;
	int posy = 0;
};