#pragma once
#include <string>
#include "Render/Graphics/GraphicsAPI.h"

class Asset
{
public:
    virtual ~Asset() = default;
	virtual void UploadToGPU(GraphicsAPI& api) = 0;
	void SetName(const std::string& name) { m_name = name; }
	std::string GetName() const { return m_name; }
private:
	std::string m_name;
};