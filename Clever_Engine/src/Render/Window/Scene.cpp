#include "Scene.h"
#include "World/Assets/Mesh.h"
#include "World/Assets/Material.h"
#include "Render/PushConstants.h"
#include "World/ECS/Components.h"

void Scene::Update()
{
}

void Scene::Render()
{
    if (m_GraphicsAPI == nullptr || m_AssetManager == nullptr || m_Registry == nullptr)
    {
        return;
    }

    m_GraphicsAPI->RenderScene(m_GraphicsSceneId, *m_Registry, *m_AssetManager);
}

void Scene::ExecuteRenderStage(const RenderStage& /*stage*/)
{
}