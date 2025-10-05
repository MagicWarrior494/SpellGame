#pragma once
#include <cstdint>
#include <glm.hpp>
#include <stdexcept>
#include <memory>
#include <map>

#include "ContextVulkanData.h"
#include "Surface/SurfaceFlags.h"
#include "RenderSurface.h"


namespace Vulkan 
{

	/*
	This is what "owns" the Vulkan Data. The Engine is what modify and does the function calling but this owns the data
	*/
	class VulkanContext 
	{
	public:
		VulkanContext() = default;
		void Init();
		void Update();

		uint8_t CreateNewWindow(SurfaceFlags flags);

		//RETURNS NULLPTR if not found
		inline std::shared_ptr<RenderSurface> GetRenderSurface(uint8_t id)
		{
			if (renderSurfaces.find(id) != renderSurfaces.end()) {
				return renderSurfaces.at(id);
			}
			return nullptr;
		}

	private:

		std::shared_ptr<VulkanCore> vulkanCore;
		std::map<uint8_t, std::shared_ptr<RenderSurface>> renderSurfaces;
	};
}