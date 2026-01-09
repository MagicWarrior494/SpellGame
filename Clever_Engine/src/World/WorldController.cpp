#include "WorldController.h"

#include "ECS/Components.h"
#include <glm/glm.hpp>

void WorldController::Init()
{
	// Initialization logic here
}
void WorldController::Update()
{
	auto& visableComponents = registry.GetAllComponents<Visable>();
	for (auto& [entityID, visableComponent] : visableComponents)
	{
		if(visableComponent.isDirty)
		{
			auto& transformComponent = registry.GetComponent<Transform>(entityID);
			// Update the matrix based on the transform
			// (This is a placeholder; actual matrix calculation would go here)
			visableComponent.matrix = glm::mat4(1.0f); // Identity matrix
			visableComponent.isDirty = false;
		}
	}
}