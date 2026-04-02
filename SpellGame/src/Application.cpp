#include "Engine.h"
#include "World/Assets/Mesh.h"
#include "World/Assets/Shader.h"
#include "World/Assets/Texture.h"
#include "World/Assets/Material.h"
#include "World/ECS/Components.h"
#include "Log.h"
#include <cstdio>
#include <glm.hpp>

int main()
{
    Engine engine{};
    Window& window = engine.CreateWindow("SpellGame", 1280, 720);
    Scene&  scene  = engine.CreateScene(window, 1020, 720, 0, 0);

    Registry&     registry = scene.GetRegistry();
    AssetManager& assets   = engine.GetAssetManager();

    auto lowMesh       = assets.Get<Mesh>("models/utah_teapot_low.obj");
    auto highMesh      = assets.Get<Mesh>("models/utah_teapot_high.obj");
    auto shader        = assets.Get<Shader>("shaders/simple_shaded");
    auto smileTexture  = assets.Get<Texture>("textures/smile.png");
    auto colorTexture  = assets.Get<Texture>("textures/FourColor.png");

    // ---------------------------------------------------------------
    // Low-poly teapot — left side
    // ---------------------------------------------------------------
    EntityID lowEntity = registry.Create();
    registry.Set<MeshComponent>(lowEntity, MeshComponent{ lowMesh });
    registry.Set<ShaderComponent>(lowEntity, ShaderComponent{ shader });

    Transform lowTransform{};
    lowTransform.position = glm::vec3(-10.0f, 0.0f, 0.0f);
    registry.Set<Transform>(lowEntity, lowTransform);

    ShaderDataComponent lowData{};
    lowData.BindTexture("texSampler", smileTexture->texture);
    registry.Set<ShaderDataComponent>(lowEntity, lowData);

    // ---------------------------------------------------------------
    // High-poly teapot — right side
    // ---------------------------------------------------------------
    EntityID highEntity = registry.Create();
    registry.Set<MeshComponent>(highEntity, MeshComponent{ highMesh });
    registry.Set<ShaderComponent>(highEntity, ShaderComponent{ shader });

    Transform highTransform{};
    highTransform.position = glm::vec3(10.0f, 0.0f, 0.0f);
    registry.Set<Transform>(highEntity, highTransform);

    ShaderDataComponent highData{};
    highData.BindTexture("texSampler", colorTexture->texture);
    registry.Set<ShaderDataComponent>(highEntity, highData);

    // ---------------------------------------------------------------
    // Camera
    // ---------------------------------------------------------------
    EntityID camera = registry.Create();
    CameraComponent cam{};
    cam.position    = glm::vec3(0.0f, 50.0f, 200.0f);
    cam.aspectRatio = 1020.0f / 720.0f;
    cam.fov         = 60.0f;
    cam.moveSpeed   = 100.0f;
    cam.sensitivity = 0.15f;
    registry.Set<CameraComponent>(camera, cam);

    // ---------------------------------------------------------------
    // UI Panel — 260px wide sidebar on the right side of the window
    // ---------------------------------------------------------------
    UIScene& ui = engine.CreateUIScene(window, 260, 720, 1020, 0);

    ButtonWidget& btn1 = ui.AddButton();
    btn1.position   = { 20.0f, 20.0f };
    btn1.size       = { 220.0f, 50.0f };
    btn1.color      = { 0.20f, 0.47f, 0.82f, 1.0f };
    btn1.hoverColor = { 0.30f, 0.57f, 0.92f, 1.0f };
    btn1.pressColor = { 0.12f, 0.35f, 0.65f, 1.0f };

    ButtonWidget& btn2 = ui.AddButton();
    btn2.position   = { 20.0f, 90.0f };
    btn2.size       = { 220.0f, 50.0f };
    btn2.color      = { 0.75f, 0.22f, 0.22f, 1.0f };
    btn2.hoverColor = { 0.88f, 0.33f, 0.33f, 1.0f };
    btn2.pressColor = { 0.55f, 0.14f, 0.14f, 1.0f };

    while (engine.IsRunning())
    {
        engine.Tick();
    }
}