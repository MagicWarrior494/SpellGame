#pragma once
#include <glm.hpp>

#include "CoreTypes/VulkanWindow.h"

namespace Vulkan
{
	class VulkanScene
	{
	public:
		VulkanScene(std::shared_ptr<VulkanCore> vulkanCore, int width, int height, int xpos, int ypos, VulkanWindow* parentWindow);
		~VulkanScene();

		bool Render();

	public:
		uint8_t m_sceneId;
		uint32_t width = 600;
		uint32_t height = 400;
		int32_t xoffset = 0;//Of Surface
		int32_t yoffset = 0;//Of Surface

		uint8_t* imageFrameCounter = 0;

		VkRenderPass sceneRenderPass = VK_NULL_HANDLE;

		std::vector<VulkanImage> scenedepthAttachment{};

		std::vector<VkFramebuffer>  sceneFrameBuffers{};

		//This need to be moved to be created through VulkanGraphics API becuase each scene has a unique pipeline layout and pipeline
		/*
		* std::vector<VkPipelineLayout> scenePipelineLayouts{};
		std::vector<VkPipeline> scenePipelines{};
		DescriptorResult SceneDescriptorResult{};
		*/

		std::vector<VkSemaphore> sceneImageAvailableSemaphores{};
		std::vector<VkSemaphore> sceneRenderFinishedSemaphores{};
		std::vector<VkFence> sceneFences{};
	private:
		std::shared_ptr<VulkanCore> vulkanCore = nullptr;

		VulkanWindow* parentWindow = nullptr;
		int renderImageIndex = 0;//The index of SceneImages in the parents window
	};
}