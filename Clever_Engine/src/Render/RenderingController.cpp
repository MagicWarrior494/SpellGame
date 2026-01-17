#include "RenderingController.h"

#include "World/ECS/Components.h"

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
	storageBufferManager = std::make_unique<StorageBufferManager>(vulkanContext);
}

void RenderingController::Render(Registry& reg)
{
	auto& transforms = reg.GetAllComponents<Transform>();

	for (auto& [entityId, transform] : transforms)
	{
		if (transform.isDirty)
		{
			transform.UpdateModelMatrix();
		}
	}

	storageBufferManager->SyncDeletions<glm::mat4>({});
	storageBufferManager->SyncUpdates(transforms, [](const Transform& t){
		return t.modelMatrix;
	});

	for (auto& [windowID, window] : windows)
	{
		window->Render();
	}
}

uint8_t RenderingController::CreateNewRenderSurface(uint8_t windowID, uint32_t width, uint32_t height, int posx, int posy)
{
	uint8_t renderSurfaceID = windows.at(windowID)->CreateNewRenderSurface(width, height, posx, posy);
	RenderSurface renderSurface{ renderSurfaceID, width, height, posx, posy };
	renderSurface.setParentWindowID(windowID);
	renderSurfaces.insert({ renderSurfaceID, std::make_unique<RenderSurface>(renderSurface) });
	return renderSurfaceID;
}

uint8_t RenderingController::CreateNewWindow(std::string title, uint32_t width, uint32_t height, int posx, int posy)
{
	auto newWindow = std::make_unique<Window>(vulkanContext, title, width, height, posx, posy);
	newWindow->InitWindow();
	uint8_t windowID = newWindow->GetWindowID();
	windows.insert({ windowID, std::move(newWindow) });
	return windowID;
}