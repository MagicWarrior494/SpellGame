#pragma once
#include <cstdint>

namespace Vulkan {
	enum class SurfaceFlags : uint32_t {
		None = 0,
		EnableDepth = 1 << 0,  // Create depth image and attach to framebuffer
		EnableStencil = 1 << 1,  // Use depth-stencil format instead of depth-only
		EnableMSAA = 1 << 2,  // Enable multisample anti-aliasing (MSAA)
		EnableVSync = 1 << 3,  // Use FIFO present mode for vertical sync
		EnableHDR = 1 << 4,  // Use HDR swapchain format (if supported)
		EnableTripleBuffer = 1 << 5,  // Use 3 images in swapchain instead of 2
		EnableInputAttachment = 1 << 6, // Allow input attachments for subpasses
		OffscreenSurface = 1 << 7,   // Create a surface for offscreen rendering (no presentation)

		Fullscreen = 1 << 9, // Starts Fullscreen
		Fullscreenable = 1 << 10, // Renders the top bar on the window
		Resizeable = 1 << 11, // Allows for the render surface to be resizeable
	};

	inline SurfaceFlags operator|(SurfaceFlags a, SurfaceFlags b) {
		return static_cast<SurfaceFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
	}
	inline SurfaceFlags operator&(SurfaceFlags a, SurfaceFlags b) {
		return static_cast<SurfaceFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
	}
	inline SurfaceFlags& operator|=(SurfaceFlags& a, SurfaceFlags b) {
		a = a | b;
		return a;
	}
}