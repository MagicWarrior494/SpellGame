#include "Engine.h"
#include "World/Assets/Mesh.h"
#include "World/Assets/Shader.h"
#include "World/Assets/Texture.h"
#include "World/Assets/Material.h"
#include "World/ECS/Components.h"
#include "Log.h"
#include <cstdio>

int main()
{

    Engine engine{};    
    Window& window = engine.CreateWindow("SpellGame", 1280, 720);
    Scene& scene  = engine.CreateScene(window, 1280, 720, 0, 0);

    Registry& registry   = scene.GetRegistry();

    AssetManager& assets = engine.GetAssetManager();

    auto teapotMesh = assets.Get<Mesh>("models/cube.obj");
    auto standardShader = assets.Get<Shader>("shaders/simple_shaded");
    auto fourColorTexture = assets.Get<Texture>("textures/smile.png");

	for (int x = 0; x < 100; ++x)
    for (int y = 0; y < 100; ++y)
    {
        EntityID teapot = registry.Create();
	    Transform transform{};
	    transform.position = glm::vec3(static_cast<float>(x * 20), 0.0f, static_cast<float>(y * 20));
	    transform.rotation = glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        registry.Set<Transform>(teapot, transform);
        registry.Set<MeshComponent>(teapot, MeshComponent{ teapotMesh });
        registry.Set<ShaderComponent>(teapot, ShaderComponent{ standardShader });
		registry.Set<TextureComponent>(teapot, TextureComponent{ fourColorTexture });
    }
    

    EntityID camera = registry.Create();
    CameraComponent cam{};
    cam.position    = glm::vec3(0.0f, 3.0f, 30.0f);
    cam.aspectRatio = 1280.0f / 720.0f;
    cam.fov         = 60.0f;
    cam.moveSpeed   = 50.0f;
    cam.sensitivity = 0.15f;
    registry.Set<CameraComponent>(camera, cam);

    while (engine.IsRunning())
    {
        engine.Tick();
    }
}