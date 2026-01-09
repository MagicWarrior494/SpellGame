#pragma once
#include <glm/glm.hpp>

//Self explanatory
struct Transform
{
	glm::vec3 position{0};
	glm::vec4 rotation{ 1,0,0,0 };
	glm::vec3 scale{ 0 };
};

//If an entity has this component it will be rendered
struct Visable
{
	glm::mat4 matrix{1.0f};
	bool isDirty = true;
};