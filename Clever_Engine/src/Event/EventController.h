#pragma once
#include <memory>
#include <unordered_map>
#include <GLFW/glfw3.h>

#include <functional>
#include <vector>

#include "Event/Io/KeySet.h"
#include "Render/Window/Window.h"

class EventController
{
public:
	EventController(std::map<int, std::unique_ptr<Window>>& windows);

	void Init();
	void Update();
	void CleanUp();

	void RegisterFunction(KeySet keyset, std::function<void()> func);

	//void setKey(InputCodes::Keyboard key, bool state);
	//void setMouseButton(InputCodes::Mouse button, bool state);

public:
	std::map<int, std::unique_ptr<Window>>& windows;

private:
	std::unordered_map<KeySet, std::function<void()>, KeySetHash> eventSubscriberList;

	//std::shared_ptr<EventData> eventData = std::make_shared<EventData>();
	//std::shared_ptr<KeyInputData> keyInputData = std::make_shared<KeyInputData>();
};