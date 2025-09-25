#pragma once
#include <string>
#include <glm.hpp>
#include <memory>
#include "Context/VulkanContext.h"
#include "Event/Io/KeySet.h"
#include "Event/Io/ConversionData.h"

class Window 
{

public:
	Window(std::shared_ptr<Vulkan::VulkanContext> vulkanContext, std::string title, int width, int height, int posx, int posy);

public:
	KeySet keyset;

	std::string title;
	uint8_t width = 0;
	uint8_t height = 0;
	uint8_t posx = 0;
	uint8_t posy = 0;

	GLFWwindow* p_GLFWWindow;
public:
	bool IsWindowStillValid();
	int GetVulkanContextWindowId();
	std::shared_ptr<Vulkan::RenderSurface> GetVulkanWindow();


	inline void ClearKeySets() { keyset.clear(); }
	inline KeySet GetKeySet() { return keyset; }

	void InitWindow();  // Initialize window and OpenGL context
	void CloseWindow();                                         // Close window and unload OpenGL context
	void Update();
	
	bool WindowShouldClose();                                   // Check if application should close (KEY_ESCAPE pressed or windows close icon clicked)
	bool IsWindowReady();                                       // Check if window has been initialized successfully
	bool IsWindowFullscreen();                                  // Check if window is currently fullscreen
	bool IsWindowHidden();                                      // Check if window is currently hidden
	bool IsWindowMinimized();                                   // Check if window is currently minimized
	bool IsWindowMaximized();                                   // Check if window is currently maximized
	bool IsWindowFocused();                                     // Check if window is currently focused
	bool IsWindowResized();                                     // Check if window has been resized last frame
	bool IsWindowState(uint32_t flag);                          // Check if one specific window flag is enabled
	void SetWindowState(uint32_t flags);                        // Set window configuration state using flags
	void ClearWindowState(uint32_t flags);                      // Clear window configuration state flags
	void ToggleFullscreen();                                    // Toggle window state: fullscreen/windowed, resizes monitor to match window resolution
	void ToggleBorderlessWindowed();                            // Toggle window state: borderless windowed, resizes window to match monitor resolution
	void MaximizeWindow();                                      // Set window state: maximized, if resizable
	void MinimizeWindow();                                      // Set window state: minimized, if resizable
	void RestoreWindow();                                       // Set window state: not minimized/maximized
	//void SetWindowIcon(Image image);                            // Set icon for window (single image, RGBA 32bit)
	//void SetWindowIcons(Image* images, int count);              // Set icon for window (multiple images, RGBA 32bit)
	void SetWindowTitle(std::string title);                     // Set title for window
	void SetWindowPosition(int x, int y);                       // Set window position on screen
	void SetWindowMonitor(int monitor);                         // Set monitor for the current window
	void SetWindowMinSize(int width, int height);               // Set window minimum dimensions (for FLAG_WINDOW_RESIZABLE)
	void SetWindowMaxSize(int width, int height);               // Set window maximum dimensions (for FLAG_WINDOW_RESIZABLE)
	void SetWindowSize(int width, int height);                  // Set window dimensions
	void SetWindowOpacity(float opacity);                       // Set window opacity [0.0f..1.0f]
	void SetWindowFocused();                                    // Set window focused
	std::string GetWindowTitle();                               // Get window title
	void* GetWindowHandle();                                    // Get native window handle
	int GetScreenWidth();                                       // Get current screen width
	int GetScreenHeight();                                      // Get current screen height
	int GetRenderWidth();                                       // Get current render width (it considers HiDPI)
	int GetRenderHeight();                                      // Get current render height (it considers HiDPI)
	int GetMonitorCount();                                      // Get number of connected monitors
	int GetCurrentMonitor();                                    // Get current monitor where window is placed
	//Vector2 GetMonitorPosition(int monitor);                    // Get specified monitor position
	int GetMonitorWidth(int monitor);                           // Get specified monitor width (current video mode used by monitor)
	int GetMonitorHeight(int monitor);                          // Get specified monitor height (current video mode used by monitor)
	//int GetMonitorPhysicalWidth(int monitor);                   // Get specified monitor physical width in millimetres
	//int GetMonitorPhysicalHeight(int monitor);                  // Get specified monitor physical height in millimetres
	int GetMonitorRefreshRate(int monitor);                     // Get specified monitor refresh rate
	glm::vec2 GetWindowPosition();                            // Get window position XY on monitor
	glm::vec2 GetWindowScaleDPI();                            // Get window scale DPI factor
	std::string GetMonitorName(int monitor);                    // Get the human-readable, UTF-8 encoded name of the specified monitor
	void SetClipboardText(const char* text);                    // Set clipboard text content
	std::string GetClipboardText();                         // Get clipboard text content
	//Image GetClipboardImage(void);                              // Get clipboard image
	void EnableEventWaiting();                              // Enable waiting for events on EndDrawing(), no automatic event polling
	void DisableEventWaiting();

private:
	Vulkan::SurfaceFlags defaultVulkanWindowFlags = Vulkan::SurfaceFlags::EnableDepth | Vulkan::SurfaceFlags::Resizeable | Vulkan::SurfaceFlags::Fullscreenable;

	std::shared_ptr<Vulkan::RenderSurface> renderSurface;
	std::shared_ptr<Vulkan::VulkanContext> vulkanContext;
	uint8_t vulkanWindowId;
};

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	Window* windowObj = static_cast<Window*>(glfwGetWindowUserPointer(window));

	windowObj->keyset.keys.push_back((InputCodes::Keyboard)keyboardGLFWtoCleverKeyCodes.at(key));

}

static void mouseButton_callback(GLFWwindow* window, int mouseButton, int action, int mods)
{
	Window* windowObj = static_cast<Window*>(glfwGetWindowUserPointer(window));
	windowObj->keyset.mouseButtons.push_back((InputCodes::Mouse)mouseGLFWtoCleverKeyCodes.at(mouseButton));
}