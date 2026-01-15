#include "EventController.h"

#include <vector>
#include <iostream>

#include "Io/ConversionData.h"
#include "Render/Window/Window.h"

void EventController::Init()
{
}

void EventController::Update(std::map<uint8_t, std::unique_ptr<Window>>& windows)
{
	glfwPollEvents();
	for (auto& [id, window] : windows)
	{
		KeySet windowKeyset = window->GetKeySet();
		
		for (auto& [eventKeyset, eventAction] : eventSubscriberList)
		{
			bool success = true;
			for (const auto& key : eventKeyset.keys) {
				if (std::find(windowKeyset.keys.begin(), windowKeyset.keys.end(), key) == windowKeyset.keys.end()) {
					success = false;
					break;
				}
			}

			for (const auto& mousebutton : eventKeyset.mouseButtons) {
				if (std::find(windowKeyset.mouseButtons.begin(), windowKeyset.mouseButtons.end(), mousebutton) == windowKeyset.mouseButtons.end()) {
					success = false;
					break;
				}
			}
			 
			if (!success) continue;

			uint32_t current_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			if (current_time - eventAction.time_at_last_press > eventAction.delay_between_presses)
			{
				eventAction.time_at_last_press = current_time;
				eventAction.function(*window);
			}
		}

		window->ClearKeySets();
	}
}



void EventController::CleanUp()
{

}

void EventController::RegisterFunction(KeySet keyset, EventAction eventAction)
{
	eventSubscriberList.insert({ keyset, std::move(eventAction) });
}

//void EventController::setKey(InputCodes::Keyboard key, bool state)
//{
//	keyInputData->keys[key] = state;
//}
//
//void EventController::setMouseButton(InputCodes::Mouse button, bool state)
//{
//	keyInputData->mouseButtons[button] = state;
//}