#pragma once
#include <memory>
#include <unordered_map>
#include <GLFW/glfw3.h>

#include <functional>
#include <vector>
#include <chrono>

#include "Event/Io/KeySet.h"
#include "Render/Window/Window.h"

struct EventAction
{
	std::function<void(Window& window)> function;
	uint32_t delay_between_presses = 200; // milliseconds
	uint32_t time_at_last_press = 0;//since epoch in milliseconds
public:
	// Constructor
	EventAction(std::function<void(Window&)> func, uint32_t delay = 200)
		: function(std::move(func)), delay_between_presses(delay)
	{
		// Initialize last press time to current time since epoch
		time_at_last_press = static_cast<uint32_t>(
			std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch()
			).count()
		);
	}
};

class EventController
{
public:
	EventController() = default;

	void Init();
	void Update(std::map<uint8_t, std::unique_ptr<Window>>& windows);
	void CleanUp();

	void RegisterFunction(KeySet keyset, EventAction eventAction);

	//void setKey(InputCodes::Keyboard key, bool state);
	//void setMouseButton(InputCodes::Mouse button, bool state);

private:
	std::unordered_map<KeySet, EventAction, KeySetHash> eventSubscriberList;
	
	//std::shared_ptr<EventData> eventData = std::make_shared<EventData>();
	//std::shared_ptr<KeyInputData> keyInputData = std::make_shared<KeyInputData>();
};