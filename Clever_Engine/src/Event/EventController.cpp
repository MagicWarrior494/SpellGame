#include "EventController.h"
#include "Io/ConversionData.h"
#include <iostream>


void EventController::Init(std::shared_ptr<std::map<int, Window>> windows)
{
	this->windows = windows;
}

void EventController::Update()
{
	for (auto& window : *windows)
	{
		KeySet keyset = window.second.GetWindowInputs();
		
		for (auto key : keyset.keys)
		{
			std::cout << window.second.GetWindowTitle() << ": " << keyboardCleverToStringName[key] << std::endl;
		}
		for (auto mouse : keyset.mouseButtons)
		{
			std::cout << window.second.GetWindowTitle() << ": " << mouseCleverToStringName[mouse] << std::endl;
		}
	}
}

void EventController::CleanUp()
{

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