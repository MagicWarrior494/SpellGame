#pragma once
#include <vulkan/vulkan.h>
#include <glm.hpp>
#include <vector>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/quaternion.hpp>

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

struct Transform
{
	glm::vec3 position{ 0.0f, 0.0f, 0.0f };
	glm::vec4 rotation{ 0.0f, 0.0f, 0.0f, 1.0f }; // Quaternion (x, y, z, w)
	glm::vec3 scale{ 1.0f, 1.0f, 1.0f };

	glm::mat4 modelMatrix{ 0.0f };

	void UpdateModelMatrix() {
		// Translation matrix
		glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
		// Rotation matrix from quaternion
		glm::mat4 rotationMatrix = glm::mat4_cast(glm::quat(rotation.w, rotation.x, rotation.y, rotation.z));
		// Scale matrix
		glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
		// Combine transformations: ModelMatrix = Translation * Rotation * Scale
		modelMatrix = translationMatrix * rotationMatrix * scaleMatrix;
	}

	bool isDirty = true;
};