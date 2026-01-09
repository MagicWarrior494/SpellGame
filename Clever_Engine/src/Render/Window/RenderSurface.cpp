#include "RenderSurface.h"
#include <iostream>



RenderSurface::RenderSurface(std::shared_ptr<Vulkan::VulkanContext> vulkanContext, std::string title, int width, int height, int posx, int posy)
	: v_VulkanContext(vulkanContext), title(title), width(width), posx(posx), posy(posy)
{
	

	v_RenderSurface = vulkanContext->GetRenderSurface(vulkanWindowId);
}

bool RenderSurface::IsWindowStillValid()
{
	if (v_RenderSurface == nullptr)
	{
		return false;
	}
	return true;
}

int RenderSurface::GetVulkanContextWindowId()
{
	return vulkanWindowId;
}

void RenderSurface::InitWindow()
{
	if(IsWindowStillValid())
		GetVulkanWindow()->InitRenderSurface(p_GLFWWindow);
}

void RenderSurface::CloseWindow()
{
	v_RenderSurface->CloseRenderSurface();
	if (p_GLFWWindow) {
		glfwDestroyWindow(p_GLFWWindow);
		p_GLFWWindow = nullptr;
		v_RenderSurface = nullptr;
	}
}

void RenderSurface::Update()
{
	v_RenderSurface->Render();
}

void RenderSurface::resizeScenes()
{
	v_RenderSurface->resizeScenes();
}

void RenderSurface::addTriangle()
{
	v_RenderSurface->AddRandomTriangle();
}

bool RenderSurface::WindowShouldClose()
{
	return glfwWindowShouldClose(p_GLFWWindow);
}

std::shared_ptr<Vulkan::RenderSurface> RenderSurface::GetVulkanWindow()
{
	return v_RenderSurface;
}

std::string RenderSurface::GetWindowTitle()
{
	return title;
}
