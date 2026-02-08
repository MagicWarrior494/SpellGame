#pragma once
#include <vulkan/vulkan.h>
#include <glm.hpp>
#include <vector>
#include <map>
#include <stdint.h>

#include "Flags.h"
#include "Image/VulkanImage.h"
#include "Buffer/VulkanBuffer.h"
#include "VulkanCore.h"
#include "VulkanScene.h"

namespace Vulkan
{
	struct DescriptorBindingInfo
	{
		uint32_t binding = 0;
		VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		VkShaderStageFlags stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		std::vector<VkDescriptorBufferInfo> buffers; // For uniform/storage buffers
		std::vector<VkDescriptorImageInfo> images;   // For sampled images/samplers
		uint32_t count = 1;
	};

	struct DescriptorSetInfo {
		std::vector<DescriptorBindingInfo> bindings;
		uint32_t maxSets = 1; // how many sets we want
	};

	struct DescriptorResult {
		VkDescriptorSetLayout layout = VK_NULL_HANDLE;
		VkDescriptorPool pool;
		std::vector<VkDescriptorSet> sets;
	};

	struct SurfacePushConstants
	{
		int sceneIndex;
	};

	class VulkanWindow
	{
	public:
		VulkanWindow(std::shared_ptr<VulkanCore> vulkanCore, void* windowPtr);
		~VulkanWindow();

		void RecreateSwapchain();
		bool Render();

	private:
		std::shared_ptr<VulkanCore> vulkanCore = nullptr;

		std::vector<std::unique_ptr<VulkanScene>> vulkanScenes{};

		glm::uvec2 windowSize{ 0, 0 };
		int MAX_FRAMES_IN_FLIGHT = 2;

		int MAX_SCENES = 16;

		//TODO: Make configurable
		SurfaceFlags flags = SurfaceFlags::None;
		
		SwapchainAttachmentType surfaceType = SwapchainAttachmentType::ColorDepth;

		uint8_t imageFrameCounter = 0;
		void* windowPtr = nullptr;

		VkSurfaceKHR surfaceSurface = VK_NULL_HANDLE;
		VkSwapchainKHR surfaceSwapChain = VK_NULL_HANDLE;
		VkFormat surfaceSwapChainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

		std::vector<VulkanImage> surfaceColorImages{};
		std::vector<VulkanImage> surfaceDepthImages{};
		VkRenderPass surfaceRenderPass = VK_NULL_HANDLE;

		std::vector<VkFramebuffer> surfaceFrameBuffers{};

		VkCommandPool surfaceCommandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> surfacePresentCommandBuffers{};


		std::vector<VkSemaphore> surfaceImageAvailableSemaphores{};
		std::vector<VkSemaphore> surfaceRenderFinishedSemaphores{};
		std::vector<VkFence> surfaceFences{};

		DescriptorSetInfo descriptorSetInfo{};
		DescriptorResult surfaceDescriptorResult{};

		VkPipelineLayout surfacePipelineLayout = VK_NULL_HANDLE;
		VkPipeline surfacePipeline = VK_NULL_HANDLE;

		VkSampler offscreenSampler = VK_NULL_HANDLE;

		//Need to create here so  do not need to recreate every time a new scene is needed
		std::vector<std::vector<VulkanImage>> sceneImages{};

 		std::vector<std::shared_ptr<std::vector<VulkanImage>>> offscreenImages{}; // Offscreen images for each frame in flight
	};
}