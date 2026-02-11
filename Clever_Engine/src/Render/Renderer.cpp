#include "Renderer.h"

void Renderer::Update()
{
    std::vector<int> windowsToClose;

    // 1. Identify which windows need to close
    for (auto& [id, window] : m_windows)
    {
        if (!window->ShouldWindowClose())
        {
            window->Update();
        }
        else
        {
            windowsToClose.push_back(id);
        }
    }

    // 2. Safely close them after the loop is done
    for (int id : windowsToClose)
    {
        // Find the window pointer in the map using the ID
        auto it = m_windows.find(id);
        if (it != m_windows.end())
        {
            CloseWindow(*it->second);
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
