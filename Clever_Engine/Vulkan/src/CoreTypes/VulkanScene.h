#pragma once
#include <glm.hpp>

namespace Vulkan
{
	class VulkanScene
	{
	public:
		VulkanScene() = default;
		~VulkanScene() = default;

		bool Render();

		int& GetAssignedWindowId() { return windowAssignedSceneId; }

	public:
		glm::vec2 sceneSize{ 0.0f, 0.0f };
		glm::vec2 sceneOffset{ 0.0f, 0.0f };

	private:
		int windowAssignedSceneId = -1;
	};
}