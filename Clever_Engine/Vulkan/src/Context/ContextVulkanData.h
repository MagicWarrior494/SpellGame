#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include <string>
#include <glm.hpp>
#include <memory>
#include <array>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "Surface/SurfaceFlags.h"

namespace Vulkan {
	inline uint32_t GetNextSurfaceID() {
		static uint32_t surfaceIDCounter = 0;
		return surfaceIDCounter++;
	}

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

		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;                         // binding index in the shader
			bindingDescription.stride = sizeof(Vertex);             // size of each vertex
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // move to next data entry per vertex
			return bindingDescription;
		}

		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
			std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
			attributeDescriptions.push_back(VkVertexInputAttributeDescription{});

			// Position attribute
			attributeDescriptions[0].binding = 0;                   // matches bindingDescription.binding
			attributeDescriptions[0].location = 0;                  // location in the shader (layout(location = 0))
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; // vec3 = 3 floats
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			return attributeDescriptions;
		}
	};

	struct VulkanImage {
		VkImage image;
		VkDeviceMemory memory;
		VkImageView view;

		VkFormat format = VK_FORMAT_UNDEFINED;
		VkExtent3D extent{};
		VkImageLayout currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkDeviceSize size = 0; // optional

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
		bool isDeviceLocal = false;
	};

	enum BufferTypes {
		VertexBuffer,
		IndexBuffer,
		UniformBuffer
	};

	enum class SwapchainAttachmentType {
		ColorOnly,     // Only swapchain images (no depth)
		ColorDepth,    // Swapchain + Depth
		ColorDepthStencil // Swapchain + Depth+Stencil
	};

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

	struct PipelineInfo {
		std::string vertShaderPath;
		std::string fragShaderPath;

		VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
		VkRenderPass renderPass = VK_NULL_HANDLE;

		// Optional settings
		VkCullModeFlags cullMode = VK_CULL_MODE_NONE;
		VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;

		bool enableBlending = false;
		bool enableDepthTest = false;

		VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;

		// Vertex input
		VkVertexInputBindingDescription bindingDescription{};
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

		// Dynamic states
		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
	};

	struct PipelineLayoutInfo {
		std::vector<VkDescriptorSetLayout> setLayouts;
		std::vector<VkPushConstantRange> pushConstants;
	};

	struct VulkanCore {
		VkInstance vkInstance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT vkDebugMesseneger = VK_NULL_HANDLE;
		VkDevice vkDevice = VK_NULL_HANDLE;
		VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
		PhysicalDeviceData d_PhysicalDeviceData;
		VkQueue graphicsQueue;
		VkQueue presentQueue;
		VkCommandPool coreCommandPool = VK_NULL_HANDLE;
		VkCommandBuffer coreCommandBuffer = VK_NULL_HANDLE;
	};

	struct SurfacePushConstants
	{
		int sceneIndex;
	};

	class VulkanSurface {
		public:
			int MAX_FRAMES_IN_FLIGHT = 2;
			SurfaceFlags flags = SurfaceFlags::None;
			SwapchainAttachmentType surfaceType = SwapchainAttachmentType::ColorDepth;

			uint8_t imageFrameCounter = 0;//This will range from 0 to {MAX_FRAMES_IN_FLIGHT}
			glm::uvec2 windowSize{0, 0};
			GLFWwindow* p_GLFWWindow = nullptr;///////////////////////////////////////////////

			VkSurfaceKHR surfaceSurface = VK_NULL_HANDLE;
			VkSwapchainKHR surfaceSwapChain = VK_NULL_HANDLE;
			VkFormat surfaceswapChainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

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
			DescriptorResult SurfaceDescriptorResult{};

			VkPipelineLayout surfacePipelineLayout = VK_NULL_HANDLE;
			VkPipeline surfacePipeline = VK_NULL_HANDLE;

			VkSampler offscreenSampler = VK_NULL_HANDLE;

			std::vector<std::vector<VulkanImage>> dummyImages{};
			std::vector<std::shared_ptr<std::vector<VulkanImage>>> offscreenImages{}; // Offscreen images for each frame in flight

			int AddNewScene(std::shared_ptr<VulkanCore> vulkanCore, uint32_t width, uint32_t height);



			void CreateSurfaceResources(std::shared_ptr<VulkanCore> vulkanCore, GLFWwindow* p_GLFWWindow);
			// Recreate swapchain (window resize) — destroys & recreates swapchain-dependent objects
			void RecreateSwapchain(std::shared_ptr<VulkanCore> core);
			// Destroy everything owned by the surface (waits device idle)
			void Destroy(std::shared_ptr<VulkanCore> vulkanCore);
		private:
			void CreateEmptyStartingDescriptors(std::shared_ptr<VulkanCore> vulkanCore, uint32_t maxSets);
	};

	struct SceneInfo {
		uint32_t width = 600;
		uint32_t height = 400;
		int32_t xoffset = 0;//Of Surface when comositing
		int32_t yoffset = 0;//Of Surface when comositing
		std::string vertShaderPath;
		std::string fragShaderPath;
		bool enableDepth = true;

	};

	class VulkanScene {
		public:
			uint8_t sceneID;
			uint32_t width = 600;
			uint32_t height = 400;
			int32_t xoffset = 0;//Of Surface when comositing
			int32_t yoffset = 0;//Of Surface when comositing

			int sceneIndex = -1; // Index of the scene in the surface's offscreenImages vector

			int* MAX_FRAMES_IN_FLIGHT = nullptr; // Pointer to surface's max frames in flight 
			uint8_t* imageFrameCounter = 0;

			//Offscreen rendering
			VkRenderPass sceneRenderPass = VK_NULL_HANDLE;
		
			std::shared_ptr<std::vector<VulkanImage>> sceneColorImage{};
			std::vector<VulkanImage> scenedepthAttachment{};

			std::vector<VkFramebuffer>  sceneOffscreenFrameBuffers{};

			VkCommandPool sceneCommandPool = VK_NULL_HANDLE;
			std::vector<VkCommandBuffer> sceneCommandBuffers{};


			std::vector<VkPipelineLayout> scenePipelineLayouts{}; 
			std::vector<VkPipeline> scenePipelines{};

			DescriptorResult SceneDescriptorResult{};

			std::vector<VkSemaphore> sceneImageAvailableSemaphores{};
			std::vector<VkSemaphore> sceneRenderFinishedSemaphores{};
			std::vector<VkFence> sceneFences{};

			std::vector<Vertex> vertexData{};
			std::vector<VulkanBuffer> sceneBuffers{};
			


			void CreateSceneResources(std::shared_ptr<VulkanCore> vulkanCore, VulkanSurface* vulkanSurface);
			void UpdateSceneSurface(std::shared_ptr<VulkanCore> vulkanCore, VulkanSurface* vulkanSurface);
			void ResizeScene(std::shared_ptr<VulkanCore> vulkanCore, VulkanSurface* vulkanSurfacePtr, uint32_t newWidth, uint32_t newHeight, uint32_t newX = 0, uint32_t newY = 0);
			// Destroy everything owned by the scene (waits device idle)
			void Destroy(VulkanCore& core);
	};
}