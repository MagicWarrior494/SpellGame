#pragma once
#include "ECS/Registry.h"
#include "ECS/Components.h"
#include <random>

class WorldController
{
public:
	WorldController() = default;
	~WorldController() = default;

	void Init();
	void Update();

	void AddTriangle()
	{
		auto entity = registry.CreateEntity();

		std::random_device rd;
		std::mt19937 gen(rd());

		std::uniform_real_distribution<float> distXY(-0.75f, 0.75f);

		Transform transform{};
		transform.position = glm::vec3(distXY(gen), distXY(gen), 0.5f);
		transform.scale = glm::vec3(1.0f, 1.0f, 1.0f);
		transform.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

		registry.SetComponent<Transform>(entity, transform);
	}

	Registry& GetRegistry() { return registry; }

private:
	Registry registry{};
};