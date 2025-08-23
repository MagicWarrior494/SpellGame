#pragma once
#include <memory>
#include <map>

#include "Render/Window/Window.h"

class EventController
{
public:
	EventController() = default;

	void Init(std::shared_ptr<std::map<int, Window>> windows);
	void Update();
	void CleanUp();

	//void setKey(InputCodes::Keyboard key, bool state);
	//void setMouseButton(InputCodes::Mouse button, bool state);

public:
	std::shared_ptr<std::map<int, Window>> windows;

	//std::shared_ptr<EventData> eventData = std::make_shared<EventData>();
	//std::shared_ptr<KeyInputData> keyInputData = std::make_shared<KeyInputData>();
};