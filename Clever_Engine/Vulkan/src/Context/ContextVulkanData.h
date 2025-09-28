#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include <string>
#include <glm.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Vulkan {

	enum class ImageType : uint8_t {
		Depth,
		Color,
		Texture,
		CubeMap,
		Multisampled,
		Storage
	};

	struct PhysicalDeviceData
	{
		std::optional<uint32_t> graphicsIndex;
		std::optional<uint32_t> presentIndex;

		std::string deviceName = "Undefined";

		bool isComplete()
		{
			return graphicsIndex.has_value() && presentIndex.has_value();
		}
	};

	struct Vertex
	{
		glm::vec3 pos;
	};

	struct VulkanImage {
		VkImage image;
		VkDeviceMemory memory;
		VkImageView view;

		void Destory(VkDevice& device) {
			if (view != VK_NULL_HANDLE) {
				vkDestroyImageView(device, view, nullptr);
				view = VK_NULL_HANDLE;
			}
			if (image != VK_NULL_HANDLE) {
				vkDestroyImage(device, image, nullptr);
				image = VK_NULL_HANDLE;
			}
			if (memory != VK_NULL_HANDLE) {
				vkFreeMemory(device, memory, nullptr);
				memory = VK_NULL_HANDLE;
			}
		}
	};

	struct VulkanBuffer {
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		VkDeviceSize size{};
	};

	enum BufferTypes {
		VertexBuffer,
		IndexBuffer,
		UniformBuffer
	};

	struct VulkanCore {
		VkInstance vkInstance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT vkDebugMesseneger = VK_NULL_HANDLE;
		VkDevice vkDevice = VK_NULL_HANDLE;
		VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
		PhysicalDeviceData d_PhysicalDeviceData;
		VkQueue graphicsQueue;
		VkQueue presentQueue;
		//Typically one per thread
		std::vector<VkCommandPool> VkCommandPools{};
		int MAX_FRAMES_IN_FLIGHT = 2;
	};

	struct VulkanSurface {
		
		uint8_t imageFrameCounter = 0;//This will range from 0 to {MAX_FRAMES_IN_FLIGHT}

		glm::uvec2 windowSize;
		GLFWwindow* p_GLFWWindow;

		VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
		VkSwapchainKHR vkSwapChain = VK_NULL_HANDLE;

		/*
		* Frames in Flight, This will likely be 1-3 frames. used to Render, Present, and or transfer in parallel. Speeds things up
		* You need each one of these per VkImage for the swapchain EI The next line down
		*These are for the Swapchain
		*/

		std::vector<VulkanImage> surfaceSwapchainImages{};
		std::vector<VulkanImage> surfaceDepthImages{};

		std::vector<VkFramebuffer> surfaceVkFrameBuffers{};

		VkRenderPass vkRenderPass = VK_NULL_HANDLE;

		std::vector<VkCommandBuffer> surfaceVkCommandBuffers{};

		std::vector<VkFence> surfaceVkFences{};

		std::vector<VkSemaphore> imageAvailableSemaphores{};
		std::vector<VkSemaphore> renderFinishedSemaphores{};

		void Close(VulkanCore& vulkanCore) {
			vkDeviceWaitIdle(vulkanCore.vkDevice);

			// --- Destroy synchronization objects ---
			for (auto fence : surfaceVkFences) {
				if (fence != VK_NULL_HANDLE) {
					vkDestroyFence(vulkanCore.vkDevice, fence, nullptr);
				}
			}
			surfaceVkFences.clear();

			for (auto semaphore : imageAvailableSemaphores) {
				if (semaphore != VK_NULL_HANDLE) {
					vkDestroySemaphore(vulkanCore.vkDevice, semaphore, nullptr);
				}
			}
			imageAvailableSemaphores.clear();

			for (auto semaphore : renderFinishedSemaphores) {
				if (semaphore != VK_NULL_HANDLE) {
					vkDestroySemaphore(vulkanCore.vkDevice, semaphore, nullptr);
				}
			}
			renderFinishedSemaphores.clear();

			// --- Destroy framebuffers ---
			for (auto framebuffer : surfaceVkFrameBuffers) {
				if (framebuffer != VK_NULL_HANDLE) {
					vkDestroyFramebuffer(vulkanCore.vkDevice, framebuffer, nullptr);
				}
			}
			surfaceVkFrameBuffers.clear();

			// --- Destroy render pass ---
			if (vkRenderPass != VK_NULL_HANDLE) {
				vkDestroyRenderPass(vulkanCore.vkDevice, vkRenderPass, nullptr);
				vkRenderPass = VK_NULL_HANDLE;
			}

			// --- Destroy swapchain ---
			if (vkSwapChain != VK_NULL_HANDLE) {
				vkDestroySwapchainKHR(vulkanCore.vkDevice, vkSwapChain, nullptr);
				vkSwapChain = VK_NULL_HANDLE;
			}

			// --- Destroy surface ---
			if (vkSurface != VK_NULL_HANDLE) {
				vkDestroySurfaceKHR(vulkanCore.vkInstance, vkSurface, nullptr);
				vkSurface = VK_NULL_HANDLE;
			}
		}
	};

	struct VulkanScene {

		uint8_t sceneID;

		//Make large enough for all of the Descriptor sets
		VkDescriptorPool vkDesciptorPool = VK_NULL_HANDLE;

		//need 1 per unique descriptor configuration
		std::vector<VkDescriptorSetLayout> sceneVkDesciptoreSetLayouts{};

		//1 per object type/material OR per frame for uniform buffers.
		std::vector<VkDescriptorSet> sceneVkDescriptorSets{};

		//need 1 per unique descriptor layout configuration
		std::vector<VkPipelineLayout> sceneVkPipelineLayouts{};

		//need 1 per unique shader/stage/render state combination
		std::vector<VkPipeline> sceneVkPipelines{};

		std::vector<Vertex> vertexData{};
	
		//1 per vertex/index/uniform buffer
		std::vector<VulkanBuffer> sceneVkBuffers{};

		//1 per texture/depth image/off-screen render target
		std::vector<VulkanImage> sceneVkImages{};

		//1 per texture type
		std::vector<VkSampler> sceneVkSamplers{};
	};
}