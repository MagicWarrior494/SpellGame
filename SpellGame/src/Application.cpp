#include "Engine.h"

#include "World/Assets/Mesh.h"
#include "World/Assets/Shader.h"
#include "World/Assets/Material.h"
#include "World/ECS/Components.h"

int main()
{
    Engine engine{};
    Window& window = engine.CreateWindow("SpellGame", 800, 600);

    Scene& scene = engine.CreateScene(window.GetGraphicsWindowID(), 400, 300);

    Registry& registry = scene.GetRegistry();
    auto& assets = engine.GetAssetManager();

    auto teapotMesh = assets.Get<Mesh>("models/utah_teapot.obj");
    auto standardShader = assets.Get<Shader>("shaders/simple");

    standardShader->SetResourceProvider("ColorBlock", []() {
        return { glm::vec4{ 255, 0, 0, 255 } };
        })

    //auto teapotMaterial = std::make_shared<Material>(standardShader);
    //teapotMaterial->SetVector4("color", glm::vec4(0.8f, 0.2f, 0.2f, 1.0f));

    EntityID teapot = registry.Create();
    registry.Set<Transform>(teapot, Transform{});
    registry.Set<MeshComponent>(teapot, MeshComponent{ teapotMesh });
	//registry.Set<MaterialComponent>(teapot, MaterialComponent{ teapotMaterial });
	registry.Set<ShaderComponent>(teapot, ShaderComponent{ standardShader });

    while (window.IsAlive())
    {
        engine.BeginFrame();
        scene.Render();
        engine.EndFrame();
    }

    engine.Shutdown();
}