#include "VulkanWrapper.h"
#include "CoreTypes/VulkanCore.h"
#include "CoreTypes/VulkanWindow.h"
#include "CoreTypes/VulkanScene.h"
#include "Buffer/VulkanBuffer.h"

namespace Vulkan
{

	VulkanWrapper::VulkanWrapper()
		: m_core(nullptr)
	{
		m_core = std::make_unique<VulkanCore>();
	}

	void VulkanWrapper::CreateCore()
	{
	}

	VulkanCore* VulkanWrapper::GetCore() const
	{
		return m_core.get();
	}
}