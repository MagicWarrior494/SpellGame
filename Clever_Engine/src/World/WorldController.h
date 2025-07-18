#pragma once


class WorldController
{
public:
	WorldController();
	~WorldController();

	void Init(std::shared_ptr<VulkanData> vulkanData);
	void StepPhysics(std::shared_ptr<FrameTime> frameTime);

	void CleanUp();

	std::shared_ptr<RenderData> getRenderables();

	void CreateNewGhost(GameObject& gameObject);

	void AddObject();
	void RemoveObject();
	void ModifyObject();

	float clip(float n, float lower, float upper);
	float distance(glm::vec3 v1, glm::vec3 v2);
	glm::vec3 normalize(glm::vec3 v1);
	glm::vec3 clip(glm::vec3 n, float lower, float upper);
private:
	GameObjectsData gameObjects{};
	std::vector<std::shared_ptr<Renderable>> renderables{};
	std::shared_ptr<RenderData> d_RenderData = std::make_shared<RenderData>();
};