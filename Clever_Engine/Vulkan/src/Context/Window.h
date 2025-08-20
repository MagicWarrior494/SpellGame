#pragma once

#include "ContextVulkanData.h"
#include "Surface/SurfaceFlags.h"
#include <memory>

namespace Vulkan {
	struct RenderSurface
	{
		VulkanSurface vulkanSurface;
		bool resized;
		SurfaceFlags flags;
	};

	struct KeySet
	{
		std::vector<int> keys;
		std::vector<int> mouseButtons;
	};

	class Window {
	public:
		RenderSurface renderSurface;
		std::shared_ptr<VulkanCore> vulkanCore;

		std::vector<KeySet> keysets;

		Window(std::shared_ptr<VulkanCore> core, SurfaceFlags flags);

		void InitWindow(int width, int height, std::string title, int posx, int posy);  // Initialize window and OpenGL context
		void CloseWindow();                                         // Close window and unload OpenGL context
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
	};

	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		Window* windowObj = static_cast<Window*>(glfwGetWindowUserPointer(window));

		KeySet keyset{};
		keyset.keys.push_back(key);
		windowObj->keysets.push_back(keyset);

	}

	static void mouseButton_callback(GLFWwindow* window, int mouseButton, int action, int mods)
	{
		Window* windowObj = static_cast<Window*>(glfwGetWindowUserPointer(window));

		KeySet keyset{};
		keyset.mouseButtons.push_back(mouseButton);
		windowObj->keysets.push_back(keyset);
	}
}