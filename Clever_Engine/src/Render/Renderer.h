#pragma once
#include <map>
#include <memory>
#include "Render/Window/Window.h"


class Renderer
{
public:
	Renderer() = default;
	~Renderer() = default;
	
	
	Window& NewWindow(const std::string& title, int width, int height);
	void CloseWindow(Window& window);
	void ResizeWindow(Window& window, int width, int height);

	Window& GetWindow(int windowId);

private:
	

	std::map<int, std::unique_ptr<Window>> m_windows;
};

