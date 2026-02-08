#include "Renderer.h"

Window& Renderer::NewWindow(const std::string& title, int width, int height)
{
	std::unique_ptr<Window> newWindow = std::make_unique<Window>(nullptr, title, width, height, 0, 0);
}
