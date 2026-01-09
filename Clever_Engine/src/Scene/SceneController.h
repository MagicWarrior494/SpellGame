#pragma once
#include <memory>

enum class SceneType {
	CAMERA,
	UI
};

struct RenderTarget {
	uint32_t renderSurfaceID;
	SceneType type;
};

class SceneController
{
};

