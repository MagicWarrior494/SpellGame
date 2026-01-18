#include "RenderingController.h"
#include "World/ECS/Components.h"

RenderingController::RenderingController()
{
}

RenderingController::~RenderingController() = default;

void RenderingController::Update()
{
	glfwPollEvents();
	for (auto& [windowID, window] : windows)
	{
		if (glfwWindowShouldClose(window->GetGLFWWindowPtr()))
		{
			window->CloseWindow();
			windows.erase(windowID);
			break;
		}
		else if (!window->IsWindowStillValid())
		{
			window->CloseWindow();
			windows.erase(windowID);
			break;
		}
	}

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
	uint8_t renderSurfaceID = windows.at(windowID)->CreateNewScene(width, height, posx, posy);
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

RenderSurface& RenderingController::GetRenderSurface(uint8_t renderSurfaceID)
{
	if (renderSurfaces.find(renderSurfaceID) != renderSurfaces.end()) {
		return *(renderSurfaces.at(renderSurfaceID));
	}
	throw std::runtime_error("Render Surface ID not found in RenderingController");
}

Window& RenderingController::GetWindow(uint8_t windowID)
{
	if (windows.find(windowID) != windows.end()) {
		return *(windows.at(windowID));
	}
	throw std::runtime_error("Window ID not found in RenderingController");
}

std::map<uint8_t, std::unique_ptr<Window>>& RenderingController::GetAllWindows()
{
	return windows;
}