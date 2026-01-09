#pragma once
#include "ECS/Registry.h"

class WorldController
{
public:
	WorldController() = default;
	~WorldController() = default;

	void Init();
	void Update();


	Registry& GetRegistry() { return registry; }

private:
	Registry registry{};
};