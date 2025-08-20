#pragma once
#include <memory>

#include "Render/Window/Window.h"

class EventController
{
public:
	EventController();

	void Init(Window& window);
	void Update();
	void CleanUp();

	void setKey(InputCodes::Keyboard key, bool state);
	void setMouseButton(InputCodes::Mouse button, bool state);

public:
	std::shared_ptr<EventData> eventData = std::make_shared<EventData>();
	std::shared_ptr<KeyInputData> keyInputData = std::make_shared<KeyInputData>();
};