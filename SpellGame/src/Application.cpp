#include "Engine.h"

int main()
{
	/*
	Load Settings File
	Create Engine()
	LoadWorld()
	RunWorld()
	*/
	Engine::Engine engine{};

	Renderer& renderer = engine.GetRenderer();
	Window& window = renderer.NewWindow("SpellGame", 800, 600);
	int windowId = window.GetWindowID();

	/*Scene& scene = window.CreateNewScene();

	RegistryManager& registryManager = engine.GetRegistryManager();
	Registry& registry = registryManager.newRegistry();

	AssetManager& assetManager = engine.GetAssetManager();
	int teapotId = assetManager.Load("teapot");

	EntityID teapot = registry.CreateEntity();
	registry.SetComponent<Mesh>(teapot, MeshComponent{ teapotId });
	registry.AddComponent<Transform>(teapot);

	scene.BindRegistry(registry);*/

	bool shouldEnd = false;
	while (!shouldEnd)
	{
		engine.Tick();
		if (!renderer.IsWindowAlive(windowId)) break;
		window.Render();
	}

	engine.Terminate();
}