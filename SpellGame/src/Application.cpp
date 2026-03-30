#include "Engine.h"
#include "World/Assets/Mesh.h"
#include "World/Assets/Shader.h"
#include "World/Assets/Material.h"
#include "World/ECS/Components.h"
#include "Log.h"
#include <cstdio>

int main()
{

    Engine engine{};    
    Window& window = engine.CreateWindow("SpellGame", 800, 600);
    Scene& scene = engine.CreateScene(window, 800, 600);
    scene.AttachToWindow(window);

    Registry& registry = scene.GetRegistry();
    AssetManager& assets = engine.GetAssetManager();

    auto teapotMesh = assets.Get<Mesh>("models/utah_teapot.obj");
    auto standardShader = assets.Get<Shader>("shaders/simple");


    EntityID teapot = registry.Create();
    registry.Set<Transform>(teapot, Transform{});
    registry.Set<MeshComponent>(teapot, MeshComponent{ teapotMesh });
    registry.Set<ShaderComponent>(teapot, ShaderComponent{ standardShader });

    EntityID camera = registry.Create();
    CameraComponent cam{};
    cam.position    = glm::vec3(0.0f, 3.0f, 8.0f);
    cam.aspectRatio = 800.0f / 600.0f;
    cam.fov         = 60.0f;
    cam.moveSpeed   = 5.0f;
    cam.sensitivity = 0.15f;
    registry.Set<CameraComponent>(camera, cam);

    int frameCount = 0;
    while (window.IsAlive())
    {
        window.Update();
        scene.Update();
        window.Render({ &scene });
    }

    engine.Shutdown();
}