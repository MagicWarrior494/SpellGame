#pragma once
#include <vector>
#include "ConversionData.h"
#include <unordered_map>

#include <memory>

struct KeySet
{
	std::vector<InputCodes::Keyboard> keys;
	std::vector<InputCodes::Mouse> mouseButtons;
	bool exact = false;

	
	KeySet() = default;

	KeySet(InputCodes::Keyboard key)
	{
		keys.push_back(key);
	}

	KeySet(InputCodes::Mouse mouseButton)
	{
		mouseButtons.push_back(mouseButton);
	}

	KeySet(std::vector<InputCodes::Keyboard> keys):
		keys(keys)
	{
	}

	KeySet(std::vector<InputCodes::Mouse> mouseButtons) :
		mouseButtons(mouseButtons)
	{
	}

	void clear()
	{
		keys.clear();
		mouseButtons.clear();
	}

	// Equality operator (required for unordered_map/set)
	bool operator==(const KeySet& other) const
	{
		return keys == other.keys &&
			mouseButtons == other.mouseButtons &&
			exact == other.exact;
	}
};

// Hash functor for KeySet, FULLY AI. I HAVE NO IDEA
struct KeySetHash
{
	std::size_t operator()(const KeySet& ks) const
	{
		std::size_t h = std::hash<bool>()(ks.exact);

		// Combine hash for keyboard keys
		for (auto key : ks.keys)
		{
			h ^= std::hash<int>()(static_cast<int>(key)) + 0x9e3779b9 + (h << 6) + (h >> 2);
		}

		// Combine hash for mouse buttons
		for (auto btn : ks.mouseButtons)
		{
			h ^= std::hash<int>()(static_cast<int>(btn)) + 0x9e3779b9 + (h << 6) + (h >> 2);
		}

		return h;
	}
};