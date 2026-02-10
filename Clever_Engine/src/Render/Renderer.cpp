#include "Renderer.h"

void Renderer::Update()
{
	for (auto& [id, window] : m_windows)
	{
		if (!window->ShouldWindowClose())
		{
			window->Update();
		}
		else
		{
			CloseWindow(*window);
		}
	}
}

Window& Renderer::NewWindow(const std::string& title, int width, int height)
{
	std::unique_ptr<Window> newWindow = std::make_unique<Window>(m_graphicsAPI.get(), title, width, height, 0, 0);
	int windowId = newWindow->GetWindowID();
	m_windows[windowId] = std::move(newWindow);
	return *m_windows[windowId];
}

void Renderer::CloseWindow(Window& window)
{
	int windowId = window.GetWindowID();
	auto it = m_windows.find(windowId);
	if (it != m_windows.end())
	{
		// First, close the window through the GraphicsAPI
		m_graphicsAPI->CloseWindow(windowId);
		m_windows.erase(it);
	}
}
