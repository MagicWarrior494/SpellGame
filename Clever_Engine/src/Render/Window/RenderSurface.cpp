#include "RenderSurface.h"

RenderSurface::RenderSurface(uint8_t vulkanSceneId, uint32_t width, uint32_t height, int posx, int posy)
	: vulkanSceneId(vulkanSceneId), width(width), height(height), posx(posx), posy(posy)
{
}

void RenderSurface::Update()
{
	// Currently no dynamic properties to update
}