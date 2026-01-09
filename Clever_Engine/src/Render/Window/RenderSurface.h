#pragma once
#include <string>
#include <glm.hpp>
#include <memory>
#include "Context/VulkanContext.h"


class RenderSurface 
{
public:
	RenderSurface();
private:
	uint8_t vulkanSceneId = 0;
	uint8_t parentWindowID = 0;
};