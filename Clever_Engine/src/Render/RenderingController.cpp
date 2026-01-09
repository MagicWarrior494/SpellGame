#include "RenderingController.h"

void RenderingController::Update()
{
	for (auto& [id, renderSurface] : renderSurfaces)
	{
		renderSurface->Update();
	}
}

void RenderingController::SetUp()
{
	vulkanContext = std::make_shared<Vulkan::VulkanContext>();
	vulkanContext->Init();
}
void RenderingController::Render(const SceneController& sceneController, const WorldController& worldController)
{
	//TO-DO: Implement rendering logic here
}
int RenderingController::CreateNewRenderSurface(uint8_t windowID, uint32_t width, uint32_t height, int posx, int posy)
{
	//TO-DO: Implement Creating A New Render Surface
}

int RenderingController::CreateNewWindow(std::string title, uint32_t width, uint32_t height, int posx, int posy)
{
	auto newWindow = std::make_unique<Window>(vulkanContext, title, width, height, posx, posy);
	newWindow->InitWindow();
	uint8_t windowID = newWindow->GetWindowID();
	windows.insert({ windowID, std::move(newWindow) });
	return windowID;
}