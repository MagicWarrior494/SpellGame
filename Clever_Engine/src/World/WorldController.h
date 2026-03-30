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
        auto /*entity*/ _ = registry.Create();
        (void)_;
    }

    Registry& GetRegistry() { return registry; }

private:
    Registry registry{};
};