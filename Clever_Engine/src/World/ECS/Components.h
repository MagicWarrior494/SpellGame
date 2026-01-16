#pragma once
#include <glm.hpp>

#include "Objects/Vertex.h"

//If an entity has this component it will be rendered
struct Visable
{
	glm::mat4 matrix{1.0f};
	bool isDirty = true;
};