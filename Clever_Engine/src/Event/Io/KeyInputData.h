#pragma once
#include <vector>
#include "ConversionData.h"
#include <unordered_map>

#include <memory>

struct KeySet
{
	std::vector<InputCodes::Keyboard> keys;
	std::vector<InputCodes::Mouse> mouseButtons;
};

struct KeyInputData
{
	bool keys[InputCodes::KEY_UNDEFINED];
	bool mouseButtons[InputCodes::BUTTON_UNDEFINED];
	//std::unordered_map<KeySet*, std::unordered_map<int, std::pair<void (*)(void*), void*>>> subscriptionKeysAndButtonstoFunctionMapping;
};
