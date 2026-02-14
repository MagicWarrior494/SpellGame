#pragma once
#include <glm.hpp>

#include "CoreTypes/VulkanWindow.h"

namespace Vulkan
{
	class ISceneSystem {
	public:
		virtual void Record(
			VkCommandBuffer cmd,
			uint32_t frameIndex,
			const ResourceMap& resources, // Map of Buffers/Images
			const DescriptorResult& descriptors
		) = 0;
	};

	class VulkanScene
	{
	public:
		VulkanScene(std::shared_ptr<VulkanCore> vulkanCore, int width, int height, int xpos, int ypos, VulkanWindow* parentWindow);
		~VulkanScene();

		bool Render();

		void ConstructPipeline(
			VulkanCore* VC,
			PipelineInfo& info,
			const ResourceMap& resourceMap);

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

		VkPipelineLayout scenePipelineLayout = VK_NULL_HANDLE;
		VkPipeline scenePipeline = VK_NULL_HANDLE;
		DescriptorResult sceneDescriptorResult{};
		
		std::vector<VkSemaphore> sceneImageAvailableSemaphores{};
		std::vector<VkSemaphore> sceneRenderFinishedSemaphores{};
		std::vector<VkFence> sceneFences{};
	private:
		std::shared_ptr<VulkanCore> vulkanCore = nullptr;

		VulkanWindow* parentWindow = nullptr;
		int renderImageIndex = 0;//The index of SceneImages in the parents window
	};
}