#pragma once
#include <string>

class AssetManager
{
public:
	AssetManager() = default;
	// Load an asset from a file and return a unique identifier for it
	template<typename T>
	int LoadAsset(const std::string& filePath)
	{
		// Implementation to load the asset and store it in a map
		// Return a unique identifier (e.g., an integer) for the loaded asset
		return 0; // Placeholder return value
	}
};