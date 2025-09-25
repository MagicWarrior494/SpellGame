#include "EventController.h"

#include <vector>
#include <iostream>
#include "Io/ConversionData.h"


EventController::EventController(std::map<int, std::unique_ptr<Window>>& windows)
	: windows(windows)
{
}

void EventController::Init()
{
}

void EventController::Update()
{
	glfwPollEvents();
	for (auto& [id, window] : windows)
	{
		KeySet windowKeyset = window->GetKeySet();
		
		for (auto [eventKeyset, lambdafunction] : eventSubscriberList)
		{
			bool success = true;
			for (const auto& key : eventKeyset.keys) {
				if (std::find(windowKeyset.keys.begin(), windowKeyset.keys.end(), key) == windowKeyset.keys.end()) {
					success = false;
					break;
				}
			}

			if (success == false) break;

			for (const auto& mousebutton : eventKeyset.mouseButtons) {
				if (std::find(windowKeyset.mouseButtons.begin(), windowKeyset.mouseButtons.end(), mousebutton) == windowKeyset.mouseButtons.end()) {
					success = false;
					break;
				}
			}

			if (success == false) break;

			lambdafunction();
		}

		window->ClearKeySets();
	}
}



void EventController::CleanUp()
{

}

void EventController::RegisterFunction(KeySet keyset, std::function<void()> func)
{
	eventSubscriberList.insert({ keyset, std::move(func) });
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