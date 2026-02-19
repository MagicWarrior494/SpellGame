#include "Engine.h"

int main()
{
    Engine engine{};

    Window& window = engine.CreateWindow("SpellGame", 800, 600);
    Scene& scene = engine.CreateScene(window.GetWindowID(), 400, 300);

    Registry& registry = scene.GetRegistry();
    auto& assets = engine.GetAssetManager();

    auto teapotMesh = assets.LoadAsset<Mesh>("utah_teapot");

    EntityID teapot = registry.Create();
    registry.Add<Transform>(teapot);
    registry.Set<Mesh>(teapot, Mesh{ teapotMesh });

    while (window.IsAlive())
    {
        engine.BeginFrame();

        scene.Render();   // explicit
        engine.EndFrame();
    }

    engine.Shutdown();
}