#include "ContextVulkanData.h"

#include <filesystem>
#include <iostream>

#include "Surface/CreateVulkanSurface.h"
#include "Surface/CreateSwapChain.h"
#include "Surface/CreateSwapChainImages.h"
#include "Surface/CreateRenderpass.h"
#include "Surface/CreateCommandPool.h"
#include "Surface/CreateCommandBuffers.h"
#include "Surface/CreateSyncObjects.h"
#include "Surface/CreateFrameBuffers.h"
#include "Surface/CreateRenderResources.h"

#include "Scene/CreateDescriptors.h"
#include "Scene/CreatePipelines.h"

#include <filesystem>

namespace Vulkan {
	void VulkanSurface::CreateSurfaceResources(std::shared_ptr<VulkanCore> vulkanCore, GLFWwindow* p_GLFWWindow)
	{
		this->p_GLFWWindow = p_GLFWWindow;
		int width = 0, height = 0;
		glfwGetFramebufferSize(p_GLFWWindow, &width, &height);
		windowSize = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
		CreateVulkanRenderSurface(vulkanCore, p_GLFWWindow, surfaceSurface);
		surfaceRenderPass = CreateRenderPass(
			vulkanCore,
			VK_FORMAT_B8G8R8A8_UNORM,
			false, // depth?
			VK_FORMAT_D32_SFLOAT,
			RenderPassType::Surface
		);
		SwapChainCreateInfo swapChainCreateInfo{};
		swapChainCreateInfo.p_GLFWWindow = p_GLFWWindow;
		swapChainCreateInfo.surface = surfaceSurface;
		swapChainCreateInfo.windowSize = windowSize;
		swapChainCreateInfo.swapChainImageFormat = surfaceswapChainImageFormat;
		swapChainCreateInfo.flags = flags;
		CreateSwapchain(vulkanCore, swapChainCreateInfo, surfaceSwapChain);
		CreateSwapchainImages(vulkanCore, this, SwapchainAttachmentType::ColorOnly);
		CreateFrameBuffers(
			vulkanCore,
			surfaceRenderPass,
			surfaceColorImages,
			surfaceDepthImages,
			surfaceFrameBuffers,
			windowSize.x,
			windowSize.y
		);
		surfaceCommandPool = CreateCommandPool(vulkanCore);
		CreateCommandBuffers(
			vulkanCore,
			surfaceCommandPool,
			static_cast<uint32_t>(surfaceFrameBuffers.size()),
			surfacePresentCommandBuffers
		);
		CreateSyncObjects(
			vulkanCore,
			MAX_FRAMES_IN_FLIGHT,
			surfaceImageAvailableSemaphores,
			surfaceRenderFinishedSemaphores,
			surfaceFences
		);

		offscreenSampler = CreateOffscreenSampler(vulkanCore);
		CreateEmptyStartingDescriptors(vulkanCore, 16);

		// --- 5. Recreate pipeline layout & pipeline (push constants unchanged) ---
		PipelineLayoutInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.setLayouts.push_back(SurfaceDescriptorResult.layout);

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SurfacePushConstants);

		pipelineLayoutInfo.pushConstants.push_back(pushConstantRange);
		surfacePipelineLayout = CreatePipelineLayout(vulkanCore, pipelineLayoutInfo);

		std::cout << std::filesystem::current_path().string() << std::endl;

		PipelineInfo pipelineInfo{};
		pipelineInfo.vertShaderPath = std::filesystem::current_path().string() + "/SpellGame_Solution/Clever_Engine/Vulkan/res/surfaceVert.spv";
		pipelineInfo.fragShaderPath = std::filesystem::current_path().string() + "/SpellGame_Solution/Clever_Engine/Vulkan/res/surfaceFrag.spv";
		pipelineInfo.pipelineLayout = surfacePipelineLayout;
		pipelineInfo.renderPass = surfaceRenderPass;
		pipelineInfo.cullMode = VK_CULL_MODE_NONE;
		surfacePipeline = CreateGraphicsPipeline(vulkanCore, pipelineInfo);
	}

	int VulkanSurface::AddNewScene(std::shared_ptr<VulkanCore> vulkanCore, uint32_t width, uint32_t height)
	{
		VulkanCore& VC = *vulkanCore;

		// --- 1. Create offscreen images for each frame in flight ---
		std::vector<VulkanImage> newSceneImages = initImageByType(
			VC,
			ImageType::Color,
			width,
			height,
			MAX_FRAMES_IN_FLIGHT,
			VK_SAMPLE_COUNT_1_BIT,
			VK_FORMAT_UNDEFINED
		);

		for (auto& img : newSceneImages) {
			TransitionImageLayout(
				vulkanCore,
				img.image,
				img.format,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_IMAGE_ASPECT_COLOR_BIT
			);
		}

		std::shared_ptr<std::vector<VulkanImage>> newSceneImagesPtr = std::make_shared<std::vector<VulkanImage>>(std::move(newSceneImages));

		offscreenImages.push_back(newSceneImagesPtr);
		int sceneIndex = static_cast<int>(offscreenImages.size()) - 1;

		DescriptorBindingInfo& binding = descriptorSetInfo.bindings[0];

		// --- 3. Append new scene images to the existing binding ---
		for (uint32_t frame = 0; frame < MAX_FRAMES_IN_FLIGHT; ++frame)
		{
			VkDescriptorImageInfo info{};
			info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			info.imageView = offscreenImages[sceneIndex]->at(frame).view;
			info.sampler = offscreenSampler;

			binding.images[(sceneIndex * MAX_FRAMES_IN_FLIGHT) + frame] = info;
		}

		// Update descriptor count to match total number of images per frame
		binding.count = static_cast<uint32_t>(binding.images.size());

		// --- 4. Set maxSets ---
		descriptorSetInfo.maxSets = MAX_FRAMES_IN_FLIGHT;

		SurfaceDescriptorResult = CreateDescriptors(vulkanCore, descriptorSetInfo);

		return sceneIndex;
	}


	void VulkanSurface::RecreateSwapchain(std::shared_ptr<VulkanCore> core)
	{
		VulkanCore& vulkanCore = *core;

		// --- 0. Wait until window has a valid size ---
		int width = 0, height = 0;
		glfwGetFramebufferSize(p_GLFWWindow, &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(p_GLFWWindow, &width, &height);
			glfwWaitEvents();
		}
		windowSize = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

		vkDeviceWaitIdle(vulkanCore.vkDevice);
		vkWaitForFences(vulkanCore.vkDevice, MAX_FRAMES_IN_FLIGHT, surfaceFences.data(), VK_TRUE, UINT64_MAX);

		// Destroy framebuffers
		for (auto framebuffer : surfaceFrameBuffers)
		{
			if (framebuffer != VK_NULL_HANDLE)
				vkDestroyFramebuffer(vulkanCore.vkDevice, framebuffer, nullptr);
		}
		surfaceFrameBuffers.clear();

		// Destroy depth images
		for (auto& image : surfaceDepthImages)
		{
			if (image.view != VK_NULL_HANDLE) vkDestroyImageView(vulkanCore.vkDevice, image.view, nullptr);
			if (image.image != VK_NULL_HANDLE) vkDestroyImage(vulkanCore.vkDevice, image.image, nullptr);
			if (image.memory != VK_NULL_HANDLE) vkFreeMemory(vulkanCore.vkDevice, image.memory, nullptr);
		}
		surfaceDepthImages.clear();

		// Destroy swapchain
		if (surfaceSwapChain != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(vulkanCore.vkDevice, surfaceSwapChain, nullptr);
			surfaceSwapChain = VK_NULL_HANDLE;
		}

		SwapChainCreateInfo swapChainCreateInfo{};
		swapChainCreateInfo.p_GLFWWindow = p_GLFWWindow;
		swapChainCreateInfo.surface = surfaceSurface;
		swapChainCreateInfo.windowSize = windowSize;
		swapChainCreateInfo.swapChainImageFormat = surfaceswapChainImageFormat;
		swapChainCreateInfo.flags = flags;
		CreateSwapchain(core, swapChainCreateInfo, surfaceSwapChain);
		CreateSwapchainImages(core, this, SwapchainAttachmentType::ColorOnly);
		CreateFrameBuffers(
			core,
			surfaceRenderPass,
			surfaceColorImages,
			surfaceDepthImages,
			surfaceFrameBuffers,
			windowSize.x,
			windowSize.y
		);
	}
	void VulkanSurface::Destroy(std::shared_ptr<VulkanCore> vulkanCore)
	{
		vkDeviceWaitIdle(vulkanCore->vkDevice);

		// --- Destroy synchronization objects ---
		for (auto fence : surfaceFences) {
			if (fence != VK_NULL_HANDLE) {
				vkDestroyFence(vulkanCore->vkDevice, fence, nullptr);
			}
		}
		surfaceFences.clear();

		for (auto semaphore : surfaceImageAvailableSemaphores) {
			if (semaphore != VK_NULL_HANDLE) {
				vkDestroySemaphore(vulkanCore->vkDevice, semaphore, nullptr);
			}
		}
		surfaceImageAvailableSemaphores.clear();

		for (auto semaphore : surfaceRenderFinishedSemaphores) {
			if (semaphore != VK_NULL_HANDLE) {
				vkDestroySemaphore(vulkanCore->vkDevice, semaphore, nullptr);
			}
		}
		surfaceRenderFinishedSemaphores.clear();

		// --- Destroy framebuffers ---
		for (auto framebuffer : surfaceFrameBuffers) {
			if (framebuffer != VK_NULL_HANDLE) {
				vkDestroyFramebuffer(vulkanCore->vkDevice, framebuffer, nullptr);
			}
		}
		surfaceFrameBuffers.clear();

		// --- Destroy render pass ---
		if (surfaceRenderPass != VK_NULL_HANDLE) {
			vkDestroyRenderPass(vulkanCore->vkDevice, surfaceRenderPass, nullptr);
			surfaceRenderPass = VK_NULL_HANDLE;
		}

		// --- Destroy swapchain ---
		if (surfaceSwapChain != VK_NULL_HANDLE) {
			vkDestroySwapchainKHR(vulkanCore->vkDevice, surfaceSwapChain, nullptr);
			surfaceSwapChain = VK_NULL_HANDLE;
		}

		// --- Destroy surface ---
		if (surfaceSurface != VK_NULL_HANDLE) {
			vkDestroySurfaceKHR(vulkanCore->vkInstance, surfaceSurface, nullptr);
			surfaceSurface = VK_NULL_HANDLE;
		}
	}

	void VulkanSurface::CreateEmptyStartingDescriptors(std::shared_ptr<VulkanCore> VC, uint32_t arraySize)
	{
		descriptorSetInfo.bindings.clear();
		descriptorSetInfo.maxSets = MAX_FRAMES_IN_FLIGHT;

		DescriptorBindingInfo bindinginfo{};
		bindinginfo.binding = 0;
		bindinginfo.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		bindinginfo.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		bindinginfo.count = 0;
		descriptorSetInfo.bindings.push_back(bindinginfo);

		DescriptorBindingInfo& binding = descriptorSetInfo.bindings[0];

		for (int i = 0; i < arraySize; i++)
		{
			// --- 1. Create offscreen images for each frame in flight ---
			std::vector<VulkanImage> newSceneImages = initImageByType(
				*VC,
				ImageType::Color,
				1,
				1,
				MAX_FRAMES_IN_FLIGHT,
				VK_SAMPLE_COUNT_1_BIT,
				VK_FORMAT_UNDEFINED
			);

			for (auto& img : newSceneImages) {
				TransitionImageLayout(
					VC,
					img.image,
					img.format,
					VK_IMAGE_LAYOUT_UNDEFINED,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					VK_IMAGE_ASPECT_COLOR_BIT
				);
			}

			dummyImages.push_back(newSceneImages);

			for (int frame = 0; frame < MAX_FRAMES_IN_FLIGHT; frame++) {
				VkDescriptorImageInfo info{};
				info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				info.imageView = newSceneImages[frame].view;
				info.sampler = offscreenSampler;

				binding.images.push_back(info);
			}
		}
		binding.count = static_cast<uint32_t>(binding.images.size());

		SurfaceDescriptorResult = CreateDescriptors(VC, descriptorSetInfo);
	}

	void VulkanScene::CreateSceneResources(std::shared_ptr<VulkanCore> vulkanCore, VulkanSurface* vulkanSurface)
	{
		MAX_FRAMES_IN_FLIGHT = &vulkanSurface->MAX_FRAMES_IN_FLIGHT;
		imageFrameCounter = &vulkanSurface->imageFrameCounter;

		sceneIndex = vulkanSurface->AddNewScene(vulkanCore, width, height);

		sceneColorImage = vulkanSurface->offscreenImages[sceneIndex];

		sceneRenderPass = CreateRenderPass(
			vulkanCore,
			VK_FORMAT_B8G8R8A8_UNORM,
			false,
			VK_FORMAT_D32_SFLOAT,
			RenderPassType::Offscreen
		);

		CreateFrameBuffers(
			vulkanCore,
			sceneRenderPass,
			*sceneColorImage,
			{ scenedepthAttachment },
			sceneOffscreenFrameBuffers,
			width,
			height
		);

		sceneCommandPool = CreateCommandPool(vulkanCore);

		CreateCommandBuffers(vulkanCore, sceneCommandPool, static_cast<uint32_t>(sceneOffscreenFrameBuffers.size()), sceneCommandBuffers);

		DescriptorSetInfo info{};
		info.maxSets = *MAX_FRAMES_IN_FLIGHT;

		// 1. Prepare the buffer info for the descriptor
		VkDescriptorBufferInfo storageBufferInfo{};
		storageBufferInfo.buffer = vulkanCore->persistentData.objectMatrixStorageBuffer.buffer;
		storageBufferInfo.offset = 0;
		storageBufferInfo.range = VK_WHOLE_SIZE;

		// 2. Create the binding info
		DescriptorBindingInfo matrixBinding{};
		matrixBinding.binding = 0;
		matrixBinding.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		matrixBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		// 3. IMPORTANT: Add the buffer info for EVERY frame in flight
		// If you use one global buffer for all frames:
		for (int i = 0; i < info.maxSets; i++) {
			matrixBinding.buffers.push_back(storageBufferInfo);
		}

		info.bindings.push_back(matrixBinding);
		//info.bindings.push_back({ 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT });

		sceneDescriptorResult = CreateDescriptors(vulkanCore, info);

		PipelineLayoutInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.setLayouts.push_back(sceneDescriptorResult.layout);
		scenePipelineLayouts.push_back(CreatePipelineLayout(vulkanCore, pipelineLayoutInfo));

		PipelineInfo pipelineInfo{};
		pipelineInfo.vertShaderPath = std::filesystem::current_path().string() + "/SpellGame_Solution/Clever_Engine/Vulkan/res/sceneVert.spv";
		pipelineInfo.fragShaderPath = std::filesystem::current_path().string() + "/SpellGame_Solution/Clever_Engine/Vulkan/res/sceneFrag.spv";
		pipelineInfo.pipelineLayout = scenePipelineLayouts[0];
		pipelineInfo.renderPass = sceneRenderPass;
		pipelineInfo.bindingDescription = Vertex::getBindingDescription();
		pipelineInfo.attributeDescriptions = Vertex::getAttributeDescriptions();
		pipelineInfo.cullMode = VK_CULL_MODE_NONE;
		scenePipelines.push_back(CreateGraphicsPipeline(vulkanCore, pipelineInfo));

		CreateSyncObjects(
			vulkanCore,
			*MAX_FRAMES_IN_FLIGHT,
			sceneImageAvailableSemaphores,
			sceneRenderFinishedSemaphores,
			sceneFences
		);
	}

	void VulkanScene::ResizeScene(std::shared_ptr<VulkanCore> vulkanCore, VulkanSurface* vulkanSurfacePtr, uint32_t newWidth, uint32_t newHeight, uint32_t newX, uint32_t newY)
	{
		width = newWidth;
		height = newHeight;
		xoffset = newX;
		yoffset = newY;

		VkDevice device = vulkanCore->vkDevice;

		VulkanSurface& vulkanSurface = *vulkanSurfacePtr;

		for (size_t i = 0; i < sceneFences.size(); ++i) {
			vkWaitForFences(device, 1, &sceneFences[i], VK_TRUE, UINT64_MAX);
		}

		for (auto& fb : sceneOffscreenFrameBuffers) {
			vkDestroyFramebuffer(device, fb, nullptr);
		}
		for (auto& img : *sceneColorImage) {
			vkDestroyImageView(device, img.view, nullptr);
			vkDestroyImage(device, img.image, nullptr);
			vkFreeMemory(device, img.memory, nullptr);
		}

		std::vector<VulkanImage> newSceneImages = initImageByType(
			*vulkanCore,
			ImageType::Color,
			width,
			height,
			*MAX_FRAMES_IN_FLIGHT,
			VK_SAMPLE_COUNT_1_BIT,
			VK_FORMAT_UNDEFINED
		);

		for (auto& img : newSceneImages) {
			TransitionImageLayout(
				vulkanCore,
				img.image,
				img.format,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_IMAGE_ASPECT_COLOR_BIT
			);
		}

		std::shared_ptr<std::vector<VulkanImage>> newSceneImagesPtr = std::make_shared<std::vector<VulkanImage>>(std::move(newSceneImages));
		
		vulkanSurface.offscreenImages.at(sceneIndex) = newSceneImagesPtr;
		sceneColorImage = vulkanSurface.offscreenImages[sceneIndex];

		DescriptorBindingInfo& binding = vulkanSurface.descriptorSetInfo.bindings[0];

		// --- 3. Append new scene images to the existing binding ---
		for (uint32_t frame = 0; frame < *MAX_FRAMES_IN_FLIGHT; ++frame)
		{
			VkDescriptorImageInfo info{};
			info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			info.imageView = vulkanSurface.offscreenImages[sceneIndex]->at(frame).view;
			info.sampler = vulkanSurface.offscreenSampler;

			binding.images[(sceneIndex * *MAX_FRAMES_IN_FLIGHT) + frame] = info;
		}

		// Update descriptor count to match total number of images per frame
		binding.count = static_cast<uint32_t>(binding.images.size());

		// --- 4. Set maxSets ---
		vulkanSurface.descriptorSetInfo.maxSets = *MAX_FRAMES_IN_FLIGHT;

		UpdateDescriptorSets(vulkanCore, vulkanSurface.descriptorSetInfo, vulkanSurface.SurfaceDescriptorResult);

		CreateFrameBuffers(
			vulkanCore,
			sceneRenderPass,
			*sceneColorImage,
			{ scenedepthAttachment },
			sceneOffscreenFrameBuffers,
			width,
			height
		);
	}

	void VulkanScene::UpdateSceneSurface(std::shared_ptr<VulkanCore> vulkanCore, VulkanSurface* vulkanSurface)
	{
		MAX_FRAMES_IN_FLIGHT = &vulkanSurface->MAX_FRAMES_IN_FLIGHT;
		imageFrameCounter = &vulkanSurface->imageFrameCounter;
	}
}