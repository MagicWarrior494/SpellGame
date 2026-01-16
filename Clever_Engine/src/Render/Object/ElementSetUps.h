#pragma once
#include "Objects/Vertex.h"
#include <vector>

struct ViewPortSetUp {
	int x;
	int y;
	int width;
	int height;
	std::vector<Vertex> initialVertices;
};